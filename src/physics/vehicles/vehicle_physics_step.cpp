#include "nfs/physics/frame/simulation_runtime.hpp"
#include "car_physics_frame.hpp"
#include "car_drive_frame_x87.hpp"
#include <cmath>
#include <algorithm>
namespace nfs::game::runtime::pending {

void prepare_vehicle_states() {
    for (int i = 0; i <= physics::pending::last_vehicle; ++i) {
        Byte *state = physics::pending::vehicle_states + i * 736;
        if (!physics::pending::vehicle_flags[i] && state[728])
            prepare_vehicle(state);
    }
}

void step_vehicle_physics(Dword full_step) {
    using namespace physics::pending;
    using physics::car_frame::drive::retained;
    if (full_step)
        for (int i = 0; i <= last_vehicle; ++i)
            if (!vehicle_flags[i])
                kernel::memory_zero(vehicle_states + i * 736 + 604, 72);
    for (int i = 0; i <= last_vehicle; ++i) {
        Byte *state = vehicle_states + i * 736;
        if (!vehicle_flags[i] && state[728]) {
            if (!state[5])
                advance_car(state);
            else if (full_step)
                advance_body(state);
        }
    }
    if (!full_step)
        return;
    for (int i = 0; i <= last_vehicle; ++i) {
        Byte *state = vehicle_states + i * 736;
        if (vehicle_flags[i] || !state[728] || !state[728])
            continue;
        auto &contacts = at<std::uint16_t>(state, 596);
        const float sleep = at<float>(state, 60);
        if (sleep == 0) {
            bool test_wheels;
            if (!contacts) {
                if (state[5]) {
                    if (!collide_shape(state))
                        contacts = 1;
                    test_wheels = !state[5];
                } else
                    test_wheels = true;
            } else {
                ++contacts;
                test_wheels = !state[5];
            }
            if (test_wheels) {
                if (collide_wheels_entry(physics::vehicle_car(state)))
                    contacts = 0;
                else if (!contacts)
                    contacts = 1;
            }
            const auto clearance = retained(at<float>(state, 484)) - at<float>(state, 132);
            if (clearance < retained(0.6f) || __builtin_isnan(clearance)) {
                if (contacts && collide_ground(state)) {
                    const Dword material = at<Dword>(state, 40);
                    if (collide_shape(state))
                        contacts = 0;
                    const float denominator = at<float>(state, 660);
                    const auto volume =
                        denominator != 0 ? retained(at<float>(state, 608)) / retained(denominator)
                                         : retained(at<float>(state, 608)) * at<float>(state, 36);
                    submit_impact_entry(reinterpret_cast<float *>(state + 680), material, 19,
                                           float(volume * 0.5f), 0);
                }
            } else if (!contacts)
                contacts = 1;
        }
        if (!collision_disabled && !at<Dword>(state, 732)) {
            for (int other_index = i + 1; other_index <= last_vehicle; ++other_index) {
                Byte *other = vehicle_states + other_index * 736;
                if (vehicle_flags[other_index] || !other[728] || at<Dword>(other, 732))
                    continue;
                const float radius = float(retained(at<float>(other, 132)) + at<float>(state, 132));
                bool adjacent = true;
                for (unsigned axis : {688u, 680u, 684u}) {
                    auto difference = retained(at<float>(state, axis)) - at<float>(other, axis);
                    if (difference < retained(0))
                        difference = -difference;
                    if (!(difference < retained(radius))) {
                        adjacent = false;
                        break;
                    }
                }
                if (!adjacent)
                    continue;
                bool collided = false;
                for (int attempt = 30;; --attempt) {
                    if (!collide_pair(state, other))
                        break;
                    collided = true;
                    if (!attempt)
                        break;
                }
                if (collided) {
                    float normal[3] = {-collision_normal[0], -collision_normal[1],
                                       -collision_normal[2]};
                    hardware::normalize_collision(normal);
                    resolve_pair(state, other, collision_point, normal);
                }
            }
        }

        // its unwritten upper bytes are not invented as observable C++ state.
        Byte integrate = 1;
        if (!state[5] && at<void *>(state, 0) && physics::car_is_racer(at<void *>(state, 0)))
            integrate = 0;
        update_contacts_entry(state, integrate);
        finish_body(state);
    }
}
static void visit_cell(Byte *cell) {
    if (!cell)
        return;
    Dword *node = at<Dword *>(cell, 16);
    while (node) {
        Dword *current = node;
        node = at<Dword *>(node, 8);
        invoke(current, 3);
    }
}
// ECX is not used as a state argument by the body.
void update_track_collision(Dword *) {
    using namespace physics;
    physics::pending::begin_track_contacts_entry(system(48));
    for (int i = 0; i <= physics::pending::last_vehicle; ++i) {
        Byte *state = physics::pending::vehicle_states + i * 736;
        if (!physics::pending::vehicle_flags[i] && state[728] && state[5] == 1 &&
            at<void *>(state, 0))
            physics::pending::update_track_car_entry(at<void *>(state, 0));
    }
    visit_cell(global_cell());
    int x0 = 0, z0 = 0;
    void *car = local_car(system(68), 0);
    cell_coordinates(reinterpret_cast<const float *>(bytes(at<void *>(car, 1768)) + 680), &x0,
                     &z0);
    int x1 = x0, z1 = z0;
    if (split_screen()) {
        car = local_car(system(68), 1);
        cell_coordinates(reinterpret_cast<const float *>(bytes(at<void *>(car, 1768)) + 680),
                         &x1, &z1);
    }
    for (int z = (std::min)(z0 - 3, z1 - 3); z <= (std::max)(z0 + 3, z1 + 3); ++z)
        for (int x = (std::min)(x0 - 3, x1 - 3); x <= (std::max)(x0 + 3, x1 + 3); ++x)
            if ((x0 - 3 <= x && x <= x0 + 3 && z0 - 3 <= z && z <= z0 + 3) ||
                (x1 - 3 <= x && x <= x1 + 3 && z1 - 3 <= z && z <= z1 + 3))
                visit_cell(cell(x, z));
    reinsert_moved_nodes();
}
} // namespace nfs::game::runtime::pending
