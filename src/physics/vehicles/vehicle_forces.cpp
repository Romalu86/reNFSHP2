#include "nfs/physics/solver/solver_runtime.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
#include <intrin.h>
namespace nfs::game::runtime::physics::pending {

float *world_shape(void *state) {
    matrices::transform_positions(26, reinterpret_cast<float *>(bytes(state) + 140),
                                  reinterpret_cast<float *>(bytes(state) + 692),
                                  reinterpret_cast<float *>(bytes(state) + 680),
                                  shape_vertices, 12);
    return shape_vertices;
}

void contact_force(void *state, const float *force, const float *arm, const float *normal,
                            float *linear, float *angular) {
    const auto X = [](float value) { return static_cast<long double>(value); };
    const auto normal_force = -hardware::dot_product(force, normal);
    const float amount = float(normal_force);
    if (!(X(0) < normal_force)) {
        __movsd(reinterpret_cast<unsigned long *>(linear),
                reinterpret_cast<const unsigned long *>(saved::zero_vector), 3);
    } else {
        float direction[3];
        normalize_vector(force, direction);
        for (int i = 0; i < 3; ++i)
            linear[i] =
                float(X(direction[i]) * at<float>(state, 36) * amount);
    }
    float torque[3], body_torque[3], body_arm[3], temporary[3];
    cross_vectors_entry(arm, force, torque);
    __movsd(reinterpret_cast<unsigned long *>(body_arm),
            reinterpret_cast<const unsigned long *>(arm), 3);
    matrices::transform_vectors(1, torque, reinterpret_cast<float *>(bytes(state) + 72),
                                body_torque);
    matrices::transform_vectors(1, body_arm, reinterpret_cast<float *>(bytes(state) + 72),
                                temporary);
    __movsd(reinterpret_cast<unsigned long *>(body_arm),
            reinterpret_cast<const unsigned long *>(temporary), 3);
    const auto xx = X(body_arm[0]) * body_arm[0];
    const auto yy = X(body_arm[1]) * body_arm[1];
    const auto zz = X(body_arm[2]) * body_arm[2];
    const long double sums[3] = {yy + zz, xx + zz, xx + yy};
    for (int i = 0; i < 3; ++i) {
        const auto denominator = sums[i] * at<float>(state, 32) +
                                 at<float>(state, 452 + i * 4);
        angular[i] = float(X(body_torque[i]) / denominator);
    }
    matrices::transform_vectors(1, angular, reinterpret_cast<float *>(bytes(state) + 692),
                                temporary);
    __movsd(reinterpret_cast<unsigned long *>(angular),
            reinterpret_cast<const unsigned long *>(temporary), 3);
}

void integrate_forces(void *state) {
    if (!at<std::int16_t>(state, 596) || !at<Byte>(state, 5))
        return;
    const auto X = [](float value) { return static_cast<long double>(value); };
    float velocity[3], direction[3];
    __movsd(reinterpret_cast<unsigned long *>(velocity),
            reinterpret_cast<const unsigned long *>(bytes(state) + 8), 3);
    __movsd(reinterpret_cast<unsigned long *>(direction),
            reinterpret_cast<const unsigned long *>(bytes(state) + 8), 3);
    const float speed = float(hardware::normalize_collision(direction));
    const float *rotation = reinterpret_cast<float *>(bytes(state) + 692);
    const auto drag = [&](int axis) {
        const auto value = hardware::dot_product(direction, rotation + axis * 3);
        return value < X(0) ? -value : value;
    };
    const float first = float(X(at<float>(state, 124)) * at<float>(state, 128) *
                              at<float>(state, 468) * drag(0) * 4.0f);
    const float second = float(X(at<float>(state, 128)) * at<float>(state, 120) *
                                   at<float>(state, 472) * drag(1) * 4.0f +
                               first);
    const auto density = (X(at<float>(state, 124)) * at<float>(state, 120) *
                           at<float>(state, 476) * drag(2) * 4.0f +
                       second) *
                      at<float>(state, 464) * 0.612500011920928955078125f;
    float force[3];
    for (int i = 0; i < 3; ++i)
        force[i] = float(-(X(velocity[i]) * speed * density));
    float *shape = world_shape(state), arms[6][3];
    for (int i = 0; i < 6; ++i)
        for (int axis = 0; axis < 3; ++axis)
            arms[i][axis] = float(X(shape[24 + i * 3 + axis]) -
                                  at<float>(state, 680 + axis * 4));
    float total_linear[3]{}, total_angular[3]{};
    for (auto &arm : arms)
        if (X(0) < hardware::dot_product(velocity, arm)) {
            float normal[3], linear[3], angular[3];
            __movsd(reinterpret_cast<unsigned long *>(normal),
                    reinterpret_cast<const unsigned long *>(arm), 3);
            hardware::normalize_collision(normal);
            contact_force(state, force, arm, normal, linear, angular);
            for (int i = 0; i < 3; ++i)
                total_angular[i] = float(X(angular[i]) + total_angular[i]);
            for (int i = 0; i < 3; ++i)
                total_linear[i] = float(X(linear[i]) + total_linear[i]);
        }
    scale_vectors(1, total_linear, 0.0333333350718021392822265625f, total_linear);
    scale_vectors(1, total_angular, 0.005305164493620395660400390625f, total_angular);
    for (int i = 0; i < 3; ++i)
        at<float>(state, 8 + i * 4) = float(X(total_linear[i]) + at<float>(state, 8 + i * 4));
    for (int i = 0; i < 3; ++i)
        at<float>(state, 20 + i * 4) =
            float(X(total_angular[i]) + at<float>(state, 20 + i * 4));
}
} // namespace nfs::game::runtime::physics::pending
