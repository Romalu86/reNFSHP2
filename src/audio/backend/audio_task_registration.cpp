#include "nfs/audio/music/music_clients.hpp"
#include "nfs/audio/backend/sound_ownership.hpp"
#include "nfs/core/timing/timer.hpp"
namespace nfs::sound::streaming {

int stream_volume(Dword handle) {
    if (!control.initialized)
        return -10;
    Stream *stream = lookup(handle);
    if (!stream)
        return -8;
    int volume = voice_volume(Dword(stream->voice));
    if (volume < 0)
        volume = field<std::int8_t>(stream, 212);
    return volume;
}
} // namespace nfs::sound::streaming
namespace nfs::sound::backend {

Dword scheduled_pump(Dword, Dword) {
    pump();
    return 0;
}
// Confirmed single RET body at the stored callback address.
void release_pump_callback() {
}

int register_audio_tasks() {
    setting<Callback>(8709052) = release_pump_callback;
    if (!lifecycle<Dword>(8708808)) {
        kernel::task_add(scheduled_pump, 0, 1);
        lifecycle<Dword>(8708808) = 1;
    }
    kernel::register_exit(at_exit_shutdown);
    return 0;
}
} // namespace nfs::sound::backend
