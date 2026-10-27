#include "contact_history.hpp"
#include "nfs/physics/frame/dynamics_runtime.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
#include <cmath>
#include <cstring>
namespace nfs::game::runtime::physics::pending {
// Two-body impulse integration; frozen-body and sleep rules are the exact flag/energy tests used
// by the code.
int resolve_pair(void *first, void *second, const float *point,
                          const float *contact_normal) {
    const auto E = [](float value) { return static_cast<long double>(value); };
    float arms[2][3]{}, directions[2][3]{}, angular[2][3]{}, linear[2][3], normal[3];
    copy_triplet(normal, contact_normal);
    Byte frozen[2] = {at<Byte>(first, 730), at<Byte>(second, 730)};
    float friction[2];
    std::memcpy(friction, pair_friction, sizeof(friction));
    if (frozen[0] && frozen[1])
        frozen[0] = frozen[1] = 0;
    void *bodies[2] = {first, second};
    for (unsigned body = 0; body < 2; ++body) {
        contact_arm(bodies[body], point, arms[body]);
        copy_triplet(directions[body], arms[body]);
        hardware::normalize_collision(directions[body]);
    }
    for (unsigned body = 0; body < 2; ++body) {
        scale_vectors(1, reinterpret_cast<float *>(bytes(bodies[body]) + 20), 6.283185482025146484375f,
                      angular[body]);
        copy_triplet(linear[body], reinterpret_cast<float *>(bytes(bodies[body]) + 8));
    }
    if (at<float>(second, 600) < at<float>(first, 60))
        frozen[0] = 1;
    if (at<float>(first, 600) < at<float>(second, 60))
        frozen[1] = 1;
    float mass;
    if (!frozen[0] && frozen[1])
        mass = at<float>(first, 32);
    else if (frozen[0] && !frozen[1])
        mass = at<float>(second, 32);
    else {
        frozen[0] = frozen[1] = 0;
        mass = float(E(at<float>(second, 32)) * at<float>(first, 32) /
                     (E(at<float>(second, 32)) + at<float>(first, 32)));
    }
    float linear_change[2][3]{}, angular_change[2][3]{};
    ContactHistory linear_history[2], angular_history[2];
    int iteration = 0;
    for (; iteration < contact_iterations; ++iteration) {
        const float fraction = float(
            (E(1) / static_cast<long double>(contact_iterations)) *
            static_cast<long double>(iteration));
        float rotational[2][3], velocity[2][3], relative[3], momentum[3];
        cross_vectors(angular[0], arms[0], rotational[0]);
        for (unsigned i = 0; i < 3; ++i)
            velocity[0][i] = float(static_cast<long double>(rotational[0][i]) + linear[0][i]);
        cross_vectors(angular[1], arms[1], rotational[1]);
        for (unsigned i = 0; i < 3; ++i)
            velocity[1][i] = float(static_cast<long double>(rotational[1][i]) + linear[1][i]);
        for (unsigned i = 0; i < 3; ++i) {
            relative[i] = float(static_cast<long double>(velocity[0][i]) - velocity[1][i]);
            momentum[i] = float(static_cast<long double>(relative[i]) * mass);
        }
        const auto projection = hardware::dot_product(momentum, normal);
        const float normal_momentum = float(projection);
        if (projection >= 0.0L)
            break;
        const float approach = -normal_momentum;
        float normal_impulse[3], tangent_impulse[3]{};
        scale_triplet(normal_impulse, normal, approach);
        const int mode =
            contact_friction(momentum, normal_momentum, normal, friction, tangent_impulse);
        if (!iteration) {
            for (unsigned body = 0; body < 2; ++body) {
                void *state = bodies[body];
                at<float>(state, 608) = approach;
                at<Dword>(state, 656) = at<Dword>(bodies[1 - body], 40);
                for (unsigned i = 0; i < 3; ++i) {
                    at<float>(state, 640 + i * 4) = point[i];
                    at<float>(state, 612 + i * 4) = body ? -normal_impulse[i] : normal_impulse[i];
                    at<float>(state, 664 + i * 4) = normal[i];
                    at<float>(state, 624 + i * 4) =
                        body ? -tangent_impulse[i] : tangent_impulse[i];
                }
                at<float>(state, 660) = mass;
                at<void *>(state, 604) = bodies[1 - body];
                at<Dword>(state, 652) = 2;
                at<int>(state, 636) = mode;
            }
        }
        scale_triplet(normal_impulse, normal_impulse, fraction);
        scale_triplet(tangent_impulse, tangent_impulse, fraction);
        for (unsigned body = 0; body < 2; ++body)
            if (!frozen[body]) {
                if (body) {
                    for (unsigned i = 0; i < 3; ++i) {
                        normal_impulse[i] = -normal_impulse[i];
                        tangent_impulse[i] = -tangent_impulse[i];
                    }
                }
                float normal_linear[3]{}, normal_angular[3]{}, tangent_linear[3]{},
                    tangent_angular[3]{}, sum[3], delta[3];
                contact_force(bodies[body], normal_impulse, arms[body], directions[body],
                                       normal_linear, normal_angular);
                contact_force(bodies[body], tangent_impulse, arms[body], directions[body],
                                       tangent_linear, tangent_angular);
                sum_triplets(sum, tangent_linear, normal_linear);
                linear_history[body].append(sum);
                sum_triplets(sum, tangent_angular, normal_angular);
                angular_history[body].append(sum);
                linear_history[body].average(delta);
                accumulate_triplet(linear[body], delta);
                angular_history[body].average(delta);
                accumulate_triplet(angular[body], delta);
                linear_history[body].average(delta);
                accumulate_triplet(linear_change[body], delta);
                if (body)
                    weighted_history(&angular_history[body].values[0][0], delta);
                else
                    angular_history[body].average(delta);
                accumulate_triplet(angular_change[body], delta);
                at<Dword>(bodies[body], 60) = 0;
            }
    }
    if (!iteration)
        return 0;
    for (unsigned body = 0; body < 2; ++body) {
        float turns[3];
        scale_triplet(turns, angular_change[body], 0.15915493667125701904296875f);
        for (unsigned i = 0; i < 3; ++i)
            at<float>(bodies[body], 8 + i * 4) =
                float(static_cast<long double>(linear_change[body][i]) +
                      at<float>(bodies[body], 8 + i * 4));
        for (unsigned i = 0; i < 3; ++i)
            at<float>(bodies[body], 20 + i * 4) =
                float(static_cast<long double>(turns[i]) + at<float>(bodies[body], 20 + i * 4));
    }
    const float denominator = at<float>(second, 660);
    const auto impact =
        denominator != 0
            ? E(at<float>(second, 608)) / E(denominator)
            : E(at<float>(second, 608)) * at<float>(second, 36);
    submit_impact_entry(reinterpret_cast<float *>(bytes(second) + 680), at<Dword>(first, 40),
                           at<Dword>(second, 40), float(impact * 0.5f), at<Dword>(second, 32));
    return 1;
}
} // namespace nfs::game::runtime::physics::pending
