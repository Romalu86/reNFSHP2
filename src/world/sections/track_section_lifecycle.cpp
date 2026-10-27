#include "nfs/physics/frame/simulation_runtime.hpp"
#include "nfs/resources/files/filesystem.hpp"
#include "nfs/core/frame/game_step.hpp"
#include "../../race/rules/race_rules_x87.hpp"
#include "../../physics/motion/world_motion_x87.hpp"
#include <cstdio>
#include <cmath>
namespace nfs::game::runtime::track {
 
void direction_changed(Dword *) {
}

Byte compartment_ready(Dword *compartment) {
    if (at<Byte>(compartment, 72))
        return 1;
    const Dword request = at<Dword>(compartment, 60);
    if (files::async::poll(request) != 1)
        return 0;
    Dword size = 0;
    files::async::take(request, nullptr, &size);
    at<Dword>(compartment, 60) = 0;
    at<Byte>(compartment, 72) = 1;
    return 1;
}
// The formatter uses a 260-byte local buffer.
void open_compartment(Dword *compartment, Dword destination) {
    char path[260];
    std::sprintf(path, "%s%s\\comp%02d.viv", bytes(system(4)) + 524, bytes(system(40)) + 140,
                 compartment[0]);
    at<Dword>(compartment, 68) = destination;
    at<Dword>(compartment, 60) =
        files::async::load_into(path, reinterpret_cast<void *>(destination), 0);
    at<Byte>(compartment, 72) = 0;
}

Byte update_direction(Dword *owner, int player) {
    Byte direction = 1;
    if (void *vehicle = at<void *>(tick_services[player], 300)) {
        void *interface = at<void *>(vehicle, 7988);
        long double value;
        x87::vehicle_direction_native80(interface, &value);
        if (value < -1.f)
            direction = 0;
    }
    if (at<Byte>(owner, 64 + player) == direction)
        return 0;
    at<Byte>(owner, 64 + player) = direction;
    int section = at<int>(owner, 48 + player * 4);
    if (direction)
        section = previous_pair(owner, player);
    else if (section == at<int>(owner, 4) - 1)
        section = 1;
    else if (section == at<int>(owner, 4) - 2)
        section = 0;
    else
        section += 2;
    direction_changed(reinterpret_cast<Dword *>(bytes(owner) + (player + section * 2 + 1) * 168));
    return 1;
}

void request_next_pair(Dword *owner, int player) {
    int section = at<int>(owner, 48 + player * 4);
    if (!at<Byte>(owner, 64 + player))
        section = previous_pair(owner, player);
    else if (section == at<int>(owner, 4) - 1)
        section = 1;
    else if (section == at<int>(owner, 4) - 2)
        section = 0;
    else
        section += 2;
    at<int>(owner, 56 + player * 4) = section;
    auto *compartment = reinterpret_cast<Dword *>(bytes(owner) + (player + section * 2 + 1) * 168);
    open_compartment(compartment, at<Dword>(owner, 21696 + player * 4));
}

void rotate_compartments(Dword *owner, int player) {
    const int next = at<int>(owner, 56 + player * 4);
    auto *incoming = reinterpret_cast<Dword *>(bytes(owner) + (player + next * 2 + 1) * 168);
    if (!at<Byte>(incoming, 72))
        pending::load_compartment_entry(incoming, at<Dword>(owner, 21696 + player * 4));
    pending::bind_compartment(incoming, at<Dword>(owner, 21696 + player * 4));
    if (at<Byte>(owner, 64 + player)) {
        const int previous = previous_section(owner, player);
        pending::reset_compartment(
            reinterpret_cast<Dword *>(bytes(owner) + (player + previous * 2 + 1) * 168));
        const Dword spare = at<Dword>(owner, 21672 + player * 4);
        at<Dword>(owner, 21672 + player * 4) = at<Dword>(owner, 21680 + player * 4);
        at<Dword>(owner, 21680 + player * 4) = at<Dword>(owner, 21688 + player * 4);
        at<Dword>(owner, 21688 + player * 4) = at<Dword>(owner, 21696 + player * 4);
        at<Dword>(owner, 21696 + player * 4) = spare;
        at<int>(owner, 48 + player * 4) = next_section(owner, player);
    } else {
        const int next_section_index = next_section(owner, player);
        pending::reset_compartment(
            reinterpret_cast<Dword *>(bytes(owner) + (player + next_section_index * 2 + 1) * 168));
        const Dword spare = at<Dword>(owner, 21688 + player * 4);
        const Dword first = at<Dword>(owner, 21672 + player * 4);
        at<Dword>(owner, 21688 + player * 4) = at<Dword>(owner, 21680 + player * 4);
        at<Dword>(owner, 21672 + player * 4) = at<Dword>(owner, 21696 + player * 4);
        at<Dword>(owner, 21680 + player * 4) = first;
        at<Dword>(owner, 21696 + player * 4) = spare;
        at<int>(owner, 48 + player * 4) = previous_section(owner, player);
    }
}

void advance_compartment(Dword *compartment, Dword flags) {
    if (Dword *particles = at<Dword *>(compartment, 160))
        runtime::pending::update_world_particles(particles, flags);
}
} // namespace nfs::game::runtime::track
