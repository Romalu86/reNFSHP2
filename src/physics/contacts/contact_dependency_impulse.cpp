#include "nfs/physics/contacts/contact_dependencies.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
#include <cmath>
namespace nfs::game::runtime::physics::contact {

Dword move_out_of_plane(void *body, const float *normal, const float *point, float *vertex) {
    auto E = [](float value) { return static_cast<long double>(value); };
    float displacement[3] = {float(E(vertex[0]) - point[0]), float(E(vertex[1]) - point[1]),
                              float(E(vertex[2]) - point[2])};
    const auto distance = hardware::dot_product(displacement, normal);
    if (distance < E(0)) {
        scale_vectors(1, normal, static_cast<float>(-distance), displacement);
        const float body_x = float(E(displacement[0]) + at<float>(body, 680));
        const float body_y = float(E(displacement[1]) + at<float>(body, 684));
        const auto body_z = E(displacement[2]) + at<float>(body, 688);
        at<float>(body, 680) = body_x;
        at<float>(body, 684) = body_y;
        at<float>(body, 688) = float(body_z);
        for (unsigned axis = 0; axis < 3; ++axis)
            vertex[axis] = float(E(displacement[axis]) + vertex[axis]);
        return 1;
    }
    return 0;
}

int apply_impulse(void *body, const float *normal, const float *point, float travel,
                  Dword first_friction, Dword second_friction, float ceiling, Dword flags) {
    float vertices[26 * 3];
    matrices::transform_positions(26, reinterpret_cast<float *>(bytes(body) + 140),
                                  reinterpret_cast<float *>(bytes(body) + 692),
                                  reinterpret_cast<float *>(bytes(body) + 680), vertices, 12);
    float closest = travel, selected[3];
    for (unsigned index = 0; index < 26; ++index) {
        const float *vertex = vertices + index * 3;
        if (ceiling < vertex[1])
            continue;
        float from_plane[3], from_centre[3];
        for (unsigned axis = 0; axis < 3; ++axis) {
            from_plane[axis] = vertex[axis] - point[axis];
            from_centre[axis] = vertex[axis] - at<float>(body, 680 + axis * 4);
        }
        const float distance = static_cast<float>(hardware::dot_product(from_plane, normal));
        const auto centre_distance = hardware::dot_product(from_centre, normal);
        if (distance < closest && centre_distance <
            0.0L) {
            closest = distance;
            for (unsigned axis = 0; axis < 3; ++axis)
                selected[axis] = vertex[axis];
        }
    }
    if (!(closest < travel))
        return 0;
    move_out_of_plane(body, normal, point, selected);
    const Dword friction[] = {first_friction, second_friction};
    return physics::pending::resolve_ground_point(
        body, selected, normal, reinterpret_cast<const float *>(friction), flags);
}
} // namespace nfs::game::runtime::physics::contact
