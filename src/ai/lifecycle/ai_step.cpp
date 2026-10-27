#include "nfs/physics/frame/simulation_runtime.hpp"
#include "nfs/core/frame/game_step.hpp"
#include "nfs/graphics/resources/eagl_graphics.hpp"
#include <cmath>
#include <climits>
namespace nfs::game::runtime::ai {

void assign_targets(Dword *owner, Dword reset) {
    const int count = at<int>(owner, 84172);
    if (count <= 0)
        return;
    int assigned[2]{}, police_count = 0;
    for (int i = 0; i < count; ++i) {
        void *entry = at<void *>(owner, 84176 + i * 4);
        if (!entry)
            continue;
        void *cop = police(entry);
        if (!cop)
            continue;
        ++police_count;
        if (at<Byte>(cop, 212))
            for (int player = 0; player < 2; ++player)
                if (at<Dword>(owner, 84164 + player * 4) == at<Dword>(cop, 664)) {
                    ++assigned[player];
                    break;
                }
    }
    if (!police_count)
        return;
    for (int player = 0; player < 2; ++player) {
        void *target = at<void *>(owner, 84164 + player * 4);
        if (!target || assigned[player] >= 1)
            continue;
        target = racer(target);
        if (!target)
            continue;
        void *selected = nullptr;
        int oldest = INT_MAX;
        for (int i = 0; i < count; ++i) {
            void *entry = at<void *>(owner, 84176 + i * 4);
            if (!entry)
                continue;
            void *cop = police(entry);
            if (cop && !at<Byte>(cop, 212) && at<int>(cop, 652) < oldest) {
                oldest = at<int>(cop, 652);
                selected = cop;
            }
        }
        if (selected) {
            at<void *>(selected, 664) = target;
            pending::activate_pursuer_entry(selected, reset);
        }
    }
}

void advance_pursuit(Dword *owner) {
    const int count = at<int>(owner, 4);
    if (!count)
        return;

    const int delay = static_cast<int>(static_cast<long double>(at<float>(owner, 44)) * -60.0L);
    const Dword deadline = at<Dword>(owner, 40) - Dword(delay);
    if (static_cast<int>(graphics::unresolved::camera_state) <
            static_cast<int>(deadline) ||
        count <= 0)
        return;
    int available = 0;
    for (int i = 0; i < count; ++i)
        if (!at<Dword>(at<void *>(owner, 8 + i * 4), 568))
            ++available;
    if (!available)
        return;
    const Dword first = random_word() & 1;
    for (Dword attempt = 0; attempt < 2; ++attempt) {
        void *target = at<void *>(system(76), 84164 + ((first + attempt) % 2) * 4);
        if (!target)
            continue;
        for (int i = 0; i < count; ++i)
            if (pending::assign_pursuer_entry(at<void *>(owner, 8 + i * 4), target)) {
                at<Dword>(owner, 40) = graphics::unresolved::camera_state;
                return;
            }
    }
}
} // namespace nfs::game::runtime::ai
namespace nfs::game::runtime::pending {

void prepare_ai(Dword *owner) {
    update_ai_paths(ai::pending::path_controller);
    ai::pending::advance_pairs(network::world::vehicle_registry);
    ai::rank_racers(owner);
    ai::advance_pursuit(owner + 84344 / 4);
    ai::assign_targets(owner, 0);
    ai::begin_collision_pass(ai::collision_state());
}

void finish_ai(Dword *) {
    ai::finish_collision_pass(ai::collision_state());
}

void update_ai_paths(Dword *owner) {
    if (ai::pending::path_deadline <=
        static_cast<int>(graphics::unresolved::camera_state)) {
        Dword count = at<Dword>(owner, 112) - at<Dword>(owner, 116);
        if (at<Dword>(owner, 108) <= count)
            count = at<Dword>(owner, 108);
        for (Dword i = 0; i < count; ++i) {
            const unsigned capacity = at<std::uint16_t>(owner, 80);
            unsigned index = 0;
            while (index < capacity && at<Dword>(at<Dword *>(owner, index * 4), 572))
                ++index;
            if (index == capacity)
                break;
            Dword *vehicle = at<Dword *>(owner, index * 4);
            invoke(vehicle, 6, Dword(0));
            if (at<Dword>(vehicle, 572)) {
                ai::pending::dispatch_vehicle_event(vehicle, 3);
                --at<std::uint16_t>(owner, 84);
            }
            if (!at<Byte>(vehicle, 576)) {
                if (!ai::pending::spawn_path_vehicle_entry(owner, vehicle))
                    break;
            } else {
                invoke(vehicle, 6, Dword(0));
                if (at<Dword>(vehicle, 572)) {
                    ai::pending::dispatch_vehicle_event(vehicle, 3);
                    --at<std::uint16_t>(owner, 84);
                }
            }
        }
    }
    ai::pending::update_path_states_entry(owner);
}
// Entry overwrites EAX: the caller's EAX count is not an input.
void update_ai_contacts(Dword *owner, Dword) {
    network::world::Iterator iterator;
    network::world::construct_iterator(&iterator, at<Dword *>(owner, 4));
    Dword value = network::world::first(&iterator);
    if (at<Byte>(owner, 132)) {
        while (!network::world::iterator_done(&iterator)) {
            void *vehicle = reinterpret_cast<void *>(value);
            if (at<Byte>(vehicle, 212))
                invoke(vehicle, 4, Dword(0));
            value = network::world::next(&iterator);
        }
        return;
    }
    int previous_kind = 10;
    Dword stride = 0, ordinal = 0, phase = 0;
    for (; value; value = network::world::next(&iterator)) {
        void *vehicle = reinterpret_cast<void *>(value);
        if (ai::kind(vehicle) == 6 && at<Byte>(vehicle, 212)) {
            invoke(vehicle, 4, Dword(0));
            ++ordinal;
            continue;
        }
        if (ai::kind(vehicle) != previous_kind) {
            previous_kind = ai::kind(vehicle);
            stride = at<Dword>(owner, (previous_kind * 3 + 3) * 4 + 8);
            ordinal = 0;
            if (!stride)
                stride = 1;
            phase = at<Dword>(owner, 8) % stride;
        }
        if (at<Byte>(vehicle, 212)) {
            if (stride <= 1 || phase == ordinal % stride)
                invoke(vehicle, 4, Dword(0));
            ++ordinal;
        }
    }
    ++at<Dword>(owner, 8);
}
} // namespace nfs::game::runtime::pending
