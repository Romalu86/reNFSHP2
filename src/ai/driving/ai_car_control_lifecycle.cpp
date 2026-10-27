#include "ai_car_behavior.hpp"
#include "nfs/graphics/resources/eagl_graphics.hpp"
#include "nfs/network/session/network_runtime.hpp"
namespace nfs::game::runtime::ai::cars {

Dword __fastcall pursuit_enabled(void *entity, void *) {
    return lifecycle::state<Byte>(7352952) && !lifecycle::state<Byte>(7352731) &&
           !lifecycle::state<Byte>(7352730) && !at<void *>(entities::interface_at(entity, 4), 8);
}

Dword __fastcall ai_racer_enabled(void *entity, void *) {
    return !at<void *>(entities::interface_at(entity, 4), 8);
}

Dword __fastcall ai_cop_pursuit_view(void *entity, void *) {
    return entity ? word_pointer(entities::interface_at(entity, 4)) : 0;
}

void __fastcall ai_cop_begin_path_recovery(void *entity, void *) {
    const Dword deadline = graphics::unresolved::camera_state + 300;
    at<Dword>(entity, 404) = 4;
    at<Dword>(entity, 648) = deadline;
}

void __fastcall ai_cop_begin_body_recovery(void *entity, void *) {
    const Dword time = graphics::unresolved::camera_state;
    at<Dword>(entity, 404) = 5;
    at<Dword>(entity, 644) = time + 12;
    at<Byte>(entity, 640) = 1;
    at<Dword>(entity, 652) = time;
}

void __fastcall car_recover_body(void *entity, void *, Dword controls) {
    invoke(entity, 32, controls);
    const Dword hold_tick = at<Dword>(entity, 372);
    if (std::int32_t(hold_tick) > 0 &&
        std::int32_t(at<Dword>(entity, 376)) <
            std::int32_t(graphics::unresolved::camera_state - hold_tick)) {
        at<Dword>(entity, 376) = Dword(-1);
        at<Dword>(entity, 372) = Dword(-1);
        if (at<Byte>(entity, 408) && !network::session_active()) {
            invoke(entity, 6, Dword{0});
            return;
        }
        at<Dword>(entity, 404) = 0;
    }
}

void __fastcall network_car_recover_motion(void *entity, void *, Dword controls) {
    apply_car_path_controls(entity, controls);
    car_recover_motion(entity, nullptr, controls);
}

void __fastcall network_car_recover_body(void *entity, void *, Dword controls) {
    apply_car_path_controls(entity, controls);
    car_recover_body(entity, nullptr, controls);
}

void __fastcall ai_racer_recover_motion(void *entity, void *, Dword controls) {
    apply_car_path_controls(entity, controls);
    at<Dword>(reinterpret_cast<void *>(controls), 0) &= 4294967263u;
    car_recover_motion(entity, nullptr, controls);
}

void __fastcall ai_racer_recover_body(void *entity, void *, Dword controls) {
    apply_car_path_controls(entity, controls);
    at<Dword>(reinterpret_cast<void *>(controls), 0) &= 4294967263u;
    car_recover_body(entity, nullptr, controls);
}

void __fastcall ai_cop_recover_motion(void *entity, void *, Dword controls) {
    car_recover_motion(entity, nullptr, controls);
    const Dword time = graphics::unresolved::camera_state;
    if (std::int32_t(time) >= at<int>(entity, 644)) {
        at<Dword>(entity, 644) = time + 12;
        at<Byte>(entity, 640) = !at<Byte>(entity, 640);
    }
}

void __fastcall ai_cop_recover_body(void *entity, void *, Dword controls) {
    player_recover_body(entity, nullptr, controls);
    const Dword time = graphics::unresolved::camera_state;
    if (std::int32_t(time) >= at<int>(entity, 644)) {
        at<Dword>(entity, 644) = time + 12;
        at<Byte>(entity, 640) = !at<Byte>(entity, 640);
    }
    if (std::int32_t(time) >= at<int>(entity, 648)) {
        at<Byte>(entity, 640) = 0;
        invoke(entity, 6, Dword{0});
    }
}

void __fastcall ai_cop_drive(void *entity, void *, Dword controls) {
    apply_spike_strip_hits(entity);
    pursuit_drive(entity, nullptr, controls);
}

void __fastcall player_side_contact(void *entity, void *, Dword other, Dword speed, Dword strength) {
    if (at<Dword>(entity, 600) != 3) {
        if (void *other_entity = at<void *>(reinterpret_cast<void *>(other), 7988))
            if (invoke<Dword>(other_entity, 10))
                car_side_contact(entity, nullptr, other, speed, strength);
    }
}
} // namespace nfs::game::runtime::ai::cars
