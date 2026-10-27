#include "nfs/physics/frame/simulation_runtime.hpp"
#include "../../physics/vehicles/car_physics_frame.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
#include <cmath>
namespace nfs::game::runtime::physics::pending {
void update_ground_state(void *);
void integrate_forces(void *);
} // namespace nfs::game::runtime::physics::pending
namespace nfs::game::runtime::physics {

void project_velocity(void *state) {
    const float *velocity = reinterpret_cast<const float *>(bytes(state) + 520);
    at<float>(state, 108) =
        float(hardware::dot_product(reinterpret_cast<float *>(bytes(state) + 692), velocity));
    at<float>(state, 112) =
        float(hardware::dot_product(reinterpret_cast<float *>(bytes(state) + 704), velocity));
    at<float>(state, 116) =
        float(hardware::dot_product(reinterpret_cast<float *>(bytes(state) + 716), velocity));
}
} // namespace nfs::game::runtime::physics
namespace nfs::game::runtime::pending {

void prepare_vehicle(void *state) {
    auto X = [](float value) { return static_cast<long double>(value); };
    if (at<Dword>(state, 732))
        --at<Dword>(state, 732);
    if (at<Byte>(state, 728)) {
        at<Dword>(state, 504) = at<Dword>(state, 488) = 0;
        const auto z = X(at<float>(state, 16)), x = X(at<float>(state, 8));
        const float square = float(x * x + z * z);
        at<float>(state, 592) = float(__builtin_sqrtl(X(square)));
        physics::pending::update_ground_state(state);
        physics::project_velocity(state);
    }
    bool integrate = true, skip_forces = false;
    if (!at<Byte>(state, 5))
        if (void *car = at<void *>(state, 0))
            if (physics::car_is_remote(car)) {
                const auto mode = at<std::int16_t>(car, 7940);
                if (mode == 5)
                    integrate = false;
                else if (mode == 2) {
                    integrate = false;
                    float value = at<float>(car, 7948);
                    if (value < 0)
                        value = -value;
                    if (value < 0.1f)
                        skip_forces = true;
                }
            }
    if (!skip_forces) {
        if (at<Byte>(state, 728)) {
            const float sleep = at<float>(state, 60);
            if (sleep != 0) {
                for (unsigned offset = 8; offset <= 28; offset += 4)
                    at<Dword>(state, offset) = 0;
            } else
                at<float>(state, 12) =
                    float(X(at<float>(state, 12)) -
                          X(physics::car_frame::gravity) * 0.0333333350718021392822265625f);
        }
        if (integrate)
            physics::pending::integrate_forces(state);
    }
    float *angular = reinterpret_cast<float *>(bytes(state) + 20);
    const float angular_turns = float(hardware::vector_length(angular) * 6.283185482025146484375f);
    float *linear = reinterpret_cast<float *>(bytes(state) + 8);
    const float linear_length = float(hardware::vector_length(linear));
    const auto inertia = hardware::vector_length(reinterpret_cast<float *>(bytes(state) + 452));
    at<float>(state, 600) =
        float(inertia * angular_turns * angular_turns * 0.5f +
              X(linear_length) * at<float>(state, 32) * linear_length * 0.5f);
    if (at<Byte>(state, 5)) {
        if (at<std::uint16_t>(state, 596) > 15) {
            for (unsigned offset = 8; offset <= 28; offset += 4)
                at<float>(state, offset) = at<float>(state, offset) * 0.99f;
        }
        const auto magnitude = hardware::vector_length(angular);
        if (X(10.f) < magnitude)
            scale_vectors(1, angular, float(X(10.f) / magnitude), angular);
    }
}
} // namespace nfs::game::runtime::pending
