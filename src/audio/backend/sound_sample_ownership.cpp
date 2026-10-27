#include "nfs/audio/backend/sound_ownership.hpp"
#include "../../math/precision/frame_x87_bridge.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
namespace nfs::sound::backend {

void *bind_sample_owner(void *owner) {
    for (unsigned i = 0; i < (stream_limits & 65535); ++i) {
        auto *entry = stream_bindings + i * 2;
        if (!entry[0]) {
            entry[0] = reinterpret_cast<Dword>(owner);
            return entry;
        }
    }
    return nullptr;
}

void unbind_sample_owner(void *owner) {
    for (unsigned i = 0; i < (stream_limits & 65535); ++i)
        if (stream_bindings[i * 2] == reinterpret_cast<Dword>(owner)) {
            stream_bindings[i * 2] = 0;
            return;
        }
}

int find_sample_slot() {
    for (int i = 1; i < 256; ++i)
        if (!field<Dword>(sample_bindings, i * 24))
            return i;
    return 0;
}
// Unwritten descriptor words and the saved invalid-format path are retained.
int decode_sample_buffer(void *data, int format, int loop_start, int loop_end, int samples,
                         void *destination, int output_format) {
    auto *memory = static_cast<Byte *>(
        VirtualAlloc(nullptr, Dword(samples) * 4 + 20000, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE));
    auto *decoded = memory;
    void *decoder = nullptr;
    Dword descriptor[9];
    descriptor[0] = reinterpret_cast<Dword>(data);
    descriptor[2] = 1;
    descriptor[3] = samples;
    descriptor[8] = control.unknown_7;
    if (format == 8 || format == 9 || format == 10 || format == 4) {
        lock();
        decoder = allocate(format == 4 ? 3432 : 44);
        field<Dword>(decoder, 4) = 0;
        if (format == 8 || format == 9) {
            descriptor[6] = format == 8 ? 1 : 0;
            pending::construct_pcm(decoder, descriptor);
        } else if (format == 10) {
            decoded += 8;
            pending::construct_compressed(decoder, descriptor);
        } else
            pending::construct_speech(decoder, descriptor);
        unlock();
        field<pending::Processor>(decoder, 0)(decoder, samples, nullptr, decoded, 0);
    } else if (format == 15) {
        lock();
        decoder = allocate(44);
        field<Dword>(decoder, 4) = 0;
        field<Byte>(decoder, 26) = 0;
        mixer<pending::Constructor>(8717780)(decoder, descriptor);
        unlock();
        int offset = 0;
        auto *cursor = decoded;
        while (offset < samples) {
            int count = offset < 1153 ? offset : 1152;
            field<pending::Processor>(decoder, 0)(decoder, count, nullptr, cursor, 0);
            offset = int(Dword(offset) + 1152u);
            cursor += 4608;
        }
    }
    lock();
    if (auto destroy = field<void(__cdecl *)(void *)>(decoder, 4))
        destroy(decoder);
    release(decoder);
    unlock();
    if (loop_end > 0) {
        decoded += Dword(loop_start) * 4;
        samples = int(Dword(loop_end) - Dword(loop_start) + 1u);
    }
    if (output_format == 8) {
        for (int i = 0; i < samples; ++i) {
            if (field<volatile float>(decoded, Dword(i) * 4) > 32767)
                field<std::uint16_t>(destination, Dword(i) * 2) = 32767;
            else if (field<volatile float>(decoded, Dword(i) * 4) < -32768)
                field<std::uint16_t>(destination, Dword(i) * 2) = 32768;
            else {
                const long double sample = static_cast<long double>(field<volatile float>(decoded, Dword(i) * 4));
                field<std::uint16_t>(destination, Dword(i) * 2) =
                    std::uint16_t(game::runtime::hardware::truncate_integer_native80(&sample));
            }
        }
    }
    if (format == 10)
        decoded -= 8;
    VirtualFree(decoded, 0, MEM_RELEASE);
    return 0;
}

int rebuild_sample_buffer(int slot) {
    void *state = sample_bindings + Dword(slot) * 24;
    void *data = field<void *>(state, 0);
    if (!data)
        return 0;
    int bits = field<Byte>(state, 23) == 8 ? 16 : slot;
    int samples = field<int>(state, 8);
    if (field<int>(state, 16) > 0)
        samples = int(field<Dword>(state, 16) - field<Dword>(state, 12) + 1u);
    int stride = bits >> 3;
    Dword bytes = Dword(stride) * Dword(samples);
    alignas(4) Byte wave[18], description[36];
    zero_memory(description, 36);
    field<volatile std::uint16_t>(wave, 0) = 1;
    field<volatile std::uint16_t>(wave, 2) = 1;
    field<Dword>(wave, 4) = field<std::uint16_t>(state, 20);
    field<Dword>(wave, 8) = field<Dword>(wave, 4) * Dword(stride);
    field<std::uint16_t>(wave, 12) = std::uint16_t(stride);
    field<std::uint16_t>(wave, 14) = std::uint16_t(bits);
    field<std::uint16_t>(wave, 16) = 0;
    field<Dword>(description, 0) = 36;
    field<Dword>(description, 4) = 295088;
    field<Dword>(description, 8) = bytes;
    field<void *>(description, 16) = wave;
    auto &buffer = field<void *>(state, 4);
    if (device_call<int>(direct_sound, 12, description, &buffer, 0))
        return 0;
    void *output;
    Dword locked;
    if (!device_call<int>(buffer, 44, 0, bytes, &output, &locked, 0, 0, 0)) {
        decode_sample_buffer(data, field<std::int8_t>(state, 22), field<int>(state, 12),
                             field<int>(state, 16), int(locked), output,
                             field<std::int8_t>(state, 23));
        if (device_call<int>(buffer, 76, output, bytes, 0, 0))
            return 0;
    }
    return slot;
}

int create_sample_buffer(Byte format, std::uint16_t rate, Dword samples, int first, int last,
                         void *data) {
    if (!direct_sound)
        return 0;
    int slot = find_sample_slot();
    if (slot < 1)
        return 0;
    void *state = sample_bindings + Dword(slot) * 24;
    field<Byte>(state, 22) = format;
    field<std::uint16_t>(state, 20) = rate;
    field<Dword>(state, 8) = samples;
    field<int>(state, 12) = first;
    field<Byte>(state, 23) = 8;
    field<int>(state, 16) = last;
    field<void *>(state, 0) = data;
    int result = rebuild_sample_buffer(slot);
    if (result < 1)
        field<Dword>(state, 0) = 0;
    return result;
}

int release_sample_buffer(int slot) {
    if (direct_sound)
        device_call<Dword>(field<void *>(sample_bindings, Dword(slot) * 24 + 4), 8);
    field<Dword>(sample_bindings, Dword(slot) * 24) = 0;
    return 0;
}
} // namespace nfs::sound::backend
