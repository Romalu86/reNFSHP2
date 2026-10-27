#include "nfs/ai/lifecycle/ai_lifecycle.hpp"
#include "nfs/graphics/resources/eagl_graphics.hpp"
#include "nfs/world/lifecycle/world_lifecycle.hpp"
#include "nfs/core/frame/game_step.hpp"
#include <cstring>

namespace nfs::game::runtime::ai::lifecycle {

void activate_pursuer(void *entity, Dword reset) {
    invoke(entity, 6, Dword{1});
    invoke(entity, 46, Dword{0});
    void *ai = system(76);
    float range, distance;
    if (static_cast<Byte>(reset)) {
        range = at<float>(ai, 84580);
        distance = 0;
    } else {
        range = at<float>(ai, 84576);
        distance = 300.0f;
        if (range < 300.0f)
            distance = range;
    }
    Dword route;
    int node_index;
    std::memcpy(&route, &range, 4);
    if (pending::find_pursuit_spawn(at<void *>(entity, 664), 0, range, distance, &route,
                                    &node_index)) {
        float position[3], orientation[9];
        pending::sample_route_pose(reinterpret_cast<void *>(route), node_index, position,
                                   orientation, 0);
        pending::position_entity(entity, position, orientation, 0);
    }
    void *car = at<void *>(entity, 368);
    world::pending::reset_vehicle_renderable(at<void *>(car, 3372));
    reset_chase_contact(bytes(car) + 7952);
}

void update_path_states(Dword *controller) {
    for (int index = 0; index < 2; ++index) {
        void *record = &state<Byte>(7352496 + index * 116);
        const Dword phase = at<Dword>(record, 88);
        switch (phase) {
        case 1:
            pending::advance_chase_warmup(record);
            break;
        case 2:
            pending::update_police_chase(controller, record);
            break;
        case 3:
            if (at<int>(system(72), 176) == 1) {
                void *race = at<void *>(system(92), 4);
                const Dword slot = network::world::vehicle_slot(at<void *>(record, 44));
                if (pending::participant_enabled(race, slot)) {
                    race = at<void *>(system(92), 4);
                    const Dword enabled_slot = network::world::vehicle_slot(at<void *>(record, 44));
                    pending::set_participant_enabled(race, enabled_slot, 1);
                    game::start_deadline = graphics::unresolved::camera_state + 300;
                }
            }
            pending::complete_police_chase(controller, record, index);
            break;
        case 4:
            if (at<int>(system(72), 176) == 1) {
                void *race = at<void *>(system(92), 4);
                const Dword slot = network::world::vehicle_slot(at<void *>(record, 44));
                if (pending::participant_enabled(race, slot) &&
                    static_cast<int>(game::start_deadline -
                                     graphics::unresolved::camera_state) < 0) {
                    race = at<void *>(system(92), 4);
                    const Dword disabled_slot = network::world::vehicle_slot(at<void *>(record, 44));
                    pending::set_participant_enabled(race, disabled_slot, 0);
                }
            }
            break;
        }
    }
}
} // namespace nfs::game::runtime::ai::lifecycle
namespace nfs::game::runtime::ai::pending {
void activate_pursuer_entry(void *entity, Dword reset) {
    lifecycle::activate_pursuer(entity, reset);
}
void update_path_states_entry(Dword *owner) {
    lifecycle::update_path_states(owner);
}
} // namespace nfs::game::runtime::ai::pending
