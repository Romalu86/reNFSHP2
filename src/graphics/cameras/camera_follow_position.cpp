#include "camera_director_runtime.hpp"
#include <initializer_list>
namespace nfs::game::runtime::ai::entities::pending {
kernel::numeric::Extended direction_turns(float, float);
}
namespace nfs::game::world::camera {
using kernel::numeric::Extended;
namespace {
Extended X(float value) {
    return Extended(value, kernel::numeric::current_format());
}
Extended history_sum(void *history) {
    const auto middle = X(at<float>(history, 8)) + at<float>(history, 4);
    return middle + middle + at<float>(history, 12) + at<float>(history, 0);
}
void shift_history(void *history) {
    for (unsigned field = 0; field < 12; field += 4)
        at<Dword>(history, field) = at<Dword>(history, field + 4);
}
void advance_angle_filter(void *behavior, unsigned angle_field, unsigned out_tension,
                          unsigned back_tension, unsigned history_field, float threshold,
                          float scale, bool speed_scaled) {
    float coefficient = -at<float>(behavior, out_tension);
    const float absolute = std::fabs(at<float>(behavior, angle_field));
    if (threshold < absolute)
        coefficient = float(X(absolute) * coefficient * scale);
    if (speed_scaled) {
        const float speed = at<float>(at<void *>(behavior, 4), 3168);
        if (26.8218f < speed)
            coefficient = float(X(coefficient) * speed * 0.0372831039130687713623046875f);
    }
    const float root = float(X(std::fabs(coefficient)).square_root());
    auto damping = -(X(root) * at<float>(behavior, back_tension));
    void *history = offset(behavior, history_field);
    if (X(0.f) < history_sum(history) * at<float>(behavior, angle_field) * 0.16666667163372039794921875f)
        damping = damping + damping;
    const auto old_sum = history_sum(history);
    shift_history(history);
    at<float>(history, 12) = float(
        (X(coefficient) * at<float>(behavior, angle_field) + old_sum * damping * 0.16666667163372039794921875f) *
            0.01666666753590106964111328125f +
        at<float>(history, 12));
    at<float>(behavior, angle_field) =
        float(history_sum(history) * 0.00277777784503996372222900390625f + at<float>(behavior, angle_field));
}
void advance_acceleration_spring(void *behavior) {
    void *car = at<void *>(behavior, 4);
    void *history = offset(behavior, 396);
    auto force = X(0.f);
    if (!(2.23515f < at<float>(car, 3168))) {
        for (unsigned field = 0; field < 16; field += 4)
            at<Dword>(history, field) = 0;
        at<float>(behavior, 260) = float(X(at<float>(behavior, 260)) * 0.95f);
    } else {
        const float acceleration = at<float>(car, 3156);
        if ((at<float>(car, 1608) != 0 || at<Byte>(car, 1585)) && acceleration < 0)
            force = X(-acceleration);
        force = force - X(acceleration) * 0.5f;
    }
    const bool negative = !(0.f < at<float>(behavior, 260));
    const float stiffness = at<float>(behavior, negative ? 124 : 132);
    const float net_force = float(force - X(stiffness) * at<float>(behavior, 260));
    const float root = float(X(stiffness).square_root());
    const auto damping =
        negative ? history_sum(history) * at<float>(behavior, 128) * root * 0.16666667163372039794921875f
                 : history_sum(history) * root * 0.16666667163372039794921875f * at<float>(behavior, 136);
    shift_history(history);
    at<float>(history, 12) =
        float((X(net_force) - damping) * 0.01666666753590106964111328125f + at<float>(history, 12));
    auto increment = history_sum(history) * 0.00277777784503996372222900390625f;
    const auto change = history_sum(history) * at<float>(behavior, 260) * 0.16666667163372039794921875f;
    if (change < X(0.f)) {
        const auto bound = X(std::fabs(at<float>(behavior, 260)));
        increment = X(float(increment));
        if (increment < -bound)
            increment = -bound;
        else if (bound < increment)
            increment = bound;
    }
    float displacement = float(increment + at<float>(behavior, 260));
    if (displacement < -3.f)
        displacement = -3.f;
    else if (displacement > 1.f)
        displacement = 1.f;
    at<float>(behavior, 260) = displacement;
}
} // namespace

void fill_four_values(void *destination, const float *value) {
    const Dword bits = *reinterpret_cast<const Dword *>(value);
    for (unsigned field = 0; field < 16; field += 4)
        at<Dword>(destination, field) = bits;
}
void reset_follow_histories(void *behavior) {
    for (unsigned group : {428u, 412u, 396u})
        for (unsigned field = 0; field < 16; field += 4)
            at<Dword>(behavior, group + field) = 0;
}
// Only AL is consumed.
Byte hold_unstable_follow_camera(void *behavior, void *state) {
    void *car = at<void *>(behavior, 4), *body = at<void *>(car, 1768);
    const bool stable =
        !(2.f < std::fabs(at<float>(body, 24))) && !(at<float>(body, 112) < 0.707f) &&
        !(1.f < std::fabs(at<float>(body, 20))) && !(1.f < std::fabs(at<float>(body, 28)));
    if (at<Dword>(body, 732)) {
        at<Dword>(behavior, 444) = 0;
        return 0;
    }
    if (!stable && !at<Dword>(behavior, 444))
        at<Dword>(behavior, 444) = 15;
    if (at<Dword>(behavior, 444)) {
        if (stable && !at<std::int16_t>(body, 596))
            --at<Dword>(behavior, 444);
        float *position = reinterpret_cast<float *>(offset(behavior, 448));
        for (unsigned axis = 0; axis < 3; ++axis)
            position[axis] =
                float(X(at<float>(body, 680 + axis * 4)) + at<float>(behavior, 376 + axis * 4));
        const bool bird = at<Dword>(state, 72) == 2;
        clip_camera_position(offset(behavior, 8), position, body, bird ? 3.75f : 1.5f, bird);
        raise_camera_above_ground(offset(behavior, 8), position, 1.f);
        avoid_camera_vehicles(position, car, 1.f);
        refresh_camera_ground(position, at<void *>(at<void *>(behavior, 4), 1768),
                              offset(behavior, 8));
        reset_follow_histories(behavior);
        std::memcpy(offset(state, 4), position, 12);
        at<Byte>(behavior, 492) = 0;
    }
    return at<Dword>(behavior, 444) != 0;
}

void __fastcall advance_follow_position(void *behavior, void *, void *state) {
    if (hold_unstable_follow_camera(behavior, state)) {
        std::memcpy(offset(behavior, 460),
                    offset(at<void *>(at<void *>(behavior, 4), 1768), 680), 12);
        return;
    }
    float local[3] = {0, at<float>(behavior, 100), 0};
    const auto squared =
        X(at<float>(behavior, 104)) * at<float>(behavior, 104) - X(local[1]) * local[1];
    local[2] = float(-squared.square_root());
    runtime::physics::pending::normalize_collision_entry(local);
    runtime::scale_vectors(1, local, at<float>(behavior, 104), local);
    const bool bird = at<Dword>(state, 72) == 2;
    const float radius = bird ? 3.75f : 1.5f;
    void *director = at<void *>(state, 0);
    bool changed_look = false;
    const bool looking = director && (at<Byte>(director, 164) || at<Byte>(director, 165) ||
                                      at<Byte>(director, 166));
    if (looking) {
        at<Byte>(behavior, 96) = 1;
        if (at<Byte>(director, 164))
            local[2] = -local[2];
        else {
            local[0] = at<Byte>(director, 165) ? -local[2] : local[2];
            local[2] = 0;
        }
    } else if (!at<Byte>(behavior, 96)) {
        if (at<Byte>(behavior, 97) && !bird) {
            advance_acceleration_spring(behavior);
            local[2] = float(X(at<float>(behavior, 260)) + local[2]);
        }
    } else {
        at<Byte>(behavior, 96) = 0;
        changed_look = true;
    }
    void *car = at<void *>(behavior, 4), *body = at<void *>(car, 1768);
    float position[3];
    if (!at<Byte>(behavior, 96) && !changed_look && !at<Dword>(body, 732)) {
        float body_position[3], retained[3], delta[3], relative[3];
        std::memcpy(body_position, offset(body, 680), 12);
        for (unsigned axis = 0; axis < 3; ++axis) {
            const auto movement = X(body_position[axis]) - at<float>(behavior, 460 + axis * 4);
            // The saved Z delta is spilled before the retained position is added.
            retained[axis] = float((axis == 2 ? X(float(movement)) : movement) +
                                   at<float>(behavior, 448 + axis * 4));
        }
        runtime::ai::entities::pending::direction_turns(at<float>(behavior, 100),
                                                        at<float>(behavior, 104));
        for (unsigned axis = 0; axis < 3; ++axis) {
            const auto relative = X(retained[axis]) - body_position[axis];
            delta[axis] = float(axis == 1 ? relative - at<float>(behavior, 100) : relative);
        }
        runtime::matrices::transform_vectors(
            1, delta, reinterpret_cast<float *>(offset(body, 72)), relative);
        at<float>(behavior, 388) =
            float(runtime::ai::entities::pending::direction_turns(relative[0], -relative[2]));
        const float horizontal_squared =
            float(X(relative[2]) * relative[2] + X(relative[0]) * relative[0]);
        const float horizontal = std::sqrt(horizontal_squared);
        at<float>(behavior, 392) =
            float(runtime::ai::entities::pending::direction_turns(relative[1], horizontal));
        if (at<float>(behavior, 108) != 0)
            advance_angle_filter(behavior, 388, 108, 112, 412, 0.0625f, 16.f, false);
        if (at<float>(behavior, 116) != 0)
            advance_angle_filter(behavior, 392, 116, 120, 428, 0.0416666679084300994873046875f, 24.f, true);
        car = at<void *>(behavior, 4);
        if (at<float>(car, 3168) < -4.4703f && !at<Byte>(car, 1584)) {
            at<float>(behavior, 392) = float(X(at<float>(behavior, 392)) * 0.95f);
            at<float>(behavior, 388) = float(X(at<float>(behavior, 388)) * 0.95f);
        }
        const float distance = std::fabs(local[2]);
        const float horizontal_radius =
            float(Extended::from_double(double(runtime::turn_cosine(at<float>(behavior, 392))),
                                        kernel::numeric::current_format()) *
                  distance);
        relative[0] =
            float(Extended::from_double(double(runtime::turn_sine(at<float>(behavior, 388))),
                                        kernel::numeric::current_format()) *
                  horizontal_radius);
        relative[2] =
            float(-(Extended::from_double(double(runtime::turn_cosine(at<float>(behavior, 388))),
                                          kernel::numeric::current_format()) *
                    horizontal_radius));
        relative[1] =
            float(Extended::from_double(double(runtime::turn_sine(at<float>(behavior, 392))),
                                        kernel::numeric::current_format()) *
                  distance);
        runtime::matrices::transform_vectors(
            1, relative, reinterpret_cast<float *>(offset(at<void *>(car, 1768), 692)), delta);
        delta[1] = float(X(delta[1]) + at<float>(behavior, 100));
        for (unsigned axis = 0; axis < 3; ++axis)
            position[axis] = float(X(body_position[axis]) + delta[axis]);
        std::memcpy(offset(behavior, 448), position, 12);
        avoid_camera_vehicles(position, car, at<Byte>(behavior, 492) ? 1.5f : 1.f);
        at<Byte>(behavior, 492) = clip_camera_position(offset(behavior, 8), position,
                                                         at<void *>(car, 1768), radius, bird);
        raise_camera_above_ground(offset(behavior, 8), position, 1.f);
        avoid_camera_vehicles(position, at<void *>(behavior, 4),
                              at<Byte>(behavior, 492) ? 1.5f : 1.f);
        if (!refresh_camera_ground(position, nullptr, offset(behavior, 8)))
            std::memcpy(position, offset(behavior, 448), 12);
        if (at<int>(behavior, 472) > 0 && !bird) {
            const float *axes = reinterpret_cast<float *>(offset(behavior, 480));
            if (at<Byte>(behavior, 272))
                add_road_shake(position, at<float>(behavior, 476), axes);
            else
                add_impact_shake(position, at<float>(behavior, 264), axes);
            --at<Dword>(behavior, 472);
        }
    } else {
        reset_follow_histories(behavior);
        const float zero = 0;
        fill_four_values(offset(behavior, 396), &zero);
        car = at<void *>(behavior, 4);
        body = at<void *>(car, 1768);
        at<Byte>(behavior, 492) = 0;
        float transformed[3];
        runtime::matrices::transform_vectors(
            1, local, reinterpret_cast<float *>(offset(body, 692)), transformed);
        for (unsigned axis = 0; axis < 3; ++axis)
            position[axis] = float(X(transformed[axis]) + at<float>(body, 680 + axis * 4));
        at<Byte>(behavior, 492) =
            clip_camera_position(offset(behavior, 8), position, body, radius, bird);
        raise_camera_above_ground(offset(behavior, 8), position, 1.f);
        car = at<void *>(behavior, 4);
        avoid_camera_vehicles(position, car, at<Byte>(behavior, 492) ? 1.5f : 1.f);
        refresh_camera_ground(position, at<void *>(car, 1768), offset(behavior, 8));
        std::memcpy(offset(behavior, 448), position, 12);
    }
    body = at<void *>(at<void *>(behavior, 4), 1768);
    std::memcpy(offset(behavior, 460), offset(body, 680), 12);
    body = at<void *>(at<void *>(behavior, 4), 1768);
    for (unsigned axis = 0; axis < 3; ++axis)
        at<float>(behavior, 376 + axis * 4) =
            float(X(position[axis]) - at<float>(body, 680 + axis * 4));
    std::memcpy(offset(state, 4), position, 12);
}

void __fastcall begin_follow_position(void *behavior, void *, void *state) {
    void *body = at<void *>(at<void *>(behavior, 4), 1768);
    float *relative = reinterpret_cast<float *>(offset(behavior, 376));
    for (unsigned axis = 0; axis < 3; ++axis)
        relative[axis] =
            float(X(at<float>(state, 4 + axis * 4)) - at<float>(body, 680 + axis * 4));
    const auto squared = X(at<float>(behavior, 104)) * at<float>(behavior, 104) -
                         X(at<float>(behavior, 100)) * at<float>(behavior, 100);
    const float radius = float(squared.square_root());
    runtime::physics::pending::normalize_collision_entry(relative);
    for (unsigned axis = 0; axis < 3; ++axis)
        relative[axis] = float(X(radius) * relative[axis]);
    const float local[3] = {0, at<float>(behavior, 100), -at<float>(behavior, 104)};
    body = at<void *>(at<void *>(behavior, 4), 1768);
    std::memcpy(offset(behavior, 8), offset(body, 508), 84);
    std::memcpy(offset(behavior, 460), offset(body, 680), 12);
    float transformed[3];
    runtime::matrices::transform_vectors(1, local, reinterpret_cast<float *>(offset(body, 692)),
                                         transformed);
    body = at<void *>(at<void *>(behavior, 4), 1768);
    for (unsigned axis = 0; axis < 3; ++axis)
        at<float>(behavior, 448 + axis * 4) =
            float(X(transformed[axis]) + at<float>(body, 680 + axis * 4));
    for (unsigned field = 472; field <= 488; field += 4)
        at<Dword>(behavior, field) = 0;
    reset_follow_histories(behavior);
    at<Byte>(behavior, 492) = 0;
    invoke(behavior, 1, state);
}
} // namespace nfs::game::world::camera
