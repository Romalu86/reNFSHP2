#include "../../ai/driving/ai_drive_x87.hpp"
#include "cop_driving_runtime.hpp"
namespace nfs::game::runtime::ai::cars {
using namespace physics::car_frame;

Extended car_route_heading(void *entity) {
    unsigned char retained[10];
    drive_frame::route_heading_native80(entity, retained);
    return Extended::from_native80(retained, kernel::numeric::current_format());
}

void __fastcall autonomous_cop_drive(void *entity, void *, Dword controls) {
    const Dword state = at<Dword>(entity, 572);
    if (state != 5)
        at<Dword>(entity, 1808) = 0;
    switch (state) {
    case 1:
        lifecycle::dispatch_cop_event(static_cast<Dword *>(entity), 11);
        break;
    case 4:
        ++at<Dword>(entity, 588);
        if (at<Dword>(entity, 588) >= lifecycle::state<Dword>(7352424))
            at<Dword>(entity, 572) = 5;
        break;
    case 5:
        update_cop_chase_controls(entity, controls);
        break;
    case 6:
        lifecycle::dispatch_cop_event(static_cast<Dword *>(entity), 10);
        break;
    case 7:
        detect_speeding_racers(entity);
        break;
    }
}

void update_cop_chase_controls(void *entity, Dword controls_word) {
    drive_frame::update_cop_controls(entity, controls_word);
}
} // namespace nfs::game::runtime::ai::cars
