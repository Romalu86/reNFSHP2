#include "ai_car_behavior.hpp"
#include "ai_path_frame_x87.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
#include "../../math/precision/frame_x87_bridge.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
#include "nfs/physics/frame/dynamics_runtime.hpp"
#include "nfs/world/resources/runtime_resources.hpp"
#include <cmath>
#include <cstring>
namespace nfs::game::runtime::ai::cars::path_frame {
namespace {
int truncate_control(long double value) {
    return legacy::truncate_control(&value);
}
}
static long double calculate_steering(void *entity, const float *position, void *controls) {
    void *car = at<void *>(entity, 368);
    invoke(entity, 30, position);
    void *body_view = entities::interface_at(entity, 3);
    void *body = invoke<void *>(body_view, 0);
    float *cached_position = reinterpret_cast<float *>(bytes(body_view) + 4);
    if (body) {
        at<Dword>(cached_position, 0) = at<Dword>(body, 680);
        at<Dword>(cached_position, 4) = at<Dword>(body, 684);
        cached_position[2] = -at<float>(body, 688);
    } else {
        std::memcpy(cached_position, runtime::saved::zero_vector, 12);
    }
    auto E = [](float value) { return static_cast<long double>(value); };
    float delta[3];
    delta[0] = float(E(position[0]) - cached_position[0]);
    delta[1] = float(E(position[1]) - cached_position[1]);
    const auto z = E(position[2]) - cached_position[2];
    if (delta[0] == 0.f && z == 0.0L) {
        at<float>(entity, 396) = 0.f;
        at<Byte>(controls, 4) = 0;
        return E(0);
    }
    delta[2] = float(-z);
    invoke(entities::interface_at(entity, 3), 1);
    float relative[3];
    matrices::transform_vectors(
        1, delta, reinterpret_cast<const float *>(bytes(at<void *>(car, 1768)) + 72), relative);
    const auto heading = runtime::turn_direction_retained(relative[0], relative[2]);
    float angle = float(heading);
    if (heading < E(-0.5f))
        angle = -0.5f;
    else if (angle > 0.5f)
        angle = 0.5f;
    if (invoke<long double>(entity, 22) < 0)
        angle = -angle;
    body = invoke<void *>(entities::interface_at(entity, 3), 0);
    if (body && at<Byte>(body, 728) && at<std::uint16_t>(at<void *>(car, 1768), 596) > 15)
        at<float>(entity, 396) = at<float>(car, 476);
    else
        at<float>(entity, 396) = angle;
    const float limit = at<float>(car, 124);
    float steering = at<float>(entity, 396);
    if (steering < -limit)
        steering = -limit;
    else if (steering > limit)
        steering = limit;
    at<float>(entity, 396) = steering;
    auto command = truncate_control((E(steering) / E(limit)) * 127.f);
    if (command < -127)
        command = -127;
    else if (command > 127)
        command = 127;
    at<Byte>(controls, 4) = Byte(command);
    return E(angle);
}

void set_target_speed(void *entity, Dword requested_word, Dword controls_word) {
    auto E = [](float value) { return static_cast<long double>(value); };
    auto speed_value = [&] {
        return invoke<long double>(entity, 22);
    };
    void *car = at<void *>(entity, 368);
    const float speed = float(speed_value());
    void *body = invoke<void *>(entities::interface_at(entity, 3), 0);
    void *controls = reinterpret_cast<void *>(controls_word);
    if (body && at<Byte>(body, 728) && at<std::uint16_t>(at<void *>(car, 1768), 596) > 15) {
        at<Byte>(controls, 5) = 0;
        at<Byte>(controls, 6) = 0;
        return;
    }
    const float requested = float_word(requested_word);
    at<Dword>(entity, 400) = requested_word;
    const float threshold = 4.470300197601318359375f;
    bool proportional = false;
    if (requested > 0.f && E(0) < speed_value()) {
        auto difference = E(requested) - speed_value();
        if (difference < E(-threshold))
            difference = difference * 2.2369899749755859375f * 0.02f;
        else if (difference < E(0)) {
            at<float>(entity, 392) = 0.f;
            proportional = true;
        } else
            difference = difference * 2.2369899749755859375f * 0.05f;
        if (!proportional) {
            if (difference < E(-1))
                difference = E(-1);
            else if (E(1) < difference)
                difference = E(1);
            at<float>(entity, 392) = float(difference);
            proportional = true;
        }
    }
    if (!proportional) {
        if (speed > threshold && requested < -threshold && at<float>(entity, 392) >= 0.f)
            at<float>(entity, 392) = -1.f;
        else if (speed < -threshold && requested > threshold && at<float>(entity, 392) <= 0.f)
            at<float>(entity, 392) = 1.f;
        if (speed < requested) {
            const auto throttle = E(0.0599999986588954925537109375f) + at<float>(entity, 392);
            at<float>(entity, 392) = float(throttle);
            if (E(1) < throttle)
                at<float>(entity, 392) = 1.f;
        } else if (speed > requested) {
            if (E(speed) < E(4.470300197601318359375f) + requested && speed > 26.82180023193359375f)
                at<float>(entity, 392) = 0.f;
            else {
                auto throttle = E(at<float>(entity, 392));
                if (!(throttle < E(0)))
                    throttle = E(0);
                at<float>(entity, 392) = float(throttle - 0.0599999986588954925537109375f);
            }
            if (at<float>(entity, 392) < -1.f)
                at<float>(entity, 392) = -1.f;
        } else {
            if (at<float>(entity, 392) < 0.f)
                at<float>(entity, 392) = 0.f;
            else if (at<float>(entity, 392) > 0.f) {
                const auto throttle = E(at<float>(entity, 392)) - 0.0599999986588954925537109375f;
                at<float>(entity, 392) = float(throttle);
                if (throttle < E(0))
                    at<float>(entity, 392) = 0.f;
            }
        }
        auto throttle = E(at<float>(entity, 392));
        if (throttle < E(-1))
            throttle = E(-1);
        else if (E(1) < throttle)
            throttle = E(1);
        at<float>(entity, 392) = float(throttle);
        auto error = E(speed) - requested;
        if (error < E(0))
            error = -error;
        if (error < E(threshold)) {
            const float ratio = float((E(1) / E(threshold)) * error);
            auto limited = throttle;
            if (throttle < E(-ratio))
                limited = E(-ratio);
            else if (E(ratio) < throttle)
                limited = E(ratio);
            at<float>(entity, 392) = float(limited * 0.8f + throttle * 0.2f);
        }
    }
    const float throttle = at<float>(entity, 392);
    const bool braking = !(0.f <= throttle);
    auto amount =
        truncate_control(E(throttle) * (braking ? -255.f : 255.f));
    if (amount < 0)
        amount = 0;
    else if (amount > 255)
        amount = 255;
    at<Byte>(controls, braking ? 5 : 6) = 0;
    at<Byte>(controls, braking ? 6 : 5) = Byte(amount);
}

void recover_motion(void *entity, Dword controls_word) {
    void *controls = reinterpret_cast<void *>(controls_word);
    int sample;
    Dword point;
    void *path = bytes(entity) + 220;
    entities::pending::path_frame::advance_distance(path, 2.f, &sample, &point);
    float position[3];
    std::memcpy(position, reinterpret_cast<const void *>(point), 12);
    float pose[21];
    lifecycle::sample_path(path, sample, pose);
    if (at<int>(entity, 240) == -1)
        dependencies::reverse_section_pose(pose);
    const float width = path_side_width(path, reinterpret_cast<const void *>(point));
    float offset[3];
    runtime::scale_vectors(1, pose + 9, width, offset);
    for (int axis = 0; axis < 3; ++axis)
        position[axis] = float(static_cast<long double>(offset[axis]) + position[axis]);
    calculate_steering(entity, position, controls);
    invoke(entity, 32, controls_word);
    auto speed = invoke<long double>(entity, 22);
    if (speed < static_cast<long double>(0.f))
        speed = -speed;
    if (speed <= static_cast<long double>(float_word(1046810970)))
        invoke(entity, 34);
}
void steering_native80(void *entity, const float *position, void *controls, void *output) {
    const long double result = calculate_steering(entity, position, controls);
    std::memcpy(output, &result, 10);
}
}
namespace nfs::game::runtime::ai::entities::pending::path_frame {
namespace { long double absolute_speed(long double value) { return value < 0 ? -value : value; } }
void move_entity(void *entity, float speed, const float *target) {
    void *body = invoke<void *>(interface_at(entity, 3), 1);
    float position[3], direction[3];
    for (unsigned axis = 0; axis < 3; ++axis)
        position[axis] = at<float>(body, 680 + axis * 4);
    for (unsigned axis = 0; axis < 3; ++axis)
        direction[axis] =
            float(static_cast<long double>(axis == 2 ? -target[axis] : target[axis]) - position[axis]);
    physics::pending::normalize_collision_entry(direction);
    for (float &component : direction)
        component = float(static_cast<long double>(component) * speed);
    for (unsigned axis = 0; axis < 3; ++axis) {
        at<Dword>(body, 8 + axis * 4) = 0;
        at<Dword>(body, 20 + axis * 4) = 0;
    }
    const auto x_step = static_cast<long double>(direction[0]) * 0.0333333350718021392822265625f;
    const auto y_step = static_cast<long double>(direction[1]) * 0.0333333350718021392822265625f;
    const float z_step = float(static_cast<long double>(direction[2]) * 0.0333333350718021392822265625f);
    const float next[3] = {float(x_step + position[0]), float(y_step + position[1]),
                          float(static_cast<long double>(z_step) + position[2])};
    for (unsigned axis = 0; axis < 3; ++axis)
        at<float>(body, 680 + axis * 4) = next[axis];
}

void brake_entity(void *entity, Dword *flags) {
    const long double speed = absolute_speed(invoke<long double>(entity, 22));
    if (speed > 2.23515f) {
        invoke(entity, 33, 0.f, flags);
        *flags |= 2;
        return;
    }
    const Dword *methods = at<const Dword *>(entity, 0);
    using Speed = long double(__thiscall *)(void *);
    using SetSpeed = void(__thiscall *)(void *, float, Dword *);
    const float current_speed = float(reinterpret_cast<Speed>(methods[22])(entity));
    reinterpret_cast<SetSpeed>(methods[33])(entity, current_speed, flags);
    void *body = invoke<void *>(interface_at(entity, 3), 1);
    if (absolute_speed(at<float>(body, 8)) < 0.223515f && absolute_speed(at<float>(body, 12)) < 0.223515f &&
        absolute_speed(at<float>(body, 16)) < 0.223515f) {
        invoke(entity, 23, saved::zero_vector);
        at<Dword>(body, 8) = at<Dword>(body, 12) = at<Dword>(body, 16) = 0;
    }
    *flags |= 2;
}
}
