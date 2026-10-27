#include "nfs/physics/contacts/contact_dependencies.hpp"
#include "nfs/graphics/particles/particle_lifecycle.hpp"
#include "../vehicles/car_drive_frame_x87.hpp"
#include "../vehicles/car_physics_frame.hpp"
#include <initializer_list>
static_assert(__LDBL_MANT_DIG__ == 64 && __LDBL_MAX_EXP__ == 16384,
              "These reset calculations require native x87 intermediates");
namespace nfs::game::runtime::physics::contact {
// The full selected gear is returned; the control caller consumes its low byte.
Dword request_next_gear(void *car) {
    Dword gear = at<Dword>(car, 96);
    if (!at<Byte>(car, 776)) {
        if (!at<Byte>(car, 1586))
            gear = 1;
        else if (static_cast<int>(gear) < static_cast<int>(at<Dword>(car, 780) - 1))
            ++gear;
        at<Byte>(car, 1586) = static_cast<Byte>(gear);
    }
    return gear;
}

void reset_transmission(void *car) {
    const Byte automatic = at<Byte>(car, 776);
    at<Dword>(car, 8) = 1;
    at<Byte>(car, 1586) = automatic ? 3 : 1;
    for (unsigned offset :
         {1596u, 68u, 72u, 76u, 80u, 84u, 88u, 92u, 100u, 12u, 32u})
        at<Dword>(car, offset) = 0;
    const int idle = at<int>(car, 656);
    const auto retained_idle = static_cast<long double>(idle);
    at<Dword>(car, 96) = 1;
    at<Byte>(car, 1584) = 1;
    at<Byte>(car, 1582) = 2;
    const auto angular_speed = retained_idle * 0.10471975803375244140625f;
    at<int>(car, 20) = idle;
    at<float>(car, 4) = float(angular_speed);
    // FST retains the extended value. __ftol supplies the low 32 bits to FILD.
    const auto rounded_speed = angular_speed * 9.54929637908935546875f + 0.5f;
    const auto speed = car_frame::drive::truncate_engine_rpm(&rounded_speed);
    at<float>(car, 24) =
        float(static_cast<long double>(speed) / static_cast<long double>(at<int>(car, 28)));
}

void reset_wheel_state(void *wheel) {
    void *car = at<void *>(wheel, 0), *body = at<void *>(car, 1768);
    for (unsigned index = 0; index < 21; ++index)
        at<Dword>(wheel, 48 + index * 4) = at<Dword>(body, 508 + index * 4);
    const bool has_ground_polygon = at<void *>(body, 532) != nullptr;
    for (unsigned index : {4u, 6u, 5u, 8u, 11u})
        at<Dword>(wheel, index * 4) = 0;
    const Dword material = has_ground_polygon ? (at<Byte>(body, 586) & 31) : 0;
    const auto mass = static_cast<long double>(at<float>(car, 520));
    at<Dword>(wheel, 132) = material;
    for (unsigned index : {36u, 37u, 38u, 39u, 40u, 41u, 42u, 43u})
        at<Dword>(wheel, index * 4) = 0;
    const int wheel_index = at<int>(wheel, 12);
    for (unsigned index : {52u, 57u, 59u, 60u, 61u, 62u, 63u, 65u, 66u})
        at<Dword>(wheel, index * 4) = 0;
    const auto weight = mass * car_frame::gravity;
    const bool rear_wheel = wheel_index >= 2;
    const auto load = weight * -.25f;
    at<Dword>(wheel, 176) = 0;
    at<Dword>(wheel, 136) = 1065353216;
    at<float>(wheel, 232) = float(load);
    at<Dword>(wheel, 140) = 1061997773;
    at<Byte>(wheel, 256) = 1;
    const auto stiffness = static_cast<long double>(at<float>(car, 352 + (rear_wheel ? 4 : 0)));
    at<float>(wheel, 216) = float(stiffness);
}

void reset_controls(void *car) {
    const auto downward_gravity = -static_cast<long double>(car_frame::gravity);
    void *body = at<void *>(car, 1768);
    for (unsigned offset : {8u, 12u, 16u, 20u})
        at<Dword>(body, offset) = 0;
    for (unsigned offset : {3136u, 3144u, 3172u, 3176u, 3180u, 3148u, 3152u, 3156u,
                            3184u, 3188u, 3192u, 3132u, 3160u, 3164u, 3168u, 3196u,
                            3200u, 3204u})
        at<Dword>(car, offset) = 0;
    at<Dword>(body, 24) = 0;
    at<Dword>(body, 28) = 0;
    at<float>(car, 3140) = float(downward_gravity);
    for (unsigned offset : {496u, 492u, 488u, 484u, 480u, 476u})
        at<Dword>(car, offset) = 0;
    body = at<void *>(car, 1768);
    for (unsigned axis = 0; axis < 9; ++axis)
        at<Dword>(car, 376 + axis * 4) = at<Dword>(body, 692 + axis * 4);
    for (unsigned axis = 0; axis < 9; ++axis)
        at<Dword>(car, 412 + axis * 4) = at<Dword>(body, 72 + axis * 4);
    const Dword contact_flags = at<Dword>(car, 1792);
    for (unsigned offset : {460u, 456u, 364u, 368u, 372u, 448u, 7992u})
        at<Dword>(car, offset) = 0;
    // These adjacent state bytes remain two byte stores.
    at<volatile Byte>(car, 1692) = 0;
    at<volatile Byte>(car, 1693) = 0;
    at<Dword>(car, 452) = 1065353216;
    at<Dword>(car, 1792) = contact_flags & 4294965247u;
    for (unsigned wheel = 0; wheel < 4; ++wheel) {
        particles::lifecycle::release_wheel_skid(car, static_cast<int>(wheel));
        at<Dword>(car, 1700 + wheel * 4) = 0;
        reset_wheel_state(bytes(car) + 1948 + wheel * 296);
    }
    at<volatile Dword>(car, 1792) &= 4294965247u;
    reset_transmission(car);
    at<volatile Dword>(car, 8) |= 1;
}
} // namespace nfs::game::runtime::physics::contact
