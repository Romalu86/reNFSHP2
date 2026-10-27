#include "../vehicles/remote_car_runtime.hpp"
#include "nfs/world/lifecycle/world_lifecycle.hpp"
namespace nfs::game::world::camera {

void refresh_target(void *director) {
    if (void *provider = at<void *>(director, 300)) {
        invoke(provider, 0, offset(director, 288));
        return;
    }
    void *state = invoke<void *>(director, 2);
    const void *target = state ? offset(state, 16) : offset(subsystem(68), 784);
    const Dword position[3] = {at<Dword>(target, 0), at<Dword>(target, 4), at<Dword>(target, 8)};
    for (unsigned axis = 0; axis < 3; ++axis)
        at<Dword>(director, 288 + axis * 4) = position[axis];
}
} // namespace nfs::game::world::camera
namespace nfs::game::runtime::track {

void select_network_section(Dword *owner, int player, int section) {
    if (at<int>(owner, 48 + Dword(player) * 4) == section)
        return;
    auto compartment = [&](int index) {
        return reinterpret_cast<Dword *>(bytes(owner) + (Dword(player) + Dword(index) * 2 + 1) * 168);
    };
    direction_changed(compartment(previous_pair(owner, player)));
    direction_changed(compartment(previous_section(owner, player)));
    direction_changed(compartment(at<int>(owner, 48 + Dword(player) * 4)));
    direction_changed(compartment(next_section(owner, player)));
    direction_changed(compartment(following_pair(owner, player)));
    pending::reset_compartment(compartment(previous_section(owner, player)));
    pending::reset_compartment(compartment(at<int>(owner, 48 + Dword(player) * 4)));
    pending::reset_compartment(compartment(next_section(owner, player)));
    at<int>(owner, 48 + Dword(player) * 4) = section;
    at<Byte>(owner, 64 + Dword(player)) = 1;
    at<Dword>(owner, 68 + Dword(player) * 4) = 1;
    world::pending::initialize_track_positions(owner);
}
} // namespace nfs::game::runtime::track
