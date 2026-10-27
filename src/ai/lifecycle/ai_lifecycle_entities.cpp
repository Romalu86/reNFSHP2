#include "nfs/ai/lifecycle/ai_lifecycle.hpp"
#include "nfs/graphics/resources/eagl_graphics.hpp"

namespace nfs::game::runtime::ai::lifecycle {

void deactivate_car_dynamics(Dword *entity) {
    auto *car = at<Dword *>(entity, 368);
    for (int wheel = 0; wheel < 4; ++wheel)
        pending::clear_car_contact(car, wheel);
    void *physics = at<void *>(car, 1768);
    at<Byte>(physics, 728) = 0;
    at<Byte>(physics, 729) = 0;
}

void dispatch_cop_event(Dword *entity, Dword event) {
    switch (event) {
    case 2:
        pending::reset_cop(state<Dword *>(7352448), entity);
        return;
    case 3:
        entity[143] = 0;
        invoke(entity, 6, Dword{0});
        return;
    case 6:
        entity[143] = 5;
        return;
    case 7:
        if (!at<Byte>(entity, 576)) {
            invoke(bytes(state<Dword *>(7352448)) + 92, 3, entity);
            invoke(entity, 14, Dword{5});
            return;
        }
        entity[93] = graphics::unresolved::camera_state;
        entity[94] = 0;
        invoke(entity, 34);
        entity[143] = 7;
        return;
    case 8:
        entity[143] = 6;
        return;
    case 9:
        entity[143] = 3;
        return;
    case 10:
        entity[143] = 1;
        invoke(entity, 6, Dword{1});
        return;
    case 11:
        entity[142] = graphics::unresolved::camera_state;
        entity[143] = 2;
        invoke(entity, 46, Dword{1});
        return;
    case 12:
        entity[143] = 7;
        return;
    }
}
} // namespace nfs::game::runtime::ai::lifecycle
namespace nfs::game::runtime::ai::pending {
void dispatch_vehicle_event(Dword *entity, Dword event) {
    lifecycle::dispatch_cop_event(entity, event);
}
} // namespace nfs::game::runtime::ai::pending
namespace nfs::game::runtime::physics::pending {
void retire_vehicle(void *entity) {
    ai::lifecycle::deactivate_car_dynamics(static_cast<Dword *>(entity));
}
} // namespace nfs::game::runtime::physics::pending
