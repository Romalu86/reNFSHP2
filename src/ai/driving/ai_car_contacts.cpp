#include "ai_car_runtime.hpp"
#include "nfs/graphics/resources/eagl_graphics.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
#include <cctype>
#include <cmath>
namespace nfs::game::runtime::ai::cars {

void __fastcall car_frame(void *entity, void *) {
    if (!at<Byte>(entity, 212))
        return;
    invoke(entity, 31);
    if (!at<void *>(entity, 368))
        return;
    void *body_view = entities::interface_at(entity, 3);
    void *body = invoke<void *>(body_view, 0);
    if (body && at<Byte>(body, 728))
        return;
    void *mutable_body = invoke<void *>(body_view, 1);
    const Dword kind = invoke<Dword>(entities::interface_at(entity, 1), 0);
    if (kind == 6) {
        void *car = at<void *>(entity, 368);
        if (!car)
            return;
        if (at<std::int16_t>(car, 7940) == 5) {
            at<std::int16_t>(car, 7940) = 1;
            at<Byte>(mutable_body, 728) = 0;
            return;
        }
    }
    invoke(entity, 36);
}

void __fastcall traffic_frame(void *entity, void *) {
    if (!at<Dword>(entity, 568))
        return;
    void *body = invoke<void *>(entities::interface_at(entity, 3), 0);
    const Dword time = graphics::unresolved::camera_state;
    if ((!body || !at<Byte>(body, 728)) && (at<Dword>(entity, 568) || time == 0)) {
        body = at<void *>(at<void *>(entity, 368), 1768);
        at<Byte>(body, 728) = 0;
        at<Byte>(body, 729) = 0;
        at<Dword>(entity, 564) = time;
        at<Dword>(entity, 568) = 0;
        at<Dword>(entity, 584) = 0;
        invoke(entity, 6, Dword{0});
    }
    car_frame(entity, nullptr);
}

void __fastcall car_apply_stun(void *entity, void *, Dword strength_bits) {
    const float strength = float_word(strength_bits);
    if (!(strength <= 0.f)) {
        at<Dword>(entity, 420) = graphics::unresolved::camera_state;
        const Dword minimum = invoke<Dword>(entity, 51);
        const Dword maximum = invoke<Dword>(entity, 52);
        using kernel::numeric::Extended;
        const auto format = kernel::numeric::current_format();
        const auto ticks = (Extended::integer(std::int32_t(maximum - minimum), format) * strength)
                               .truncate_to_integer();
        const Dword duration = Dword(ticks) + minimum;
        if (std::int32_t(at<Dword>(entity, 424)) < std::int32_t(duration))
            at<Dword>(entity, 424) = duration;
    }
}

void select_contact_voice(void *entity, const char *name) {
    const Dword remote = invoke<Dword>(entities::interface_at(entity, 2), 4);
    if (!remote && name && *name &&
        std::int32_t(at<Dword>(entity, 556) + at<Dword>(entity, 560)) <
            std::int32_t(graphics::unresolved::camera_state)) {
        char *destination = reinterpret_cast<char *>(bytes(entity) + 540);
        std::strncpy(destination, name, 15);
        at<Dword>(entity, 556) = graphics::unresolved::camera_state;
        at<Dword>(entity, 560) = 0;
        const int length = int(std::strlen(destination));
        for (int index = 0; index < length; ++index)
            at<Dword>(entity, 560) += Dword((std::toupper(destination[index]) - 64) * 5);
    }
}

void __fastcall car_side_contact(void *entity, void *, Dword, Dword speed_bits, Dword strength_bits) {
    const float speed = float_word(speed_bits), strength = float_word(strength_bits);
    if (speed != 0.f && strength != 0.f && speed > 13.4109f)
        select_contact_voice(entity, reinterpret_cast<const char *>(bytes(entity) + 444));
}
// The saved interface result is AL; upper EAX is incidental.
Byte __fastcall car_accept_side_contact(void *entity, void *, Dword, Dword speed_bits) {
    if (float_word(speed_bits) < 15.f)
        select_contact_voice(entity, reinterpret_cast<const char *>(bytes(entity) + 428));
    return 0;
}

void __fastcall car_body_contact(void *entity, void *, Dword contact_address) {
    void *contact = reinterpret_cast<void *>(contact_address);
    void *body = at<void *>(contact, 0);
    if (!body)
        return;
    if (!physics::vehicle_car(body)) {
        if (at<Byte>(body, 5) == 1 && !at<Dword>(body, 676))
            at<Dword>(body, 676) = at<Dword>(entity, 368);
    } else {
        using kernel::numeric::Extended;
        const auto format = kernel::numeric::current_format();
        const auto retained_strength = Extended(at<float>(contact, 4), format) /
                                       Extended(at<float>(contact, 56), format);
        const float strength = float(retained_strength);
        if (Extended(26.8218f, format) < retained_strength)
            select_contact_voice(entity, reinterpret_cast<const char *>(bytes(entity) + 524));
        else if (strength > 17.8812f)
            select_contact_voice(entity, reinterpret_cast<const char *>(bytes(entity) + 508));
        else if (strength > 8.9406f)
            select_contact_voice(entity, reinterpret_cast<const char *>(bytes(entity) + 492));
        else if (strength > 4.4703f)
            select_contact_voice(entity, reinterpret_cast<const char *>(bytes(entity) + 476));
    }
}

void __fastcall traffic_side_contact(void *entity, void *, Dword other, Dword speed, Dword strength) {
    void *other_entity = at<void *>(reinterpret_cast<void *>(other), 7988);
    if (invoke<Dword>(entities::interface_at(other_entity, 1), 0) != 4)
        car_side_contact(entity, nullptr, other, speed, strength);
}

Dword __fastcall traffic_accept_side_contact(void *entity, void *, Dword other, Dword speed) {
    void *other_entity = at<void *>(reinterpret_cast<void *>(other), 7988);
    if (invoke<Dword>(entities::interface_at(other_entity, 1), 0) != 4)
        car_accept_side_contact(entity, nullptr, other, speed);
    return 1;
}
// Upper EAX is incidental on false paths.
Dword __fastcall player_visible(void *entity, void *, Dword visitor) {
    void *body = invoke<void *>(entities::interface_at(entity, 3), 0);
    return body && at<Byte>(body, 728) ? car_visible(entity, nullptr, visitor) : 0;
}

Dword __fastcall ai_cop_visible(void *entity, void *, Dword visitor) {
    if (at<Byte>(entity, 640))
        return 1;
    void *body = invoke<void *>(entities::interface_at(entity, 3), 0);
    return body && at<Byte>(body, 728) ? car_visible(entity, nullptr, visitor) : 0;
}

void __fastcall autonomous_cop_collision(void *entity, void *, Dword deferred) {
    car_collision(entity, nullptr, deferred);
}
} // namespace nfs::game::runtime::ai::cars
