#include "vehicle_renderable_frame.hpp"
#include "nfs/graphics/display/graphics_error.hpp"
#include <cstring>
namespace nfs::game::world::vehicle_render {
static_assert(__LDBL_MANT_DIG__ == 64 && __LDBL_MAX_EXP__ == 16384,
              "Driver arithmetic requires native x87 intermediates");

void bind_driver_car(void *driver, void *car) {
    at<void *>(driver, 24) = car;
}

void update_driver_animation(void *driver, Dword action) {
    if (!at<Byte>(driver, 12))
        return;
    void *actor = at<void *>(driver, 52);
    if (!actor || !at<void *>(actor, 384))
        return;
    const Dword animation = at<Dword>(actor, 92);
    at<Dword>(driver, 44) = animation;
    at<Byte>(driver, 40) = 0;
    if (animation > 90902 && animation < 90908) {
        actor_resources::update_actor_animation(actor);
        at<Byte>(driver, 40) = at<Byte>(actor, 62) == 0;
    }
    if (at<Byte>(driver, 40))
        return;
    const Dword reverse = at<Byte>(at<void *>(driver, 24), 1584);
    if (reverse != at<Dword>(driver, 28)) {
        at<Dword>(driver, 28) = reverse;
        at<Dword>(actor, 376) = 1051372191;
        Dword target = 90907;
        if (!at<Byte>(driver, 48)) {
            auto *rules = static_cast<Dword *>(subsystem(72));
            if (network::packets::rule_value(rules, 6) != 2 &&
                network::packets::rule_value(rules, 6) != 3)
                target = 90903;
        }
        pending::select_character_animation(actor, target, 0);
        return;
    }
    if (action >= 1 && action <= 3) {
        pending::select_character_animation(actor,
                                            action == 1   ? 90905
                                            : action == 2 ? 90906
                                                          : 90904,
                                            0);
        return;
    }
    const auto steering =
        (static_cast<long double>(at<float>(at<void *>(driver, 24), 7992)) + 1.f) * .5f;
    const Dword now = graphics::unresolved::camera_state;
    if (int(at<Dword>(driver, 16) + 3) < int(now)) {
        at<Dword>(driver, 16) = now;
        if (!(static_cast<long double>(at<float>(driver, 32)) < steering)) {
            const auto next = static_cast<long double>(at<float>(driver, 32)) - .1f;
            at<float>(driver, 32) = float(next);
            if (next < steering)
                at<float>(driver, 32) = float(steering);
        } else {
            const auto next = static_cast<long double>(at<float>(driver, 32)) + .1f;
            at<float>(driver, 32) = float(next);
            if (steering < next)
                at<float>(driver, 32) = float(steering);
        }
    }
    const float frame = float(static_cast<long double>(at<float>(driver, 36)) * at<float>(driver, 32));
    Dword frame_bits;
    std::memcpy(&frame_bits, &frame, 4);
    pending::select_character_animation(actor, 90902, frame_bits);
    at<Dword>(actor, 376) = 1051372191;
}

void draw_vehicle_driver(void *driver, void *director, const float *rotation,
                         const float *position) {
    if (!at<Byte>(driver, 13) || !at<Byte>(driver, 12))
        return;
    void *actor = at<void *>(driver, 52);
    if (!actor)
        return;
    const float origin[4] = {0, 0, 0, 0};
    float matrix[16], mount[4], transformed[4];
    for (unsigned axis = 0; axis < 3; ++axis)
        std::memcpy(mount + axis, offset(driver, axis * 4), 4);
    mount[3] = 0;
    actor_resources::compose_actor_transform(matrix, rotation, origin);
    graphics::math::transform_row(mount, matrix, transformed);
    const float x = float(static_cast<long double>(transformed[0]) + position[0]),
                y = float(static_cast<long double>(transformed[1]) + position[1]),
                z = float(-(static_cast<long double>(transformed[2]) + position[2]));
    actor_resources::reset_animation_clock(offset(actor, 52));
    Dword position_bits[3];
    std::memcpy(position_bits, &x, 4);
    std::memcpy(position_bits + 1, &y, 4);
    std::memcpy(position_bits + 2, &z, 4);

    at<volatile Dword>(actor, 124) = position_bits[0];
    at<volatile Dword>(actor, 128) = position_bits[1];
    at<volatile Dword>(actor, 132) = position_bits[2];
    at<volatile Dword>(actor, 124) = position_bits[0];
    at<volatile Dword>(actor, 128) = position_bits[1];
    Dword *viewport = *static_cast<Dword *volatile *>(&graphics::main_viewport);
    at<volatile Dword>(actor, 132) = position_bits[2];
    at<volatile float>(actor, 132) =
        float(static_cast<long double>(at<volatile float>(actor, 132)) * -1.f);
    // The native REP MOVSD reads each word after the preceding output store.
    for (unsigned index = 0; index < 9; ++index) {
        Dword component;
        std::memcpy(&component, rotation + index, 4);
        at<volatile Dword>(actor, 336 + index * 4) = component;
    }
    world::invoke(actor, 2, director, viewport);
}

void advance_damage_visibility(void *damage) {
    const int now = int(graphics::unresolved::camera_state);
    if (!at<Byte>(damage, 20))
        return;
    at<Dword>(damage, 52) = Dword(-1);
    at<Dword>(damage, 56) = 0;
    for (unsigned i = 0; i < 10; ++i) {
        void *zone = offset(at<void *>(damage, 76), i * 72);
        if (int(at<Dword>(zone, 56) + 200) < now)
            at<Byte>(zone, 53) = 0;
    }
}

void evaluate_damage_pose(void *damage) {
    if (!at<Byte>(damage, 20))
        return;
    for (unsigned i = 0; i < 10; ++i) {
        void *zone = offset(at<void *>(damage, 76), i * 72);
        if (at<Byte>(zone, 52) && at<float>(zone, 60) > 0) {
            actor_resources::set_channel_offset(damage, int(at<Dword>(zone, 12)),
                                                at<Dword>(zone, 60));
            actor_resources::set_channel_offset(damage, int(at<Dword>(zone, 16)),
                                                at<Dword>(zone, 60));
            actor_resources::evaluate_actor_channel(damage, at<Dword>(zone, 12),
                                                    animation_pose_scratch());
            actor_resources::evaluate_actor_channel(damage, at<Dword>(zone, 16),
                                                    animation_pose_scratch());
        }
    }
}
} // namespace nfs::game::world::vehicle_render
