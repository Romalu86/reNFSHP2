#include "player_drive_runtime.hpp"
#include "../../input/vehicles/car_binding_readers.hpp"
namespace nfs::game::runtime::ai::cars {

void __fastcall player_drive(void *entity, void *, Dword controls) {
    void *car = at<void *>(entity, 368);
    if (!car || !services::pending::car_is_remote(car))
        return;
    if (at<void *>(game::tick_services[0], 300) == car)
        read_cop_action_bindings(system(100), 0, reinterpret_cast<Dword *>(controls));
    const auto mode = at<std::int16_t>(car, 7940);
    if (mode == 3)
        drive_player_scripted_path(entity, controls);
    else if (mode == 4) {
        update_player_drive_state(entity);
        switch (at<Dword>(entity, 600)) {
        case 0:
            drive_player_before_start(entity, controls);
            return;
        case 1:
            drive_player_path(entity, controls);
            return;
        case 2:
            reverse_player_on_path(entity, controls);
            return;
        case 3:
            drive_player_start_path(entity, controls);
            return;
        case 4:
            recover_player_on_path(entity, controls);
            return;
        }
    } else if (mode == 6)
        drive_player_cinematic_path(entity, controls);
}

void __fastcall pursuit_drive(void *entity, void *, Dword controls) {
    if (!at<void *>(entity, 368))
        return;
    update_player_drive_state(entity);
    if (lifecycle::state<Dword>(7352732)) {
        invoke(entity, 32, controls);
        return;
    }
    switch (at<Dword>(entity, 600)) {
    case 0:
        drive_player_before_start(entity, controls);
        break;
    case 2:
        reverse_player_on_path(entity, controls);
        break;
    case 3:
        drive_player_start_path(entity, controls);
        break;
    case 4:
        recover_player_on_path(entity, controls);
        break;
    case 5:
        invoke(entity, 43, controls);
        break;
    default:
        drive_player_path(entity, controls);
        break;
    }
    if (at<Dword>(entity, 592) &&
        std::int32_t(graphics::unresolved::camera_state - at<Dword>(entity, 592)) < 60) {
        at<Byte>(reinterpret_cast<void *>(controls), 4) = at<Dword>(entity, 588) ? 127 : 129;
        at<Dword>(reinterpret_cast<void *>(controls), 0) |= 2;
    }
}
} // namespace nfs::game::runtime::ai::cars
