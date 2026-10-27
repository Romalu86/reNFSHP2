#include "nfs/audio/backend/sound_ownership.hpp"
#include "nfs/physics/frame/dynamics_runtime.hpp"
#include <cstdlib>
namespace nfs::sound::backend {
extern const Dword seed_words[6];
extern Byte author_tag;

void seed_generator(int seed) {
    for (unsigned i = 0; i < 6; ++i)
        sound::pending::audio_random_state[i] = seed_words[i] + Dword(seed);
}

void start_backend_thread() {
    lifecycle<volatile Byte>(8708812) = 1;
    lifecycle<volatile Byte>(8708813) = 1;
    platform_tail<HANDLE>(8717656) =
        CreateThread(nullptr, 0, backend_thread, nullptr, 0, &platform_tail<DWORD>(8717660));
    SetThreadPriority(platform_tail<HANDLE>(8717656), 15);
}
// The saved shutdown waits; it does not close the thread handle.
void stop_backend_thread() {
    Byte running = lifecycle<volatile Byte>(8708812);
    lifecycle<volatile Byte>(8708813) = 0;
    while (running) {
        Sleep(0);
        running = lifecycle<volatile Byte>(8708812);
    }
}

DWORD WINAPI backend_thread(void *) {
    while (lifecycle<volatile Byte>(8708813)) {
        auto first = field<Callback>(effect_groups, 76);
        if (first)
            first();
        pump_backend();
        auto last = field<Callback>(effect_groups, 80);
        if (last)
            last();
        Sleep(setting<volatile Dword>(8708908) & 255);
    }
    lifecycle<volatile Byte>(8708812) = 0;
    return 0;
}

void pump_backend() {
    wait_mutex();
    prime_output_ring();
    release_mutex();
}

int stop_output_device() {
    stop_backend_thread();
    release_output_device();
    return 0;
}

void at_exit_shutdown() {
    shutdown_audio();
}

int initialize_backend() {
    field<std::uint16_t>(&stream_limits, 0) = 256;
    lock();
    for (unsigned i = 0; i < setting<Byte>(8708933); ++i) {
        if ((setting<Byte>(8708936 + i * 2) & 17) == 17) {
            stream_bindings =
                static_cast<Dword *>(allocate((stream_limits & 65535) * 8));
            break;
        }
        stream_bindings = nullptr;
    }
    backends = static_cast<BackendVoice *>(allocate(Dword(control.voice_count) * 260));
    zero_memory(backends, Dword(control.voice_count) * 260);
    if (lifecycle<Byte>(8708816))
        for (unsigned i = 0; i < 2; ++i) {
            Dword raw = reinterpret_cast<Dword>(allocate(8224));
            lifecycle<Dword>(8708820 + i * 4) = raw;
            raw += 8;
            while (raw & 15)
                raw += 4;
            lifecycle<Dword>(8708828 + i * 4) = raw;
        }
    unlock();
    if (!lifecycle<Byte>(8708814)) {
        std::atexit(at_exit_shutdown);
        lifecycle<Byte>(8708814) = 1;
    }
    int status = create_output_device();
    if (status >= 0)
        start_backend_thread();
    return status;
}

int shutdown_backend() {
    stop_output_device();
    lock();
    release(backends);
    if (stream_bindings)
        release(stream_bindings);
    if (lifecycle<Byte>(8708816))
        for (unsigned i = 0; i < 2; ++i)
            if (lifecycle<void *>(8708820 + i * 4))
                release(lifecycle<void *>(8708820 + i * 4));
    unlock();
    return 0;
}

int initialize_audio(void *memory, Dword bytes) {
    author_tag = 'S';
    if (control.initialized)
        return 0;
    zero_memory(memory, bytes);
    initialize_heap(memory, bytes);
    if (!control.voice_count) {
        int status = load_configuration(configuration_bytes);
        if (status < 0)
            return status;
        apply_configuration(configuration_bytes);
    }
    seed_generator(setting<int>(8708892));
    create_mutex();
    lock();
    control.voices = static_cast<Voice *>(allocate(Dword(control.voice_count) << 7));
    control.banks = static_cast<Bank *>(allocate((setting<volatile Dword>(8708896) & 65535) << 3));
    unlock();
    control.unknown_c = 0;
    control.master_volume = 127;
    static_cast<volatile std::int8_t &>(control.filter_count) = 0;
    static_cast<volatile std::int8_t &>(control.producer_count) = 0;
    static_cast<volatile std::int8_t &>(control.pump_count) = 0;
    int status = initialize_backend();
    if (status < 0) {
        shutdown_backend();
        close_mutex();
        return status;
    }
    control.initialized = 1;
    field<std::uint16_t>(effect_groups, 52) = Byte(output_channels) == 2 ? 49152 : 57344;
    field<std::uint16_t>(effect_groups, 54) = Byte(output_channels) == 2 ? 16384 : 8192;
    field<std::uint16_t>(effect_groups, 68) = 57344;
    field<std::uint16_t>(effect_groups, 60) = setting<std::uint16_t>(8708996);
    field<std::uint16_t>(effect_groups, 62) = setting<std::uint16_t>(8708992);
    field<std::uint16_t>(effect_groups, 64) = setting<std::uint16_t>(8708994);
    field<std::uint16_t>(effect_groups, 70) = 8192;
    field<std::uint16_t>(effect_groups, 72) = 40960;
    field<std::uint16_t>(effect_groups, 74) = 24576;
    build_pan_table();
    return 0;
}

int shutdown_audio() {
    if (!initialized())
        return -14;
    configure_effect_groups(0, 0, 0, 4294967295u, 4294967295u);
    for (unsigned index : {0u, 1u, 2u, 5u, 6u, 4u})
        if (control.shutdown[index])
            control.shutdown[index]();
    stop_all();
    if (control.shutdown[3])
        reinterpret_cast<void(__cdecl *)(Dword)>(control.shutdown[3])(4294967295u);
    shutdown_backend();
    lock();
    release(control.voices);
    release(control.banks);
    unlock();
    int used = heap_used_percent();
    control.initialized = 0;
    close_mutex();
    return used;
}
} // namespace nfs::sound::backend
