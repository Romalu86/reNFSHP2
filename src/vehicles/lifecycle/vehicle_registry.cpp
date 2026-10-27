#include "nfs/physics/frame/simulation_runtime.hpp"
#include "nfs/core/frame/game_step.hpp"
namespace nfs::game::runtime::physics::pending {

// scans exactly 40 flags. These are saved footprints, not inferred game limits.
alignas(4) Byte vehicle_flags[40]{};
alignas(4) Byte vehicle_states[40 * 736]{};
int last_vehicle{};
void reset_vehicle_entry(void *);
} // namespace nfs::game::runtime::physics::pending
namespace nfs::game::runtime::physics {

void *reserve_vehicle() {
    for (int i = 0; i < 40; ++i)
        if (pending::vehicle_flags[i]) {
            pending::vehicle_flags[i] = 0;
            if (pending::last_vehicle < i)
                pending::last_vehicle = i;
            Byte *state = pending::vehicle_states + i * 736;
            state[4] = Byte(i);
            return state;
        }
    return nullptr;
}
// Preserve the saved empty-registry case (no -1 assignment).
void recompute_last_vehicle() {
    const int last = pending::last_vehicle;
    for (int i = 0; i <= last; ++i)
        if (!pending::vehicle_flags[i])
            pending::last_vehicle = i;
}

void release_vehicle(void *vehicle) {
    const unsigned index = at<Byte>(vehicle, 4);
    pending::vehicle_states[index * 736 + 728] = 0;
    const int last = pending::last_vehicle;
    pending::vehicle_flags[index] = 1;
    if (index == Dword(last) && last >= 0)
        recompute_last_vehicle();
}

void reset_vehicle_registry() {
    pending::last_vehicle = 0;
    for (unsigned i = 0; i < 40; ++i) {
        Byte *state = pending::vehicle_states + i * 736;
        pending::reset_vehicle_entry(state);
        state[4] = Byte(i);
        pending::vehicle_flags[i] = 1;
    }
}

void *vehicle_car(void *state) {
    return at<Byte>(state, 5) ? nullptr : at<void *>(state, 0);
}

void *local_car(Dword *manager, int player) {
    const int index = at<int>(system(72), 204 + player * 4);
    if (index != -1)
        return at<void *>(manager, 20 + index * 4);
    if (tick_services[player])
        return at<void *>(tick_services[player], 300);
    return nullptr;
}

Byte is_local_vehicle(void *state) {
    for (int player = 0; player < 2; ++player) {
        void *car = local_car(system(68), player);
        if (car && state == at<void *>(car, 1768))
            return 1;
    }
    return 0;
}

Byte car_is_racer(void *car) {
    void *interface = at<void *>(car, 7988);
    if (!interface)
        return 1;
    return !invoke<Dword>(interface, 11) && !ai::racer(interface);
}
// AL is the consumed return value.
Byte car_is_remote(void *car) {
    void *interface = at<void *>(car, 7988);
    return interface && ai::kind(interface) == 6;
}
 
Byte split_screen() {
    return 0;
}
} // namespace nfs::game::runtime::physics
