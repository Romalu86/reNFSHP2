#include "player_drive_runtime.hpp"
#include "nfs/physics/contacts/vehicle_contacts.hpp"
#include "../../math/vectors/retained_vector.hpp"
namespace nfs::game::runtime::ai::cars {

void limit_player_turn_speed(void *entity, Dword controls, const float *target, float requested) {
    using kernel::numeric::Extended;
    const auto format = kernel::numeric::current_format();
    auto E = [&](float value) { return Extended(value, format); };
    auto turn = steer_car_to_position(entity, target, reinterpret_cast<void *>(controls));
    if (turn < E(0))
        turn = -turn;
    const float stored_turn = float(turn);
    const auto speed = Extended::from_double(double(invoke<long double>(entity, 22)), format);
    if (stored_turn > 0.02222222276031970977783203125f && invoke<long double>(entity, 22) > float_word(1104581388)) {
        auto ratio = E(stored_turn) / E(at<float>(at<void *>(entity, 368), 124));
        if (ratio < E(0))
            ratio = E(0);
        else if (E(1) < ratio)
            ratio = E(1);
        const float blend = float(ratio);
        const float difference = float(E(requested) - speed);
        const auto current = Extended::from_double(double(invoke<long double>(entity, 22)), format);
        requested = float((E(1) - E(blend)) * difference + current);
    }
    invoke(entity, 33, requested, controls);
}

void drive_player_scripted_path(void *entity, Dword controls) {
    using kernel::numeric::Extended;
    const auto format = kernel::numeric::current_format();
    auto E = [&](float value) { return Extended(value, format); };
    auto speed = [&] {
        return Extended::from_double(double(invoke<long double>(entity, 22)), format);
    };
    void *car = at<void *>(entity, 368);
    Dword node = 0;
    float lookahead = at<float>(car, 7948);
    float forward[3];
    std::memcpy(forward, bytes(car) + 3500, 12);
    if (lookahead < 0)
        lookahead = -lookahead;
    void *section = invoke<void *>(entity, 1);
    int sample = invoke<int>(entity, 2);
    float velocity_alignment = 1.f;
    float path_direction[3];
    if (section && sample >= 0) {
        track::path_direction(section, sample, path_direction);
        velocity_alignment =
            float(E(forward[0]) * path_direction[0] + E(forward[1]) * path_direction[1] -
                  E(forward[2]) * path_direction[2]);
        if (at<int>(entity, 240) == -1)
            velocity_alignment = -velocity_alignment;
    }
    std::memcpy(forward, bytes(car) + 3488, 12);
    section = invoke<void *>(entity, 1);
    sample = invoke<int>(entity, 2);
    float heading_alignment = 1.f;
    if (section && sample >= 0) {
        track::path_direction(section, sample, path_direction);
        heading_alignment =
            float(E(forward[0]) * path_direction[0] + E(forward[1]) * path_direction[1] -
                  E(forward[2]) * path_direction[2]);
        if (at<int>(entity, 240) == -1)
            heading_alignment = -heading_alignment;
    }
    const float supplied_speed = at<float>(car, 3484);
    bool reverse = velocity_alignment < 0;
    const bool motion_reverse = track::retained_vehicle_direction(entity) < E(0);
    bool changing_direction = false;
    if (reverse != motion_reverse && (heading_alignment < 0) == motion_reverse) {
        const float magnitude = supplied_speed < 0 ? -supplied_speed : supplied_speed;
        if (magnitude > float_word(1055187230)) {
            changing_direction = true;
            reverse = motion_reverse;
        }
    }
    auto *requested = reinterpret_cast<float *>(bytes(entity) + 640);
    float target[3], pose[21];
    const float *position = invoke<const float *>(entity, 0);
    if (!entities::pending::sample_path_target(bytes(entity) + 220, target, requested, &node, pose,
                                               lookahead, position, Byte(reverse))) {
        invoke(entity, 33, 0.f, controls);
        return;
    }
    auto apply_supplied_speed = [&] {
        if (!changing_direction) {
            if (!(*requested > supplied_speed))
                *requested = supplied_speed;
        } else {
            float minimum = float(E(supplied_speed) * 0.25f);
            if (*requested < minimum)
                minimum = *requested;
            *requested = minimum;
        }
    };
    apply_supplied_speed();
    invoke(entity, 54, target, target, *requested, requested, 2.f);
    auto current = speed();
    if (!(E(float_word(1091505331)) < current))
        current = E(float_word(1091505331));
    const float maximum_step = float(current + current);
    float cached[3], delta[3];
    std::memcpy(cached, invoke<const float *>(entity, 29), 12);
    for (unsigned axis = 0; axis < 3; ++axis)
        delta[axis] = float(E(target[axis]) - cached[axis]);
    const long double distance = physics::pending::normalize_collision_entry(delta);
    if (distance < maximum_step)
        for (unsigned axis = 0; axis < 3; ++axis)
            target[axis] =
                float(E(delta[axis]) * maximum_step * float_word(1023969417) + cached[axis]);
    auto steering = steer_car_to_position(entity, target, reinterpret_cast<void *>(controls));
    float absolute_steering = float(steering);
    if (steering < E(0))
        absolute_steering = -absolute_steering;
    current = speed();
    const auto difference = E(*requested) - current;
    const float stored_difference = float(difference);
    if (E(0) < difference && stored_difference < float_word(1091505331)) {
        bool assign = false;
        auto adjusted = E(0);
        if (absolute_steering > 0.0416666679084300994873046875f) {
            adjusted = (E(float_word(1023969417)) / E(absolute_steering)) * speed();
            assign = true;
        } else if (absolute_steering > 0.013888888992369174957275390625f) {
            auto fraction = E(absolute_steering) * 24.f;
            if (fraction < E(0))
                fraction = E(0);
            else if (E(1) < fraction)
                fraction = E(1);
            const float rounded_fraction = float(fraction);
            adjusted = (E(1) - E(rounded_fraction) * 0.2f) * stored_difference + speed();
            assign = true;
        }
        if (assign)
            *requested = float(adjusted);
    }
    apply_supplied_speed();
    invoke(entity, 33, *requested, controls);
}
} // namespace nfs::game::runtime::ai::cars
