#include "nfs/world/lifecycle/world_lifecycle.hpp"
#include "nfs/physics/frame/simulation_runtime.hpp"
#include "nfs/core/timing/frame_clock.hpp"
#include "../../race/rules/race_rules_x87.hpp"
namespace nfs::game::runtime::track {

int following_pair(Dword *owner, int player) {
    const int current = at<int>(owner, 48 + player * 4);
    if (current == int(at<Dword>(owner, 4) - 1))
        return 1;
    if (current == int(at<Dword>(owner, 4) - 2))
        return 0;
    return int(Dword(current) + 2);
}
} // namespace nfs::game::runtime::track
namespace nfs::game::world::pending {

void initialize_track_positions(void *owner) {
    using namespace runtime::track;
    auto *track = static_cast<Dword *>(owner);
    if (at<Byte>(track, 8)) {
        at<Byte>(track, 8) = 0;
        return;
    }
    for (int player = 0; player < at<int>(subsystem(72), 176); ++player) {
        auto compartment = [&](int section) {
            return reinterpret_cast<Dword *>(bytes(track) + (player + section * 2 + 1) * 168);
        };
        const unsigned buffers = 21672 + player * 4;
        runtime::track::pending::load_compartment_entry(
            compartment(previous_section(track, player)), at<Dword>(track, buffers));
        runtime::track::pending::load_compartment_entry(
            compartment(at<int>(track, 48 + player * 4)), at<Dword>(track, buffers + 8));
        runtime::track::pending::load_compartment_entry(compartment(next_section(track, player)),
                                                           at<Dword>(track, buffers + 16));
        open_compartment(compartment(following_pair(track, player)), at<Dword>(track, buffers + 24));
        runtime::track::pending::bind_compartment(
            compartment(previous_section(track, player)), at<Dword>(track, buffers));
        runtime::track::pending::bind_compartment(
            compartment(at<int>(track, 48 + player * 4)), at<Dword>(track, buffers + 8));
        runtime::track::pending::bind_compartment(compartment(next_section(track, player)),
                                                           at<Dword>(track, buffers + 16));
        while (!runtime::track::compartment_ready(compartment(following_pair(track, player)))) {
            const Dword clock = application::race_rules_x87::truncate(
                static_cast<long double>(frame_clock::elapsed) * 60.f);
            if (clock & 2)
                kernel::cooperate(0);
        }
        at<int>(track, 56 + player * 4) = following_pair(track, player);
    }
}
} // namespace nfs::game::world::pending
