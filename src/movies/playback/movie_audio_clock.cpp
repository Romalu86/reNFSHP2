#include "movie_runtime.hpp"
namespace nfs::sound::backend {

static int output_buffer_latency() {
    if (!direct_sound)
        return 0;
    Dword position;
    if (device_call<int>(platform<void *>(8711376), 16, &position, nullptr))
        return 0;
    Dword samples = position >> 1;
    if (!setting<Byte>(8708920))
        samples /= setting<Dword>(8708928) & 255;
    int pending = int(platform<Dword>(8711412) - samples);
    if (pending < 0)
        pending = int(Dword(pending) + platform<Dword>(8711452));
    return int(Dword(pending) * 1000) / int(setting<std::uint16_t>(8708898));
}

static int locked_output_latency() {
    lock();
    int result = output_buffer_latency();
    unlock();
    return result;
}
} // namespace nfs::sound::backend
namespace nfs::sound::streaming {

int output_latency() {
    return control.initialized ? backend::locked_output_latency() : 0;
}
} // namespace nfs::sound::streaming
