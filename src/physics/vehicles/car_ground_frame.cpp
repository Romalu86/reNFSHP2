#include "car_physics_frame.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
#include <intrin.h>
namespace nfs::game::runtime::physics::car_frame {
namespace {
long double N(float value) { return static_cast<long double>(value); }
long double A(long double value) { return value < 0.0L ? -value : value; }
}

void sample_wheel_ground(void *car) {
    float wheel_positions[12];
    for (unsigned index = 0; index < 4; ++index)
        __movsd(reinterpret_cast<unsigned long *>(wheel_positions + index * 3),
                reinterpret_cast<const unsigned long *>(vector_at(wheel_at(car, index), 276)), 3);
    void *body = at<void *>(car, 1768);
    float rotation[9], position[3], predicted[12];
    __movsd(reinterpret_cast<unsigned long *>(rotation),
            reinterpret_cast<const unsigned long *>(vector_at(body, 692)), 9);
    __movsd(reinterpret_cast<unsigned long *>(position),
            reinterpret_cast<const unsigned long *>(vector_at(body, 680)), 3);
    predict_body_pose(0.0333333350718021392822265625f, vector_at(body, 8), vector_at(body, 20), position, rotation,
                      vector_at(body, 44));
    matrices::transform_positions(4, wheel_positions, rotation, position, predicted, 12);
    at<Dword>(car, 448) = 0;
    for (unsigned index = 0; index < 4; ++index) {
        void *wheel = wheel_at(car, index);
        const float previous_height = predicted[index * 3 + 1];
        surface_height(bytes(wheel) + 48, predicted + index * 3);
        const auto displacement = N(predicted[index * 3 + 1]) - previous_height;
        const float stored_displacement = float(displacement);
        const auto limit = -(N(at<float>(wheel, 212)) - at<float>(wheel, 260));
        if (!(displacement < limit)) {
            at<Byte>(wheel, 256) = 1;
            ++at<Dword>(car, 448);
        } else
            at<Byte>(wheel, 256) = 0;
        at<Dword>(wheel, 132) = !at<Dword>(wheel, 72) ? 0 : at<Byte>(wheel, 126) & 31;
        const float upper = float(N(at<float>(wheel, 212)) * .3f);
        const float lower = -at<float>(wheel, 212);
        const float stored = stored_displacement < lower ? lower
                             : upper < stored_displacement ? upper : stored_displacement;
        at<float>(wheel, 252) = stored;
    }
}

void project_wheel_ground_speeds(void *car) {
    const float front_length = at<float>(car, 112), rear_length = at<float>(car, 116);
    const float length = rear_length < front_length ? front_length : rear_length;
    const float metric = float(A(N(length) * at<float>(car, 3200) *
                                         6.283185482025146484375f) +
                                A(N(at<float>(car, 3132))));
    const auto yaw = N(at<float>(car, 3200)) * 6.283185482025146484375f;
    const float front_lateral = float(yaw * front_length);
    const float rear_lateral = float(yaw * rear_length);
    const float front_yaw = float(N(at<float>(car, 532)) * (yaw * .5f));
    const float rear_yaw = float(N(at<float>(car, 536)) * (yaw * .5f));
    for (unsigned index = 0; index < 4; ++index) {
        void *wheel = wheel_at(car, index);
        const bool grounded = at<Byte>(wheel, 256) != 0;
        auto projected = N(0);
        if (!grounded)
            projected = N(at<float>(wheel, 216)) * at<float>(wheel, 156);
        else if (!(.25f < metric)) {
            if (N(.25f) < A(N(at<float>(car, 3168))))
                projected = N(at<float>(car, 3168));
        } else {
            float forward = at<float>(car, 3168);
            float lateral;
            if (index < 2) {
                lateral = float(N(at<float>(car, 3160)) + front_lateral);
                forward =
                    float(index == 1 ? N(forward) - front_yaw : N(forward) + front_yaw);
            } else {
                lateral = float(N(at<float>(car, 3160)) - rear_lateral);
                forward =
                    float(index == 2 ? N(forward) - rear_yaw : N(forward) + rear_yaw);
            }
            const float side = float(turn_sine(at<float>(wheel, 16)) * lateral);
            projected = turn_cosine(at<float>(wheel, 16)) * forward + side;
        }
        at<float>(wheel, 44) = float(projected);
        at<Dword>(wheel, 168) = 0;
    }
}
} // namespace nfs::game::runtime::physics::car_frame
