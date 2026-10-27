#include "nfs/audio/backend/sound_backend.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
namespace nfs::sound::backend::pending {
// Byte-pattern cursor chooses insertion/deletion boundaries.
int resampler_phase(int block, float rate, const Byte **pattern, float *error) {

    auto X = [&](float v) { return static_cast<long double>(v); };
    const auto scaled = X(rate) * static_cast<long double>(block);
    const int pivot = block / 2 + **pattern;
    const float unchanged = float(static_cast<long double>(block) + *error - scaled);
    auto magnitude = [&](long double v) { return v < X(0) ? -v : v; };
    if (rate < 1) {
        auto changed = static_cast<long double>(block * 2 - pivot) + *error - (scaled + scaled);
        if (magnitude(X(unchanged)) < magnitude(changed)) {
            ++*pattern;
            *error = unchanged;
            return 0;
        }
        *pattern += 2;
        *error = float(changed);
        return -pivot;
    }
    auto changed = static_cast<long double>(pivot + block) + *error - scaled;
    ++*pattern;
    if (magnitude(X(unchanged)) < magnitude(changed)) {
        *error = unchanged;
        return 0;
    }
    *error = float(changed);
    return pivot;
}

int resampler_input_count(void *state, int count) {
    if (count <= field<int>(state, 56))
        return 0;
    int remaining = count - field<int>(state, 56), required = 0, phase = 0,
        available = field<int>(state, 48);
    const Byte *pattern = field<const Byte *>(state, 28);
    float error = field<float>(state, 40);
    while (remaining > 0) {
        int block = field<int>(state, 44);
        if (available < block)
            break;
        phase = 0;
        if (available >= block * 2) {
            phase = resampler_phase(block, field<float>(state, 36), &pattern, &error);
            if (phase < 0)
                block *= 2;
        }
        required += block;
        available -= block;
        if (remaining <= block + phase) {
            remaining = 0;
            break;
        }
        remaining -= block + phase;
    }
    if (phase > 0)
        required += field<int>(state, 44);
    if (remaining > 0)
        required += available < remaining ? available : remaining;
    return required > field<int>(state, 52) ? required - field<int>(state, 52) : 0;
}
// Offset crossfade retains the saved source addressing.
void resampler_crossfade(const float *first, const float *second, float *output, int count,
                         int offset) {

    auto X = [&](float v) { return static_cast<long double>(v); };
    auto weight = X(offset > 0 ? 1.f : 0.f),
         step = X(offset > 0 ? -1.f : 1.f) / static_cast<long double>(count);
    if (offset < 0)
        offset = -offset;
    for (int i = offset, j = 0; i < count; ++i, ++j) {
        output[j] = float((X(1) - weight) * first[j] + weight * first[i]);
        weight = step + weight;
    }
    for (int i = 0; i < offset; ++i) {
        int j = count - offset + i;
        output[j] = float((X(1) - weight) * first[j] + weight * second[i]);
        weight = step + weight;
    }
}

int resampler_drain(void *state, int *remaining, Dword **output) {
    int amount = field<int>(state, 56) < *remaining ? field<int>(state, 56) : *remaining;
    auto *source =
        reinterpret_cast<Dword *>(static_cast<Byte *>(state) + 2104) + field<int>(state, 60);
    for (int i = 0; i < amount; ++i)
        (*output)[i] = source[i];
    field<Dword>(state, 60) += Dword(amount);
    field<Dword>(state, 56) -= Dword(amount);
    *remaining -= amount;
    *output += amount;
    return amount;
}

void resampler_prepare(void *state, Dword **input, int phase) {
    int block = field<int>(state, 44);
    Dword *first, *second;
    if (field<int>(state, 52) < 1) {
        first = *input;
        second = first + block;
    } else {
        first = reinterpret_cast<Dword *>(static_cast<Byte *>(state) + 64);
        second = *input;
    }
    auto *pending = reinterpret_cast<Dword *>(static_cast<Byte *>(state) + 2104);
    if (!phase) {
        for (int i = 0; i < block; ++i)
            pending[i] = first[i];
        field<int>(state, 56) = block;
        field<Dword>(state, 48) -= Dword(block);
        *input = second;
    } else if (phase > 0) {
        for (int i = 0; i < phase; ++i)
            pending[i] = first[i];
        resampler_crossfade(reinterpret_cast<float *>(first), reinterpret_cast<float *>(second),
                            reinterpret_cast<float *>(pending + phase), block, phase);
        field<int>(state, 56) = block + phase;
        field<Dword>(state, 48) -= Dword(block);
        *input = second;
    } else {
        resampler_crossfade(reinterpret_cast<float *>(first), reinterpret_cast<float *>(second),
                            reinterpret_cast<float *>(pending), block, phase);
        for (int i = 0; i < block + phase; ++i)
            pending[block + i] = second[-phase + i];
        field<int>(state, 56) = block * 2 + phase;
        field<Dword>(state, 48) -= Dword(block) * 2;
        *input = second + block;
    }
    field<Dword>(state, 52) = field<Dword>(state, 60) = 0;
}

int resampler_output(void *state, int count, void *input, void *output) {
    auto *source = static_cast<Dword *>(input), *destination = static_cast<Dword *>(output);
    int total = resampler_drain(state, &count, &destination), phase = 0;
    if (!count)
        return total;
    while (count > 0) {
        int block = field<int>(state, 44);
        if (field<int>(state, 48) < block)
            break;
        phase = 0;
        if (field<int>(state, 48) >= block * 2)
            phase = resampler_phase(block, field<float>(state, 36),
                                    &field<const Byte *>(state, 28), &field<float>(state, 40));
        resampler_prepare(state, &source, phase);
        total += resampler_drain(state, &count, &destination);
    }
    if (phase > 0) {
        int block = field<int>(state, 44);
        auto *saved = reinterpret_cast<Dword *>(static_cast<Byte *>(state) + 64);
        for (int i = 0; i < block; ++i)
            saved[i] = source[i];
        field<int>(state, 52) = block;
        source += block;
    }
    if (count > 0) {
        int amount = field<int>(state, 48) < count ? field<int>(state, 48) : count;
        for (int i = 0; i < amount; ++i)
            destination[i] = source[i];
        total += amount;
        field<Dword>(state, 48) -= Dword(amount);
    }
    return total;
}
} // namespace nfs::sound::backend::pending
