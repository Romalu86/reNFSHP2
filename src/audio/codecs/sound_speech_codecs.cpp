#include "nfs/audio/codecs/sound_codecs.hpp"
namespace nfs::sound::backend::pending {

int speech_position(void *state) {
    return field<int>(state, 32);
}

int process_speech(void *state, int count, void *, void *output, Byte) {
    auto &position = field<Dword>(state, 32);
    Dword length = field<Dword>(state, 28);
    if (position >= length)
        return -1;
    position += Dword(count);
    int padding = int(position - length);
    if (padding < 1)
        padding = 0;
    else
        count -= padding;
    auto *out = static_cast<Dword *>(output);
    while (count > 0) {
        int n = field<int>(state, 36);
        if (count < n)
            n = count;
        count -= n;
        auto *input = reinterpret_cast<Dword *>(static_cast<Byte *>(state) +
                                               (858 - field<int>(state, 36)) * 4);
        for (int i = 0; i < n; ++i)
            *out++ = input[i];
        field<int>(state, 36) -= n;
        if (count < 1)
            break;
        codecs::decode_speech_block(static_cast<Byte *>(state) + 40);
        field<int>(state, 36) = 432;
    }
    for (int i = 0; i < padding; ++i)
        *out++ = 0;
    return 1;
}

int process_stream_speech(void *state, int count, void *, void *output, Byte) {
    auto &consumed = field<int>(state, 40);
    if (consumed) {
        consume_stream_samples(field<int>(state, 36), field<Byte>(state, 3440), consumed);
        consumed = 0;
    }
    auto *out = static_cast<Dword *>(output);
    while (count > 0) {
        auto &available = field<int>(state, 32);
        auto &cursor = field<std::uint16_t>(state, 44),
             &pending = field<std::uint16_t>(state, 46);
        if (available <= cursor) {
            int last;
            int input = read_stream_block(field<int>(state, 36), field<Byte>(state, 3440),
                                          &available, &last);
            field<Dword>(state, 28) = Dword(input);
            cursor = 0;
            if (!input) {
                if (consumed)
                    zero_memory(out, Dword(count) * 4);
                available = 0;
                return consumed;
            }
            const auto *source = reinterpret_cast<const Byte *>(Dword(input));
            if (!*source) {
                field<Dword>(state, 52) = source[1];
                field<Dword>(state, 56) = 8;
                field<const Byte *>(state, 48) = source + 2;
            } else {
                pending = 0;
                codecs::initialize_speech(source + 1, static_cast<Byte *>(state) + 48);
            }
        }
        if (!std::int16_t(pending)) {
            codecs::decode_speech_block(static_cast<Byte *>(state) + 48);
            pending = 432;
        }
        Dword amount = Dword(available) - cursor;
        if (count < int(amount))
            amount = Dword(count);
        if (pending < amount)
            amount = pending;
        count -= int(amount);
        auto *input = reinterpret_cast<Dword *>(static_cast<Byte *>(state) + (860 - pending) * 4);
        for (Dword i = 0; i < amount; ++i)
            *out++ = input[i];
        cursor = std::uint16_t(cursor + amount);
        pending = std::uint16_t(pending - amount);
        consumed += int(amount);
    }
    return consumed;
}

void construct_speech(void *state, void *descriptor) {
    field<Processor>(state, 0) = process_speech;
    field<Dword>(state, 28) = field<Dword>(descriptor, 12);
    field<Dword>(state, 32) = 0;
    field<int(__cdecl *)(void *)>(descriptor, 36) = speech_position;
    codecs::initialize_speech(field<const Byte *>(descriptor, 0),
                              static_cast<Byte *>(state) + 40);
    field<Dword>(state, 36) = 0;
}

void construct_stream_speech(void *state, void *descriptor) {
    field<Processor>(state, 0) = process_stream_speech;
    int voice = field<int>(descriptor, 28);
    field<int>(state, 36) = stream_slot(root_voice(voice));
    field<Byte>(state, 3440) = Byte(find_voice_channel(voice));
    field<Dword>(state, 28) = 0;
    field<std::uint16_t>(state, 44) = 0;
    field<Dword>(state, 32) = 0;
    field<std::uint16_t>(state, 46) = 0;
    field<Dword>(state, 40) = 0;
}
} // namespace nfs::sound::backend::pending
