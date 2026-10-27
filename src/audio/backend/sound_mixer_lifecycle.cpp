#include "nfs/audio/backend/sound_ownership.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
#include "../../math/precision/frame_x87_bridge.hpp"
#include <cmath>
namespace nfs::sound::backend {
void *mixer_allocate(Dword);
void mixer_release(void *);

void initialize_mixer(const void *description) {
    register_codec_allocator(mixer_allocate);
    register_codec_release(mixer_release);
    mixer<Byte>(8717711) = 0;
    mixer<Dword>(8717696) = field<Dword>(description, 0);
    mixer<Dword>(8717700) = field<Dword>(description, 4);
    mixer<Dword>(8717704) = field<Dword>(description, 8);
    lock();
    for (unsigned i = 0; i < 2; ++i) {
        Dword raw = reinterpret_cast<Dword>(allocate(8380));
        mixer<Dword>(8717984 + i * 4) = raw;
        Dword aligned = raw + 8;
        while (aligned & 63)
            aligned += 4;
        mixer<Dword>(8717992 + i * 4) = aligned;
    }
    if (mixer<Byte>(8717702))
        mixer<void *>(8718032) = allocate(1024);
    for (unsigned c = 0; c < pending::mix_modes; ++c) {
        Dword raw = reinterpret_cast<Dword>(allocate(1088));
        mixer<Dword>(8718000 + c * 4) = raw;
        while (raw & 63)
            raw += 4;
        mixer<Dword>(8718016 + c * 4) = raw;
    }
    if (mixer<Byte>(8717700)) {
        mix_voices = static_cast<MixVoice *>(allocate(mixer<Byte>(8717700) * 80));
        zero_memory(mix_voices, mixer<Byte>(8717700) * 80);
    }
    unlock();
    pending::register_pcm_codecs();
    pending::register_pcm16_codecs();
    pending::register_compressed_codecs();
    pending::register_speech_codecs();
    bind_mixer_methods();
    for (unsigned c = 0; c < pending::mix_modes; ++c) {
        mixer<void *>(8718056 + c * 4) = nullptr;
        auto *converter = mixer_bytes + 392 + c * 28;
        construct_converter(converter);
        field<std::uint16_t>(converter, 24) = 0;
        insert_processor(&mixer<void *>(8718056 + c * 4), converter);
        zero_memory(mixer<void *>(8718016 + c * 4), 1024);
    }
}

void reset_mixer_filter(float ratio) {
    for (unsigned c = 0; c < pending::mix_modes; ++c) {
        auto &filter = mixer<void *>(8718072 + c * 4);
        if (!(field<volatile float>(&ratio, 0) < 1.0f)) {
            if (filter) {
                release(filter);
                filter = nullptr;
            }
        } else {
            if (!filter) {
                filter = allocate(40);
                pending::create_filter(filter);
                field<Dword>(filter, 8) = 0;
            }

            const Dword source_rate = setting<std::uint16_t>(8708898);
            const long double scaled_rate =
                static_cast<long double>(source_rate) * field<volatile float>(&ratio, 0);
            const Dword rate = Dword(game::runtime::hardware::truncate_integer_native80(&scaled_rate));
            int parameters[3] = {int(rate << 7), int(source_rate << 8), 256};
            pending::update_filter(filter, parameters);
        }
    }
}

void shutdown_mixer() {
    lock();
    if (mixer<void *>(8718032)) {
        release(mixer<void *>(8718032));
        mixer<void *>(8718032) = nullptr;
    }
    if (mix_voices) {
        release(mix_voices);
        mix_voices = nullptr;
    }
    for (unsigned i = 0; i < 2; ++i) {
        auto &p = mixer<void *>(8717984 + i * 4);
        if (p) {
            release(p);
            p = nullptr;
        }
    }
    for (unsigned c = 0; c < pending::mix_modes; ++c) {
        auto &p = mixer<void *>(8718000 + c * 4);
        if (p) {
            release(p);
            p = nullptr;
        }
    }
    reset_mixer_filter(2.f);
    unlock();
}
} // namespace nfs::sound::backend
