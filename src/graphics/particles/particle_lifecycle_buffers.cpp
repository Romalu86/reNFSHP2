#include "nfs/graphics/particles/particle_lifecycle.hpp"
namespace nfs::game::runtime::particles::lifecycle {

Dword *construct_particle_buffer(Dword *buffer) {
    for (unsigned i = 0; i < 1000; ++i)
        at<Dword>(buffer, 64 + i * 68) = 0;
    at<Dword>(buffer, 72000) = 0;
    for (unsigned i = 0; i < 1000; ++i)
        at<void *>(buffer, 68000 + i * 4) = bytes(buffer) + i * 68;
    return buffer;
}

void release_wheel_skid(void *car, int wheel) {
    if (!at<Byte>(car, 1695 + wheel))
        return;
    if (void *skid = at<void *>(car, 2240 + wheel * 296))
        mark_skid_finished(skid);
    at<Dword>(car, 2240 + wheel * 296) = 0;
    at<Byte>(car, 1695 + wheel) = at<Byte>(car, 1716 + wheel) = 0;
}

void mark_skid_finished(void *skid) {
    at<Byte>(skid, 72) = 1;
}
} // namespace nfs::game::runtime::particles::lifecycle
