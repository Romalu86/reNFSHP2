#include "nfs/physics/frame/simulation_runtime.hpp"
#include "nfs/core/frame/game_step.hpp"
namespace nfs::game::runtime::pending {
static Dword *section_object(Dword *owner, int section, int player) {
    return reinterpret_cast<Dword *>(bytes(owner) + (player + section * 2 + 1) * 168);
}
static void advance_player_sections(Dword *owner, int player, Dword flags) {
    track::advance_compartment(section_object(owner, at<int>(owner, 48 + player * 4), player),
                               flags);
    track::advance_compartment(section_object(owner, track::next_section(owner, player), player),
                               flags);
    track::advance_compartment(
        section_object(owner, track::previous_section(owner, player), player), flags);
}

void advance_track(Dword *owner, Dword flags) {
    track::advance_compartment(
        reinterpret_cast<Dword *>(bytes(owner) + at<int>(owner, 4) * 336 + 168), flags);
    for (int player = 0; player < at<int>(system(72), 176); ++player) {
        advance_player_sections(owner, player, flags);
        if (!Byte(flags))
            continue;
        Dword &phase = at<Dword>(owner, 68 + player * 4);
        if (!phase) {
            const int section = at<int>(owner, 56 + player * 4);
            phase = track::compartment_ready(section_object(owner, section, player)) != 0;
        } else if (phase == 1) {
            if (track::update_direction(owner, player)) {
                track::request_next_pair(owner, player);
                phase = 0;
                continue;
            }
            Dword position[3];
            for (unsigned axis = 0; axis < 3; ++axis)
                position[axis] = at<Dword>(tick_services[player], 288 + axis * 4);
            const float *point = reinterpret_cast<float *>(position);
            if (track::contains(static_cast<Dword *>(track::boundary(owner, 1, player)), point)) {
                phase = 1;
                continue;
            }
            bool crossed = false;
            if (at<Byte>(owner, 64 + player)) {
                crossed = track::contains(static_cast<Dword *>(track::boundary(owner, 0, player)),
                                          point) != 0;
                if (!crossed && !at<Byte>(owner, 64 + player))
                    crossed =
                        track::contains(static_cast<Dword *>(track::boundary(owner, 2, player)),
                                        point) != 0;
            } else
                crossed = track::contains(static_cast<Dword *>(track::boundary(owner, 2, player)),
                                          point) != 0;
            if (crossed) {
                track::rotate_compartments(owner, player);
                track::request_next_pair(owner, player);
                phase = 0;
            } else
                phase = 1;
        }
    }
}

void advance_replay_track_entry(Dword *owner, Dword flags) {
    track::advance_compartment(
        reinterpret_cast<Dword *>(bytes(owner) + at<int>(owner, 4) * 336 + 168), flags);
    for (int player = 0; player < at<int>(system(72), 176); ++player) {
        advance_player_sections(owner, player, flags);
        if (!Byte(flags) || at<Byte>(owner, 21752))
            continue;
        track::pending::advance_replay(owner, player);
        Dword position[3];
        for (unsigned axis = 0; axis < 3; ++axis)
            position[axis] = at<Dword>(tick_services[player], 288 + axis * 4);
        const float *point = reinterpret_cast<float *>(position);
        if (!track::contains(static_cast<Dword *>(track::boundary(owner, 1, player)), point) &&
            !track::contains(static_cast<Dword *>(track::boundary(owner, 0, player)), point) &&
            !track::contains(static_cast<Dword *>(track::boundary(owner, 2, player)), point)) {
            at<Dword>(system(96), 36) = 23;
            at<Byte>(owner, 21752) = 1;
        }
    }
}
} // namespace nfs::game::runtime::pending
