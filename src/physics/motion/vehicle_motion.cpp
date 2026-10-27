#include "nfs/physics/solver/solver_runtime.hpp"
#include "nfs/graphics/resources/eagl_graphics.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
#include <intrin.h>
namespace nfs::game::runtime::physics::pending {

void advance_body(void *state) {
    auto X = [](float value) { return static_cast<long double>(value); };
    float position[3], rotation[9], before[3], after[3];
    __movsd(reinterpret_cast<unsigned long *>(position),
            reinterpret_cast<const unsigned long *>(bytes(state) + 680), 3);
    __movsd(reinterpret_cast<unsigned long *>(rotation),
            reinterpret_cast<const unsigned long *>(bytes(state) + 692), 9);
    matrices::transform_positions(1, reinterpret_cast<float *>(bytes(state) + 44), rotation,
                                  position, before, 12);
    matrices::rotate(rotation, at<float>(state, 20) * 0.0333333350718021392822265625f,
                     at<float>(state, 24) * 0.0333333350718021392822265625f,
                     at<float>(state, 28) * 0.0333333350718021392822265625f, rotation);
    matrices::transform_positions(1, reinterpret_cast<float *>(bytes(state) + 44), rotation,
                                  position, after, 12);
    position[0] =
        float((X(at<float>(state, 8)) * 0.0333333350718021392822265625f + before[0]) -
              after[0] + position[0]);
    position[1] =
        float((X(at<float>(state, 12)) * 0.0333333350718021392822265625f + before[1]) -
              after[1] + position[1]);
    at<float>(state, 680) = position[0];
    at<float>(state, 684) = position[1];
    const Dword tick = graphics::unresolved::camera_state;
    position[2] =
        float((X(at<float>(state, 16)) * 0.0333333350718021392822265625f + before[2]) -
              after[2] + position[2]);
    at<float>(state, 688) = position[2];
    if (Byte(tick >> 1) == at<Byte>(state, 4))
        matrices::orthonormalize(rotation);
    __movsd(reinterpret_cast<unsigned long *>(bytes(state) + 692),
            reinterpret_cast<const unsigned long *>(rotation), 9);
    matrices::transpose(rotation, reinterpret_cast<float *>(bytes(state) + 72));
    project_velocity(state);
}

void advance_car(void *state) {
    auto X = [](float value) { return static_cast<long double>(value); };
    if (state && !at<Byte>(state, 5))
        if (void *car = at<void *>(state, 0))
            if (car_is_remote(car)) {
                if (interpolate_vehicle_entry(system(104) + 76 / 4, 0.01666666753590106964111328125f, car)) {
                    float rotation[9];
                    __movsd(reinterpret_cast<unsigned long *>(rotation),
                            reinterpret_cast<const unsigned long *>(bytes(state) + 692), 9);
                    matrices::orthonormalize(rotation);
                    __movsd(reinterpret_cast<unsigned long *>(bytes(state) + 692),
                            reinterpret_cast<const unsigned long *>(rotation), 9);
                    matrices::transpose(rotation, reinterpret_cast<float *>(bytes(state) + 72));
                    project_velocity(state);
                    return;
                }
            }
    float position[3], rotation[9], before[3], after[3];
    __movsd(reinterpret_cast<unsigned long *>(position),
            reinterpret_cast<const unsigned long *>(bytes(state) + 680), 3);
    __movsd(reinterpret_cast<unsigned long *>(rotation),
            reinterpret_cast<const unsigned long *>(bytes(state) + 692), 9);
    matrices::transform_positions(1, reinterpret_cast<float *>(bytes(state) + 44), rotation,
                                  position, before, 12);
    matrices::rotate(rotation, at<float>(state, 20) * 0.01666666753590106964111328125f,
                     at<float>(state, 24) * 0.01666666753590106964111328125f,
                     at<float>(state, 28) * 0.01666666753590106964111328125f, rotation);
    matrices::transform_positions(1, reinterpret_cast<float *>(bytes(state) + 44), rotation,
                                  position, after, 12);
    position[0] =
        float((X(at<float>(state, 8)) * 0.01666666753590106964111328125f + before[0]) -
              after[0] + position[0]);
    position[1] =
        float((X(at<float>(state, 12)) * 0.01666666753590106964111328125f + before[1]) -
              after[1] + position[1]);
    at<float>(state, 680) = position[0];
    at<float>(state, 684) = position[1];
    const Dword tick = graphics::unresolved::camera_state;
    position[2] =
        float((X(at<float>(state, 16)) * 0.01666666753590106964111328125f + before[2]) -
              after[2] + position[2]);
    at<float>(state, 688) = position[2];
    if (Byte(tick) == at<Byte>(state, 4))
        matrices::orthonormalize(rotation);
    __movsd(reinterpret_cast<unsigned long *>(bytes(state) + 692),
            reinterpret_cast<const unsigned long *>(rotation), 9);
    matrices::transpose(rotation, reinterpret_cast<float *>(bytes(state) + 72));
    project_velocity(state);
}
} // namespace nfs::game::runtime::physics::pending
