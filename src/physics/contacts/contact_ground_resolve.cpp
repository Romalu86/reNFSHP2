#include "contact_history.hpp"
#include "nfs/physics/frame/dynamics_runtime.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
#include <cmath>
namespace nfs::game::runtime::physics {
extern const float pair_friction[2];
// Iteration count, impulse history and first-contact record.
int pending::resolve_ground_point(void *body, const float *point,
                                           const float *surface_normal, const float *friction,
                                           Dword material) {
    const auto E = [](float value) { return static_cast<long double>(value); };
    float normal[3], arm[3]{}, arm_direction[3]{}, angular[3], linear[3], linear_change[3]{},
        angular_change[3]{};
    copy_triplet(normal, surface_normal);
    hardware::normalize_collision(normal);
    const float mass = at<float>(body, 32);
    contact_arm(body, point, arm);
    copy_triplet(arm_direction, arm);
    hardware::normalize_collision(arm_direction);
    for (unsigned i = 0; i < 3; ++i) {
        angular[i] =
            float(static_cast<long double>(at<float>(body, 20 + i * 4)) * 6.283185482025146484375f);
        linear[i] = at<float>(body, 8 + i * 4);
    }
    ContactHistory linear_history, angular_history;
    int iteration = 0;
    for (; iteration < contact_iterations; ++iteration) {
        const float fraction = float(
            (E(1) / static_cast<long double>(contact_iterations)) *
            static_cast<long double>(iteration));
        float rotational[3], velocity[3], momentum[3];
        cross_vectors(angular, arm, rotational);
        for (unsigned i = 0; i < 3; ++i) {
            velocity[i] = float(static_cast<long double>(rotational[i]) + linear[i]);
            momentum[i] = float(static_cast<long double>(velocity[i]) * mass);
        }
        const auto projection = hardware::dot_product(normal, momentum);
        const float normal_momentum = float(projection);
        if (projection >= 0.0L)
            break;
        const float approach = -normal_momentum;
        float normal_impulse[3], tangent_impulse[3]{};
        scale_triplet(normal_impulse, normal, approach);
        int mode = contact_friction(momentum, normal_momentum, normal, friction, tangent_impulse);
        if (!iteration) {
            at<float>(body, 608) = approach;
            at<Dword>(body, 656) = material;
            for (unsigned i = 0; i < 3; ++i) {
                at<float>(body, 640 + i * 4) = point[i];
                at<float>(body, 612 + i * 4) = normal_impulse[i];
                at<float>(body, 664 + i * 4) = normal[i];
                at<float>(body, 624 + i * 4) = tangent_impulse[i];
            }
            at<float>(body, 660) = mass;
            at<Dword>(body, 604) = 0;
            at<Dword>(body, 652) = 1;
            at<int>(body, 636) = mode;
        }
        scale_triplet(normal_impulse, normal_impulse, fraction);
        scale_triplet(tangent_impulse, tangent_impulse, fraction);
        float normal_linear[3]{}, normal_angular[3]{}, tangent_linear[3]{}, tangent_angular[3]{},
            sum[3], delta[3];
        contact_force(body, normal_impulse, arm, arm_direction, normal_linear,
                               normal_angular);
        contact_force(body, tangent_impulse, arm, arm_direction, tangent_linear,
                               tangent_angular);
        sum_triplets(sum, tangent_angular, normal_angular);
        angular_history.append(sum);
        sum_triplets(sum, tangent_linear, normal_linear);
        linear_history.append(sum);
        linear_history.average(delta);
        accumulate_triplet(linear, delta);
        angular_history.average(delta);
        accumulate_triplet(angular, delta);
        linear_history.average(delta);
        accumulate_triplet(linear_change, delta);
        weighted_history(&angular_history.values[0][0], delta);
        accumulate_triplet(angular_change, delta);
    }
    if (!iteration)
        return 0;
    for (unsigned i = 0; i < 3; ++i)
        at<float>(body, 8 + i * 4) =
            float(static_cast<long double>(linear_change[i]) + at<float>(body, 8 + i * 4));
    float turns[3];
    scale_triplet(turns, angular_change, 0.15915493667125701904296875f);
    for (unsigned i = 0; i < 3; ++i)
        at<float>(body, 20 + i * 4) =
            float(static_cast<long double>(turns[i]) + at<float>(body, 20 + i * 4));
    return 1;
}
} // namespace nfs::game::runtime::physics
