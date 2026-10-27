#include "nfs/physics/frame/simulation_runtime.hpp"
#include "nfs/ai/lifecycle/ai_lifecycle.hpp"
#include <cstdlib>
namespace nfs::game::runtime::ai {

static Dword (&collision_words)[9]=*reinterpret_cast<Dword(*)[9]>(lifecycle::storage+336);
static Byte &initialized=lifecycle::storage[372];

void clear_collision_state() {
    for (int i = 0; i < static_cast<int>(collision_words[2]); ++i)
        if (collision_words[i])
            collision_words[i] = 0;
    for (int i = 0; i < static_cast<int>(collision_words[8]); ++i)
        if (collision_words[3 + i])
            collision_words[3 + i] = 0;
    collision_words[2] = collision_words[8] = 0;
}

Dword *collision_state() {
    if (!(initialized & 1)) {
        initialized |= 1;
        collision_words[0] = collision_words[2] = collision_words[1] = 0;
        std::atexit(clear_collision_state);
    }
    return collision_words;
}

Dword *reset_collision_header(Dword *owner) {
    owner[2] = 0;
    owner[0] = 0;
    owner[1] = 0;
    return owner;
}
 
void begin_collision_pass(Dword *) {
}

void finish_collision_pass(Dword *owner) {
    const int count = static_cast<int>(owner[2]);
    for (int i = 0; i < count; ++i) {
        Dword *vehicle = reinterpret_cast<Dword *>(owner[i]);
        if (vehicle && at<Byte>(vehicle, 212))
            pending::update_collision_vehicle_entry(vehicle);
    }
}
} // namespace nfs::game::runtime::ai
