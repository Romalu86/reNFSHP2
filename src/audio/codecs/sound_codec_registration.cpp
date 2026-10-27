#include "nfs/audio/backend/sound_ownership.hpp"
namespace nfs::sound::backend {
namespace pending {
void *(__cdecl *codec_allocator)(Dword){};
void(__cdecl *codec_deallocator)(void *){};
} // namespace pending

void register_codec_allocator(void *(__cdecl *method)(Dword)) {
    pending::codec_allocator = method;
}

void register_codec_release(void(__cdecl *method)(void *)) {
    pending::codec_deallocator = method;
}

void *codec_allocate(Dword bytes) {
    return pending::codec_allocator(bytes);
}

void codec_release(void *memory) {
    pending::codec_deallocator(memory);
}

void *mixer_allocate(Dword bytes) {
    return allocate(bytes);
}

void mixer_release(void *memory) {
    release(memory);
}

void set_mixer_mode(Byte mode) {
    pending::mix_format[2] = mode;
}

void bind_mixer_methods() {
    using Mix = void(__cdecl *)(int, float, const float *, float *);
    using Convert = void(__cdecl *)(int, const std::int16_t *, float *);
    if (control.unknown_7 & 4) {
        mixer<Mix>(8718204) = pending::mix_packed;
        mixer<Convert>(8718208) = pending::pcm16_packed;
    } else {
        mixer<Convert>(8718208) = pending::pcm16_scalar;
        mixer<Mix>(8718204) = control.unknown_7 & 2 ? pending::mix_streaming : pending::mix_scalar;
    }
    mixer<std::uint16_t>(8717708) = 512;
    if (mixer<Byte>(8717702))
        mixer<std::uint16_t>(8717708) = std::uint16_t(pending::mix_modes) << 9;
}
namespace pending {

void register_pcm_codecs() {
    build_pcm8_table();
    codec_constructors[0] = reinterpret_cast<Dword>(construct_pcm);
    codec_sizes[0] = 44;
    codec_constructors[1] = reinterpret_cast<Dword>(construct_loop_pcm);
    codec_sizes[1] = 48;
    codec_constructors[2] = reinterpret_cast<Dword>(construct_stream_pcm);
    codec_sizes[2] = 52;
}

void register_pcm16_codecs() {
    codec_constructors[3] = reinterpret_cast<Dword>(construct_pcm);
    codec_sizes[3] = 44;
    codec_constructors[4] = reinterpret_cast<Dword>(construct_loop_pcm);
    codec_sizes[4] = 48;
    codec_constructors[5] = reinterpret_cast<Dword>(construct_stream_pcm);
    codec_sizes[5] = 52;
}

void register_compressed_codecs() {
    codec_constructors[6] = reinterpret_cast<Dword>(construct_compressed);
    codec_sizes[6] = 44;
    codec_constructors[7] = reinterpret_cast<Dword>(construct_loop_compressed);
    codec_sizes[7] = 68;
    codec_constructors[8] = reinterpret_cast<Dword>(construct_stream_compressed);
    codec_sizes[8] = 56;
}

void register_speech_codecs() {
    codec_constructors[9] = reinterpret_cast<Dword>(construct_speech);
    codec_sizes[9] = 3432;
    codec_constructors[11] = reinterpret_cast<Dword>(construct_stream_speech);
    codec_sizes[11] = 3444;
}
} // namespace pending
} // namespace nfs::sound::backend
