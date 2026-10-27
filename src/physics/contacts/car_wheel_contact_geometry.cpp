#include "../../graphics/vehicles/car_wheel_effects_runtime.hpp"
#include "../motion/world_motion_x87.hpp"
namespace nfs::game::runtime::physics::contacts_pending {

void wheel_effect_pose(void *car, int wheel, float *rotation, float *point, float *velocity) {
    void *body = at<void *>(car, 1768);
    const float x = at<float>(body, 692);
    at<Dword>(rotation, 12) = at<Dword>(body, 704);
    const Dword up_y = at<Dword>(body, 708);
    rotation[0] = x;
    const float y = at<float>(body, 696);
    at<Dword>(rotation, 16) = up_y;
    const Dword forward_z = at<Dword>(body, 724);
    rotation[1] = y;
    at<Dword>(rotation, 32) = forward_z;
    const float negative_z = -at<float>(body, 700);
    rotation[2] = negative_z;
    rotation[5] = -at<float>(body, 712);
    rotation[6] = -at<float>(body, 716);
    rotation[7] = -at<float>(body, 720);
    if (wheel == 0 || wheel == 3) {
        rotation[0] = -x;
        rotation[1] = -y;
        rotation[2] = -negative_z;
    }
    matrices::transform_positions(1, &at<float>(car, 2224 + Dword(wheel) * 296),
                                  &at<float>(body, 692), &at<float>(body, 680), point, 12);
    body = at<void *>(car, 1768);
    const Dword velocity_x = at<Dword>(body, 8);
    point[2] = -point[2];
    const float velocity_z = -at<float>(body, 16);
    at<Dword>(velocity, 0) = velocity_x;
    const Dword velocity_y = at<Dword>(body, 12);
    velocity[2] = velocity_z;
    at<Dword>(velocity, 4) = velocity_y;
}
// Keep the public compatibility aggregate wholly in MSVC.
kernel::numeric::Extended wheel_slip_strength(void *car, int index) {
    unsigned char value[10];
    x87::wheel_slip_strength_native80(car, index, value);
    return kernel::numeric::Extended::from_native80(value, kernel::numeric::current_format());
}
} // namespace nfs::game::runtime::physics::contacts_pending
