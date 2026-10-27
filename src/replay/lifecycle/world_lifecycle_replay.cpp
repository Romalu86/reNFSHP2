#include "nfs/world/lifecycle/world_lifecycle.hpp"
#include "nfs/physics/frame/simulation_runtime.hpp"
namespace nfs::game::world {
namespace {
Dword *compartment(void *owner, int section, int player) {
    const Dword record = Dword(player) + Dword(section) * 2u + 1u;
    return reinterpret_cast<Dword *>(offset(owner, record * 168u));
}
int following_pair(void *owner, int player) {
    const Dword current = at<Dword>(owner, 48u + Dword(player) * 4u);
    if (!at<Byte>(owner, 64u + Dword(player)))
        return runtime::track::previous_pair(static_cast<Dword *>(owner), player);
    const Dword count = at<Dword>(owner, 4);
    if (current == count - 1u)
        return 1;
    if (current == count - 2u)
        return 0;
    return std::int32_t(current + 2u);
}
} // namespace

int compartment_ready(void *owner, int player) {
    return runtime::track::compartment_ready(
               compartment(owner, at<int>(owner, 56u + Dword(player) * 4u), player)) != 0;
}


void advance_replay_track(void *owner, int player) {
    int desired;
    Byte direction;
    read_replay_section(subsystem(132), player, &desired, &direction);
    const Dword position_offset = Dword(player) * 4u;
    const Dword direction_offset = 64u + Dword(player);
    const Dword current = at<Dword>(owner, 48u + position_offset);
    if (current == Dword(desired)) {
        if (!at<Dword>(owner, 68u + position_offset)) {
            int ready = compartment_ready(owner, player);
            at<Dword>(owner, 68u + position_offset) = Dword(ready);
            if (!ready)
                return;
        }
        if (at<Byte>(owner, direction_offset) == direction)
            return;
        runtime::track::direction_changed(
            compartment(owner, following_pair(owner, player), player));
        at<Byte>(owner, direction_offset) = direction;
    } else {
        const Dword count = at<Dword>(owner, 4);
        const Dword next = current == count - 1u ? 0u : current + 1u;
        const Dword previous = current ? current - 1u : count - 1u;
        if (Dword(desired) != next && Dword(desired) != previous) {
            at<Byte>(owner, 21752) = 1;
            at<Dword>(subsystem(96), 36) = 23;
            return;
        }
        if (!at<Dword>(owner, 68u + position_offset))
            while (!compartment_ready(owner, player))
                kernel::cooperate(0);
        if (at<Byte>(owner, direction_offset) != direction)
            runtime::track::pending::reset_compartment(
                compartment(owner, following_pair(owner, player), player));
        at<Byte>(owner, direction_offset) = direction;
        // Compartment callbacks can replace the track's count. The path reads it again at each boundary
        // below, after the preceding store.
        if (!direction) {
            const Dword last = at<volatile Dword>(owner, 4) - 1u;
            at<volatile Dword>(owner, 48u + position_offset) =
                Dword(desired) == last ? 0u : Dword(desired) + 1u;
            at<volatile Dword>(owner, 56u + position_offset) =
                desired == 0 ? at<volatile Dword>(owner, 4) - 1u : Dword(desired) - 1u;
        } else {
            at<volatile Dword>(owner, 48u + position_offset) =
                (desired ? Dword(desired) : at<volatile Dword>(owner, 4)) - 1u;
            const Dword last = at<volatile Dword>(owner, 4) - 1u;
            at<volatile Dword>(owner, 56u + position_offset) =
                Dword(desired) == last ? 0u : Dword(desired) + 1u;
        }
        runtime::track::rotate_compartments(static_cast<Dword *>(owner), player);
    }
    int next = following_pair(owner, player);
    at<int>(owner, 56u + position_offset) = next;
    runtime::track::open_compartment(compartment(owner, next, player),
                                     at<Dword>(owner, 21696u + position_offset));
    at<Dword>(owner, 68u + position_offset) = 0;
}
} // namespace nfs::game::world
