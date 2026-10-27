#include "nfs/audio/backend/sound_ownership.hpp"
#include "../../math/precision/extended_float.hpp"
namespace nfs::sound::backend::pending {
float pcm8_table[256]{};

void build_pcm8_table() {
    for (int value = -128; value < 128; ++value)
        pcm8_table[Byte(value)] = float(value) * 256.f;
}
// Reverse groups preserve the saved overlapping-store order.
void pcm8_scalar(int count, const Byte *input, float *output) {
    Dword n = Dword(count);
    while (n & 3) {
        output[n - 1] = pcm8_table[input[n - 1]];
        if (!--n)
            return;
    }
    do {
        const Dword p = n - 4;
        float b = pcm8_table[input[p + 1]];
        Byte c = input[p + 2], d = input[p + 3];
        output[p] = pcm8_table[input[p]];
        output[p + 1] = b;
        output[p + 2] = pcm8_table[c];
        output[p + 3] = pcm8_table[d];
        Dword before = n;
        n -= 4;
        if (!n || int(before) <= 3)
            break;
    } while (true);
}

void pcm16_scalar(int count, const std::int16_t *input, float *output) {
    Dword n = Dword(count);
    while (n & 7) {
        output[n - 1] = float(input[n - 1]);
        if (!--n)
            return;
    }
    do {
        Dword p = n - 8;
        std::int16_t v[8];
        for (unsigned j = 0; j < 8; ++j)
            v[j] = input[p + j];
        output[p] = float(v[0]);
        for (unsigned j = 6; j > 0; --j)
            output[p + j] = float(v[j]);
        output[p + 7] = float(v[7]);
        Dword before = n;
        n -= 8;
        if (!n || int(before) <= 7)
            break;
    } while (true);
}

int pcm_position(void *state) {
    return field<int>(state, 32);
}

int loop_pcm_position(void *state) {
    return field<int>(state, 32);
}
namespace {
void decode(bool wide, int count, const Byte *source, float *output) {
    using Convert = void(__cdecl *)(int, const std::int16_t *, float *);
    if (wide)
        mixer<Convert>(8718208)(count, reinterpret_cast<const std::int16_t *>(source), output);
    else
        pcm8_scalar(count, source, output);
}
} // namespace

int process_pcm(void *state, int count, void *, void *output, Byte) {
    Dword cursor = field<Dword>(state, 32), length = field<Dword>(state, 36);
    if (cursor >= length)
        return -1;
    field<Dword>(state, 32) = cursor + Dword(count);
    bool wide = field<Dword>(state, 40) != 0;
    auto *source = field<const Byte *>(state, 28) + cursor * (wide ? 2 : 1);
    if (cursor + Dword(count) < length)
        decode(wide, count, source, static_cast<float *>(output));
    else {
        int available = int(length - cursor);
        decode(wide, available, source, static_cast<float *>(output));
        zero_memory(static_cast<Byte *>(output) + available * 4, Dword(count - available) * 4);
    }
    return 1;
}

int process_loop_pcm(void *state, int count, void *, void *output, Byte) {
    auto *out = static_cast<float *>(output);
    while (count > 0) {
        auto &cursor = field<int>(state, 32);
        int available = field<int>(state, 40) - cursor + 1;
        if (count < available)
            available = count;
        bool wide = field<Dword>(state, 44) != 0;
        decode(wide, available, field<const Byte *>(state, 28) + cursor * (wide ? 2 : 1), out);
        cursor += available;
        count -= available;
        out += available;
        if (field<Dword>(state, 40) < Dword(cursor))
            cursor = field<int>(state, 36);
    }
    return 1;
}

int process_stream_pcm(void *state, int count, void *, void *output, Byte) {
    auto &consumed = field<int>(state, 44);
    if (consumed) {
        consume_stream_samples(field<int>(state, 40), field<Byte>(state, 49), consumed);
        consumed = 0;
    }
    auto *out = static_cast<float *>(output);
    while (count > 0) {
        auto &cursor = field<int>(state, 36), &available = field<int>(state, 32);
        if (available <= cursor) {
            int samples, last;
            int source = read_stream_block(field<int>(state, 40), field<Byte>(state, 49),
                                           &samples, &last);
            field<Dword>(state, 28) = Dword(source);
            if (!source) {
                if (consumed)
                    zero_memory(out, Dword(count) * 4);
                break;
            }
            cursor = 0;
            available = samples;
        }
        int amount = available - cursor;
        if (count < amount)
            amount = count;
        bool wide = field<Byte>(state, 48) != 0;
        decode(wide, amount, field<const Byte *>(state, 28) + cursor * (wide ? 2 : 1), out);
        count -= amount;
        cursor += amount;
        out += amount;
        consumed += amount;
    }
    return consumed;
}

void construct_pcm(void *state, void *descriptor) {
    field<Processor>(state, 0) = process_pcm;
    field<Dword>(state, 28) = field<Dword>(descriptor, 0);
    field<Dword>(state, 32) = 0;
    field<Dword>(state, 36) = field<Dword>(descriptor, 12);
    field<Dword>(state, 40) = field<Dword>(descriptor, 24);
    field<int(__cdecl *)(void *)>(descriptor, 36) = pcm_position;
}

void construct_loop_pcm(void *state, void *descriptor) {
    field<Processor>(state, 0) = process_loop_pcm;
    field<Dword>(state, 28) = field<Dword>(descriptor, 0);
    field<Dword>(state, 32) = 0;
    field<Dword>(state, 36) = field<Dword>(descriptor, 16);
    field<Dword>(state, 40) = field<Dword>(descriptor, 20);
    field<Dword>(state, 44) = field<Dword>(descriptor, 24);
    field<int(__cdecl *)(void *)>(descriptor, 36) = loop_pcm_position;
}

void construct_stream_pcm(void *state, void *descriptor) {
    field<Processor>(state, 0) = process_stream_pcm;
    int voice = field<int>(descriptor, 28);
    field<int>(state, 40) = stream_slot(root_voice(voice));
    field<Byte>(state, 49) = Byte(find_voice_channel(voice));
    field<Dword>(state, 28) = 0;
    field<Dword>(state, 32) = 4294967295u;
    field<Dword>(state, 36) = field<Dword>(state, 44) = 0;
    field<Byte>(state, 48) = field<Byte>(descriptor, 24);
}
} // namespace nfs::sound::backend::pending
