#include "nfs/physics/contacts/vehicle_contacts.hpp"
#include "nfs/core/frame/game_step.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
#include <cmath>
#include <cstring>
#include <initializer_list>
namespace nfs::game::runtime::physics {
namespace {
float magnitude(float value) {
    return value < 0 ? -value : value;
}
} // namespace ECX is not read by the body.
 
Dword interpolate_vehicle(float seconds, void *car) {
    if (car == nullptr || reinterpret_cast<Dword>(car) == Dword(-3400))
        return 0;
    const auto mode = at<std::int16_t>(car, 7940);
    if (mode == 2) {
        approach_network_pose(car);
        return 1;
    }
    if (mode != 5)
        return 0;
    auto X = [](float value) { return static_cast<long double>(value); };
    auto absolute = [&](long double value) { return value < X(0.f) ? -value : value; };
    void *body = at<void *>(car, 1768);
    const float target_y = at<float>(car, 3468);
    const float angular_z = float(X(at<float>(car, 3472)) - at<float>(body, 28));
    at<float>(body, 20) =
        float((X(at<float>(car, 3464)) - at<float>(body, 20)) * seconds + at<float>(body, 20));
    at<float>(body, 24) = float((X(target_y) - at<float>(body, 24)) * seconds + at<float>(body, 24));
    at<float>(body, 28) = float(X(angular_z) * seconds + at<float>(body, 28));
    float rotation[9], source_rotation[4], target_rotation[4], interpolated[4];
    std::memcpy(rotation, bytes(body) + 692, sizeof(rotation));
    matrix_to_quaternion(rotation, source_rotation);
    matrix_to_quaternion(reinterpret_cast<float *>(bytes(car) + 3416), target_rotation);
    interpolate_rotation(source_rotation, target_rotation, float(X(seconds) * 5.0f), interpolated);
    quaternion_to_matrix(interpolated, rotation);
    matrices::orthonormalize(rotation);
    std::memcpy(bytes(body) + 692, rotation, sizeof(rotation));
    matrices::transpose(reinterpret_cast<float *>(bytes(body) + 692),
                        reinterpret_cast<float *>(bytes(body) + 72));
    const float position_x = at<float>(body, 680), position_y = at<float>(body, 684),
                position_z = at<float>(body, 688);
    auto elapsed = static_cast<long double>(std::int32_t(game::race_ticks - at<Dword>(car, 3400))) * seconds;
    if (elapsed < X(0.f))
        elapsed = X(0.f);
    const float predicted_x = float(elapsed * at<float>(car, 3452) + at<float>(car, 3404));
    const float predicted_y = float(elapsed * at<float>(car, 3456) + at<float>(car, 3408));
    const float error_x = float(X(predicted_x) - position_x);
    const float error_y = float(X(predicted_y) - position_y);
    const auto retained_error_z = elapsed * at<float>(car, 3460) + at<float>(car, 3412) - position_z;
    const float error_z = float(retained_error_z);
    const float mean_x = float((X(at<float>(body, 8)) + at<float>(car, 3452)) * .5f);
    const float mean_z = float((X(at<float>(car, 3460)) + at<float>(body, 16)) * .5f);
    if (magnitude(error_y) < .125f)
        at<Dword>(body, 12) = at<Dword>(car, 3456);
    const float horizontal_response = float(X(12.0f) * seconds);
    const auto vertical_response = X(60.0f) * seconds;
    const float velocity_error_x = float(X(at<float>(car, 3452)) - at<float>(body, 8));
    const float velocity_error_z = float(X(at<float>(car, 3460)) - at<float>(body, 16));
    if (!(absolute(X(error_x) - X(seconds) * at<float>(body, 8)) < X(1.25f)) ||
        (error_x < 0) != (at<float>(body, 8) < 0))
        at<float>(car, 3136) = float(X(horizontal_response) * error_x);
    else if (!(11.17574977874755859375f < magnitude(velocity_error_x)))
        at<float>(car, 3136) = velocity_error_x;
    else {
        at<float>(body, 8) = mean_x;
        at<Dword>(car, 3136) = 0;
    }
    if (!(absolute(X(error_z) - X(seconds) * at<float>(body, 16)) < X(1.25f)) ||
        (error_z < 0) != (at<float>(body, 16) < 0))
        at<float>(car, 3144) = float(X(horizontal_response) * error_z);
    else if (!(11.17574977874755859375f < magnitude(velocity_error_z)))
        at<float>(car, 3144) = velocity_error_z;
    else {
        at<float>(body, 16) = mean_z;
        at<Dword>(car, 3144) = 0;
    }
    const float vertical_correction = float(vertical_response * error_y);
    at<float>(car, 3140) = vertical_correction;
    auto limit = X(50.0f) * seconds;
    for (unsigned offset : {3136, 3144})
        if (absolute(limit) < X(magnitude(at<float>(car, offset))))
            at<float>(car, offset) = float(0.f < at<float>(car, offset) ? limit : -limit);
    limit = X(24.0f) * seconds;
    if (absolute(limit) < X(magnitude(vertical_correction)))
        at<float>(car, 3140) = float(0.f < vertical_correction ? limit : -limit);
    const auto vertical_velocity = -(X(9.81000041961669921875f) * seconds) +
                                   X(seconds) * at<float>(car, 3140) + at<float>(body, 12);
    at<float>(body, 12) = float(vertical_velocity);
    const bool vertical_nonnegative = !(vertical_velocity < X(0.f)) &&
                                      !std::isnan(float(vertical_velocity));
    if (vertical_nonnegative != (error_y >= 0))
        at<Dword>(body, 12) = 0;
    const float candidate_x = float(X(at<float>(car, 3136)) + at<float>(body, 8));
    const auto candidate_z = X(at<float>(car, 3144)) + at<float>(body, 16);
    const float stored_candidate_z = float(candidate_z);
    const float squared_speed = float(candidate_z * stored_candidate_z + X(candidate_x) * candidate_x);
    const float speed = float(__builtin_sqrtl(X(squared_speed)));
    if (!(speed < 250.0f)) {
        const auto next_x = X(at<float>(body, 8)) + at<float>(car, 3136);
        if (absolute(next_x) < X(magnitude(at<float>(body, 8))))
            at<float>(body, 8) = float(next_x);
        const auto next_z = X(at<float>(car, 3144)) + at<float>(body, 16);
        if (absolute(next_z) < X(magnitude(at<float>(body, 16))))
            at<float>(body, 16) = float(next_z);
    } else {
        at<float>(body, 8) = candidate_x;
        at<float>(body, 16) = stored_candidate_z;
    }
    for (unsigned offset : {8, 12, 16})
        if (245.0f < magnitude(at<float>(body, offset)))
            at<float>(body, offset) = at<float>(body, offset) <= 0 ? -245.0f : 245.0f;
    at<float>(body, 680) = float(X(seconds) * at<float>(body, 8) + position_x);
    at<float>(body, 684) = float(X(seconds) * at<float>(body, 12) + position_y);
    at<float>(body, 688) = float(X(seconds) * at<float>(body, 16) + position_z);
    return 1;
}
} // namespace nfs::game::runtime::physics
