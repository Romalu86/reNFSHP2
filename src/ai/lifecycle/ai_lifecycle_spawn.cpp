#include "nfs/ai/lifecycle/ai_lifecycle.hpp"
#include "nfs/world/lifecycle/world_lifecycle.hpp"
#include "nfs/graphics/resources/eagl_graphics.hpp"
#include "../../math/vectors/retained_vector.hpp"
#include <cmath>
#include <cstring>
#include <initializer_list>

#include "../population/ai_population_x87.hpp"
namespace nfs::game::runtime::ai::lifecycle {

Byte player_within_range(Dword *owner, const float *point, float range) {
    return pending::population_frame::player_within_range(owner, point, range);
}

Byte retain_path_vehicle(void *entity) {
    const auto *point = invoke<const float *>(entity, 0);
    Byte nearby = player_within_range(system(76), point, pending::spawn_distance);
    if (state<Dword>(7352732))
        nearby = 0;
    return nearby;
}

Byte release_path_reservation(void *reservations, void *entity) {
    for (Dword index = 0; index < at<Dword>(reservations, 8); ++index) {
        void *entry = bytes(at<void *>(reservations, 12)) + index * 40;
        if (at<void *>(entry, 28) != entity)
            continue;
        at<Dword>(entry, 32) = graphics::unresolved::camera_state;
        at<Dword>(entry, 28) = 0;
        --at<Dword>(reservations, 24);
        ++at<Dword>(reservations, 16);
        return 1;
    }
    return 0;
}

Byte spawn_path_vehicle(Dword *controller, Dword *entity) {
    if (entity[143] && retain_path_vehicle(entity)) {
        if (entity[143] != 5)
            return 0;
        dispatch_cop_event(entity, 10);
        return 1;
    }
    if (entity[143] == 2)
        release_path_reservation(bytes(controller) + 92, entity);
    invoke(entity, 6, Dword{0});
    if (entity[143]) {
        dispatch_cop_event(entity, 3);
        --at<std::uint16_t>(controller, 84);
    }
    for (unsigned index = 0; index < 7; ++index)
        entity[453 + index] = 0;
    at<Byte>(entity, 1840) = 1;
    entity[461] = 4294967295u;
    auto *records = reinterpret_cast<void *>(bytes(controller) + 92);
    void *reservation = invoke<void *>(records, 2, entity);
    if (reservation && at<void *>(reservation, 12)) {
        const auto *marker = at<Dword *>(reservation, 12);
        entity[141] = state<Dword>(7352444)++;
        ++entity[454];
        for (unsigned index = 0; index < 5; ++index)
            entity[455 + index] = 0;
        entity[453] = at<Dword>(reservation, 36);
        at<Byte>(entity, 1840) = 1;
        entity[461] = 4294967295u;
        float orientation[9];
        world::quaternion_matrix(reinterpret_cast<const float *>(marker + 3), orientation);
        for (unsigned index : {2, 5, 6, 7})
            orientation[index] *= -1.0f;
        pending::position_entity(entity, reinterpret_cast<const float *>(marker), orientation, 0);
        Dword *cars = system(68);
        const Dword car_index = at<Dword>(reinterpret_cast<void *>(entity[92]), 1772);
        void *descriptor = car_index < 24 ? bytes(system(72)) + 248 + car_index * 84 : nullptr;
        const Dword player = at<Dword>(descriptor, 20);
        if (player < 8) {
            std::memcpy(bytes(cars) + 700 + player * 12, marker, 12);
            std::memcpy(bytes(cars) + 988 + player * 36, orientation, 36);
        }
        if (entity[143] == 0) {
            invoke(entity, 6, Dword{1});
            ++at<std::uint16_t>(controller, 84);
        }
        dispatch_cop_event(entity, 10);
        return 1;
    }
    invoke(entity, 6, Dword{0});
    if (entity[143]) {
        dispatch_cop_event(entity, 3);
        --at<std::uint16_t>(controller, 84);
    }
    return 0;
}

Byte reset_cop_vehicle(Dword *controller, Dword *entity) {
    invoke(entity, 6, Dword{0});
    if (entity[143]) {
        dispatch_cop_event(entity, 3);
        --at<std::uint16_t>(controller, 84);
    }
    if (at<Byte>(entity, 576)) {
        invoke(entity, 6, Dword{0});
        if (entity[143]) {
            dispatch_cop_event(entity, 3);
            --at<std::uint16_t>(controller, 84);
        }
        return 1;
    }
    return spawn_path_vehicle(controller, entity);
}
} // namespace nfs::game::runtime::ai::lifecycle
namespace nfs::game::runtime::ai::pending {
Byte spawn_path_vehicle_entry(Dword *controller, Dword *entity) {
    return lifecycle::spawn_path_vehicle(controller, entity);
}
} // namespace nfs::game::runtime::ai::pending
namespace nfs::game::runtime::ai::lifecycle::pending {
Byte reset_cop(Dword *controller, Dword *entity) {
    return reset_cop_vehicle(controller, entity);
}
} // namespace nfs::game::runtime::ai::lifecycle::pending
