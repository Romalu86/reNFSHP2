#include "nfs/audio/codecs/sound_codecs.hpp"
namespace nfs::sound::backend::pending {
using namespace codecs;

int compressed_position(void *state) {
    return field<int>(state, 40);
}

int loop_compressed_position(void *state) {
    return field<int>(state, 40);
}

void release_compressed(void *state) {
    if (field<void *>(state, 28))
        codec_release(field<void *>(state, 28));
}

void release_stream_compressed(void *state) {
    if (field<void *>(state, 28))
        codec_release(field<void *>(state, 28));
}

void release_loop_compressed(void *state) {
    if (field<void *>(state, 28))
        codec_release(field<void *>(state, 28));
}

int process_compressed(void *state, int count, void *, void *output, Byte) {
    if (field<int>(state, 36) <= field<int>(state, 40))
        return -1;
    auto *out = static_cast<float *>(output);
    int n = read_predictor(field<void *>(state, 28), nullptr, &out, count);
    field<Dword>(state, 40) += Dword(n);
    out += n;
    if (n < count) {
        zero_memory(out, Dword(count - n) * 4);
        n = count;
    }
    return n;
}

int process_stream_compressed(void *state, int count, void *, void *output, Byte) {
    int total = 0;
    auto &consumed = field<int>(state, 48);
    if (consumed) {
        consume_stream_samples(field<int>(state, 44), field<Byte>(state, 52), consumed);
        consumed = 0;
    }
    auto *out = static_cast<float *>(output);
    while (count > 0) {
        int n = read_predictor(field<void *>(state, 28), nullptr, &out, count);
        total += n;
        field<Dword>(state, 40) += Dword(n);
        out += n;
        consumed += n;
        if (n < count) {
            count -= n;
            int last;
            int input = read_stream_block(field<int>(state, 44), field<Byte>(state, 52),
                                          &field<int>(state, 36), &last);
            field<Dword>(state, 32) = Dword(input);
            if (!input) {
                if (count > 0 && consumed) {
                    zero_memory(out, Dword(count) * 4);
                    return total + count;
                }
                return total;
            }
            auto *header = reinterpret_cast<const std::int16_t *>(Dword(input));
            float history[2] = {float(header[0]), float(header[1])};
            load_predictor(field<void *>(state, 28), nullptr, history);
            field<Dword>(state, 40) = 0;
            field<Dword>(state, 32) += 4;
            int samples = field<int>(state, 36);
            bind_predictor(field<void *>(state, 28), nullptr,
                           field<void *>(state, 32), Dword(samples) * 4,
                           samples);
        } else
            count -= n;
    }
    return total;
}

int process_loop_compressed(void *state, int count, void *, void *output, Byte) {
    auto *out = static_cast<float *>(output);
    int total = 0;
    if (!field<Dword>(state, 48)) {
        int prefix = field<int>(state, 52) / 28;
        bind_predictor(field<void *>(state, 28), nullptr, field<void *>(state, 32), Dword(prefix) * 112,
                       prefix * 28);
        field<Dword>(state, 48) = 1;
    }
    while (count) {
        int n = read_predictor(field<void *>(state, 28), nullptr, &out, count);
        total += n;
        out += n;
        field<Dword>(state, 40) += Dword(n);
        if (n < count) {
            int first = field<int>(state, 52), skip = first % 28;
            count -= n;
            field<int>(state, 40) = first - skip;
            int samples = field<int>(state, 56) - first + 1 + skip;
            bind_predictor(field<void *>(state, 28), nullptr,
                           field<const Byte *>(state, 32) + (first - skip) / 28 * 15,
                           Dword(samples) * 4, samples);
            if (!field<Dword>(state, 44)) {
                Dword history[2];
                save_predictor(field<void *>(state, 28), nullptr, history);
                field<Dword>(state, 64) = history[1];
                field<Dword>(state, 60) = history[0];
                n = read_predictor(field<void *>(state, 28), nullptr, &out, count);
                total += n;
                field<Dword>(state, 44) = 1;
                out += n;
                field<Dword>(state, 40) += Dword(n);
                count -= n;
            } else {
                Dword history[2] = {field<Dword>(state, 60), field<Dword>(state, 64)};
                load_predictor(field<void *>(state, 28), nullptr, history);
                float discarded[28];
                auto *p = discarded;
                int skipped = read_predictor(field<void *>(state, 28), nullptr, &p, skip);
                field<Dword>(state, 40) += Dword(skipped);
            }
        } else
            count -= n;
    }
    return total;
}

void construct_compressed(void *state, void *descriptor) {
    field<Processor>(state, 0) = process_compressed;
    field<void(__cdecl *)(void *)>(state, 4) = release_compressed;
    field<Dword>(state, 32) = field<Dword>(descriptor, 0);
    field<Dword>(state, 36) = field<Dword>(descriptor, 12);
    field<Dword>(state, 40) = 0;
    void *decoder = codec_allocate(168);
    field<void *>(state, 28) = decoder ? create_predictor(decoder, nullptr) : nullptr;
    field<int(__cdecl *)(void *)>(descriptor, 36) = compressed_position;
    bind_predictor(field<void *>(state, 28), nullptr, field<void *>(state, 32),
                   field<Dword>(state, 36) * 4, field<int>(state, 36));
}

void construct_stream_compressed(void *state, void *descriptor) {
    field<Processor>(state, 0) = process_stream_compressed;
    field<void(__cdecl *)(void *)>(state, 4) = release_stream_compressed;
    int voice = field<int>(descriptor, 28);
    field<int>(state, 44) = stream_slot(root_voice(voice));
    field<Byte>(state, 52) = Byte(find_voice_channel(voice));
    field<Dword>(state, 32) = 0;
    field<Dword>(state, 36) = field<Dword>(descriptor, 12);
    field<Dword>(state, 40) = field<Dword>(state, 48) = 0;
    void *decoder = codec_allocate(168);
    field<void *>(state, 28) = decoder ? create_predictor(decoder, nullptr) : nullptr;
}

void construct_loop_compressed(void *state, void *descriptor) {
    field<Processor>(state, 0) = process_loop_compressed;
    field<void(__cdecl *)(void *)>(state, 4) = release_loop_compressed;
    field<Dword>(state, 32) = field<Dword>(descriptor, 0);
    field<Dword>(state, 36) = field<Dword>(descriptor, 12);
    field<Dword>(state, 40) = field<Dword>(state, 44) = field<Dword>(state, 48) = 0;
    field<Dword>(state, 52) = field<Dword>(descriptor, 16);
    field<Dword>(state, 56) = field<Dword>(descriptor, 20);
    void *decoder = codec_allocate(168);
    field<void *>(state, 28) = decoder ? create_predictor(decoder, nullptr) : nullptr;
    field<int(__cdecl *)(void *)>(descriptor, 36) = loop_compressed_position;
}
} // namespace nfs::sound::backend::pending
