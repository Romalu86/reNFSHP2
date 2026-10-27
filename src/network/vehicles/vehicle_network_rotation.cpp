#include "nfs/physics/contacts/vehicle_contacts.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
#include <cmath>
#include <cstring>
namespace nfs::game::runtime::physics {


void approach_network_pose(void *car) {
    void *body = at<void *>(car, 1768);
    auto X = [](float value) { return static_cast<long double>(value); };
    long double displacement[] = {X(0.f), X(0.f), X(0.f)};
    for (unsigned axis = 0; axis < 3; ++axis) {
        auto difference = X(at<float>(car, 3404 + axis * 4)) -
                          at<float>(body, 680 + axis * 4);
        if (difference < X(-.25f))
            difference = X(-.25f);
        else if (X(.25f) < difference)
            difference = X(.25f);
        displacement[axis] = difference;
        at<float>(body, 8 + axis * 4) = float(difference);
    }
    for (unsigned axis = 0; axis < 3; ++axis)
        at<float>(body, 680 + axis * 4) =
            float(displacement[axis] + at<float>(body, 680 + axis * 4));
    float rotation[9], current[4], target[4], blended[4];
    std::memcpy(rotation, bytes(body) + 692, sizeof(rotation));
    matrix_to_quaternion(rotation, current);
    matrix_to_quaternion(reinterpret_cast<float *>(bytes(car) + 3416), target);
    interpolate_rotation(current, target, 0.07999999821186065673828125f, blended);
    quaternion_to_matrix(blended, rotation);
    matrices::orthonormalize(rotation);
    body = at<void *>(car, 1768);
    std::memcpy(bytes(body) + 692, rotation, sizeof(rotation));
    matrices::transpose(reinterpret_cast<float *>(bytes(body) + 692),
                        reinterpret_cast<float *>(bytes(body) + 72));
}
} // namespace nfs::game::runtime::physics
