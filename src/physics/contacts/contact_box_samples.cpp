#include "contact_boxes.hpp"
#include "../motion/world_motion_x87.hpp"
namespace nfs::game::runtime::physics::boxes {
float relative_rotation[9]{}, relative_first[3]{}, relative_second[3]{}, scaled_first[9]{},
    scaled_second[9]{}, probe_reversed[3]{};
namespace {
long double E(float value) { return static_cast<long double>(value); }
void sample_first(float x, float y, float z) {
    for (unsigned axis = 0; axis < 3; ++axis)
        probe_reversed[2 - axis] = float((E(scaled_first[axis]) * x +
                                          E(scaled_first[3 + axis]) * y +
                                          E(scaled_first[6 + axis]) * z) -
                                         relative_second[axis]);
}
void world_point(void *body, float x, float y, float z) {
    for (unsigned axis = 0; axis < 3; ++axis)
        pending::collision_point[axis] =
            float(E(at<float>(body, 692 + axis * 4)) *
                      at<float>(body, 120) * x +
                  E(at<float>(body, 704 + axis * 4)) *
                      at<float>(body, 124) * y +
                  E(at<float>(body, 716 + axis * 4)) *
                      at<float>(body, 128) * z +
                  at<float>(body, 680 + axis * 4));
}
void world_face_point(void *body, float x, float y, float z, bool second) {
    for (unsigned axis = 0; axis < 3; ++axis) {
        const auto term_y = second && axis == 1
                                ? E(at<float>(body, 124)) * at<float>(body, 704 + axis * 4) * y
                                : E(y) * at<float>(body, 124) * at<float>(body, 704 + axis * 4);
        const auto term_z = second
                                ? E(at<float>(body, 716 + axis * 4)) * at<float>(body, 128) * z
                                : E(at<float>(body, 128)) * at<float>(body, 716 + axis * 4) * z;
        const auto term_x = second && axis == 1
                                ? E(at<float>(body, 692 + axis * 4)) * at<float>(body, 120) * x
                                : second ? E(x) * at<float>(body, 120) * at<float>(body, 692 + axis * 4)
                                         : E(x) * at<float>(body, 692 + axis * 4) * at<float>(body, 120);
        const auto sum = second && axis == 1 ? (term_y + term_x) + term_z
                                             : (term_y + term_z) + term_x;
        pending::collision_point[axis] = float(sum + at<float>(body, 680 + axis * 4));
    }
}
} // namespace

int second_face(void *first, void *second, float x, float y, float z) {
    for (unsigned axis = 0; axis < 3; ++axis)
        probe_reversed[2 - axis] =
            float(E(scaled_second[axis * 3]) * x + E(scaled_second[axis * 3 + 1]) * y +
                  E(scaled_second[axis * 3 + 2]) * z + relative_first[axis]);
    if (!inside(first))
        return 0;
    for (unsigned axis = 0; axis < 3; ++axis)
        pending::collision_normal[axis] =
            float((E(z) * at<float>(second, 716 + axis * 4) +
                   E(x) * at<float>(second, 692 + axis * 4)) +
                  E(y) * at<float>(second, 704 + axis * 4));
    world_face_point(second, x, y, z, true);
    hardware::normalize_collision(pending::collision_normal);
    return 1;
}

int first_face(void *first, void *second, float x, float y, float z) {
    sample_first(x, y, z);
    if (!inside(second))
        return 0;
    for (unsigned axis = 0; axis < 3; ++axis)
        pending::collision_normal[axis] =
            float(-((E(z) * at<float>(first, 716 + axis * 4) +
                     E(x) * at<float>(first, 692 + axis * 4)) +
                    E(y) * at<float>(first, 704 + axis * 4)));
    world_face_point(first, x, y, z, false);
    hardware::normalize_collision(pending::collision_normal);
    return 1;
}

int first_sample(void *first, void *second, float x, float y, float z) {
    sample_first(x, y, z);
    if (!inside(second))
        return 0;
    world_point(first, x, y, z);
    return 1;
}
} // namespace nfs::game::runtime::physics::boxes
