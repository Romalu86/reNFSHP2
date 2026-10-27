#include "nfs/audio/music/music_clients.hpp"
#include <cstring>
namespace nfs::game::audio {
using namespace state;

void initialize_race_listeners() {
    using namespace runtime::race_audio::pending;
    listener_configuration = nullptr;
    std::memset(event_flags, 0, 248);
    unsigned listener = Dword(listener_index) * 2760;
    world<Dword>(632 + listener) = world<Dword>(828 + listener) = 0;
    for (unsigned channel = 0; channel < 12; ++channel)
        world<Dword>(2368 + listener + channel * 56) = 10000;
    delayed_count = 0;
    for (unsigned event = 0; event < 20; ++event)
        world<Dword>(6208 + event * 32) = Dword(-1);
    initialize_volume_fades();
    world<Dword>(3080) = world<Dword>(5840) = 10000;
    world<Byte>(3108) = world<Byte>(5868) = 0;
    for (unsigned slot = 0; slot < 12; ++slot) {
        world<Dword>(636 + slot * 4) = world<Dword>(3396 + slot * 4) = Dword(-1);
        for (unsigned axis = 0; axis < 3; ++axis) {
            world<Dword>(684 + slot * 12 + axis * 4) = 0;
            world<Dword>(3444 + slot * 12 + axis * 4) = 0;
        }
    }
    for (unsigned wheel = 0; wheel < 4; ++wheel) {
        for (unsigned offset : {0u, 12u, 20u}) {
            world<Dword>(356 + wheel * 28 + offset) = 0;
            world<Dword>(3116 + wheel * 28 + offset) = 0;
        }
    }
    race_audio_listener_count = sound::field<int>(lifecycle::manager->slots[18], 176);
}
} // namespace nfs::game::audio
