#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "ai_car_runtime.hpp"
#include "nfs/graphics/resources/eagl_graphics.hpp"
namespace nfs::game::runtime::ai::cars {
Dword maximum_car_stun_ticks{};  
 
void initialize_car_stun_ticks() {
    maximum_car_stun_ticks = 180;
}

Dword __fastcall car_owner(void *self, void *) {
    return word_pointer(bytes(self) - 572);
}

Dword __fastcall car_const_owner(void *self, void *) {
    return word_pointer(bytes(self) - 572);
}

Dword __fastcall cop_base_owner(void *self, void *) {
    return word_pointer(bytes(self) - 576);
}

Dword __fastcall perpetrator_owner(void *self, void *) {
    return word_pointer(bytes(self) - 32);
}

Dword __fastcall pursuit_owner(void *self, void *) {
    return word_pointer(bytes(self) - 608);
}

Dword __fastcall car_mutable_body(void *self, void *) {
    void *car = at<void *>(self, -212);
    return car ? at<Dword>(car, 1768) : 0;
}

Dword __fastcall car_const_body(void *self, void *) {
    void *car = at<void *>(self, -212);
    return car ? at<Dword>(car, 1768) : 0;
}

Dword __fastcall autonomous_cop_type(void *self, void *) {
    return 3;
}

Dword __fastcall player_type(void *self, void *) {
    return 6;
}

Dword __fastcall pursuit_type(void *self, void *) {
    return 0;
}

Dword __fastcall ai_cop_type(void *self, void *) {
    return 5;
}

Dword __fastcall network_car_type(void *self, void *) {
    return 2;
}

Dword __fastcall traffic_type(void *self, void *) {
    return 4;
}

Dword __fastcall ai_racer_type(void *self, void *) {
    return 1;
}

Dword __fastcall car_path_group(void *self, void *) {
    return 0;
}

Dword __fastcall car_minimum_stun(void *self, void *) {
    return 1;
}

Dword __fastcall player_uses_path(void *self, void *) {
    return 1;
}

Byte __fastcall player_enabled(void *self, void *) {
    return 0;
}

Byte __fastcall autonomous_cop_accept_side_contact(void *self, void *, Dword other, Dword speed) {
    return 0;
}

Byte __fastcall traffic_visible(void *self, void *, Dword visitor) {
    return 0;
}

Byte __fastcall car_accelerator(void *self, void *) {
    return at<Byte>(at<void *>(self, 368), 1576);
}

Byte __fastcall car_brake(void *self, void *) {
    return at<Byte>(at<void *>(self, 368), 1581);
}

long double __fastcall car_steering(void *self, void *) {
    return at<float>(at<void *>(self, 368), 1600);
}

long double __fastcall car_speed(void *self, void *) {
    return at<float>(at<void *>(self, 368), 3168);
}

Dword __fastcall car_mutable_velocity(void *self, void *) {
    return word_pointer(bytes(at<void *>(self, 368)) + 3160);
}

Dword __fastcall car_velocity(void *self, void *) {
    return word_pointer(bytes(at<void *>(self, 368)) + 3160);
}

Dword __fastcall car_mutable_body_rotation(void *self, void *) {
    return word_pointer(bytes(at<void *>(self, 368)) + 412);
}

Dword __fastcall car_body_rotation(void *self, void *) {
    return word_pointer(bytes(at<void *>(self, 368)) + 412);
}

void __fastcall car_begin_path_recovery(void *self, void *) {
    at<Dword>(self, 404) = 4;
}

void __fastcall car_begin_body_recovery(void *self, void *) {
    at<Dword>(self, 404) = 5;
}

Dword __fastcall car_path_recovery(void *self, void *) {
    return at<Dword>(self, 404) == 4;
}

Dword __fastcall car_body_recovery(void *self, void *) {
    return at<Dword>(self, 404) == 5;
}

void __fastcall car_set_speed_components(void *self, void *, Dword lateral, Dword vertical,
                                         Dword forward) {
    void *car = at<void *>(self, 368);
    at<Dword>(car, 3160) = lateral;
    at<Dword>(car, 3164) = vertical;
    at<Dword>(car, 3168) = forward;
}

void __fastcall car_move_to(void *self, void *, Dword speed, Dword position) {
    entities::move_towards_position(self, nullptr, float_word(speed),
                                    reinterpret_cast<const float *>(position));
    at<Dword>(at<void *>(self, 368), 3168) = speed;
}

void __fastcall car_hold_controls(void *self, void *, Dword duration) {
    at<Dword>(self, 372) = graphics::unresolved::camera_state;
    at<Dword>(self, 376) = duration;
    at<Dword>(self, 404) = 5;
}

Dword __fastcall car_remaining_stun(void *self, void *) {
    if (at<Dword>(self, 420) != Dword(-1)) {
        const Dword remaining = at<Dword>(self, 420) - graphics::unresolved::camera_state +
                               at<Dword>(self, 424);
        if (std::int32_t(remaining) > 0)
            return remaining;
        at<Dword>(self, 424) = Dword(-1);
        at<Dword>(self, 420) = Dword(-1);
    }
    return 0;
}

Dword __fastcall car_maximum_stun(void *self, void *) {
    return maximum_car_stun_ticks;
}

Dword __fastcall autonomous_cop_minimum_stun(void *self, void *) {
    return lifecycle::pending::minimum_cop_stun_ticks;
}

Dword __fastcall autonomous_cop_maximum_stun(void *self, void *) {
    return lifecycle::state<Dword>(7352432);
}

Dword __fastcall autonomous_cop_uses_path(void *self, void *) {
    return at<Dword>(self, 1808);
}

Dword __fastcall autonomous_cop_route_option(void *self, void *) {
    return lifecycle::pending::cop_meter_threshold;
}

Dword __fastcall traffic_uses_path(void *self, void *) {
    return at<Dword>(self, 580);
}

void __fastcall car_path_notice(void *self, void *, Dword path) {
    // Complete RET4 leaf.
}

void __fastcall autonomous_cop_side_contact(void *self, void *, Dword other, Dword speed,
                                            Dword strength) {
    // Complete RET12 leaf.
}

void __fastcall racer_route_option(void *self, void *, Dword event, Dword parameter) {
    // Complete RET8 leaf.
}

void __fastcall car_set_recovery(void *self, void *, Dword enabled) {
    if (at<void *>(self, 368)) {
        if (Byte(enabled))
            invoke(self, 34);
        else
            at<Dword>(self, 404) = 0;
    }
}

void __fastcall perpetrator_reset(void *self, void *) {
    at<Dword>(self, 8) = 0;
    at<Dword>(self, 12) = 0;
    at<Dword>(self, 16) = 0;
    at<Dword>(self, 20) = 0;
    at<Byte>(self, 24) = 0;
    at<Byte>(self, 25) = 0;
}

void __fastcall perpetrator_bind(void *self, void *, Dword chase) {
    at<Dword>(self, 8) = chase;
    ++at<Dword>(self, 12);
    at<Dword>(self, 20) = 0;
    at<Byte>(self, 24) = 0;
    at<Byte>(self, 25) = 0;
}

void __fastcall perpetrator_clear(void *self, void *, Dword chase) {
    at<Dword>(self, 8) = 0;
    at<Dword>(self, 20) = 0;
    at<Byte>(self, 24) = 0;
    at<Byte>(self, 25) = 0;
}
} // namespace nfs::game::runtime::ai::cars
NFS_KERNEL_INITIALIZER(startup_initialize_car_stun_ticks, ".CRT$XCU_NFS_00690174",
                       nfs::game::runtime::ai::cars::initialize_car_stun_ticks);
