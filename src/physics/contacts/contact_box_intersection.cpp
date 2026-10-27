#include "contact_boxes.hpp"
#include "../motion/world_motion_x87.hpp"
namespace nfs::game::runtime::physics::pending {
// Preserve six face samples, the two inclusive 5x5x5 sample grids,
// the thirty-pass separation cap and the saved asymmetric frozen-body moves.
int collide_pair(void *first, void *second) {
    using namespace boxes;
    auto X = [](float value) { return static_cast<long double>(value); };
    for (unsigned row = 0; row < 3; ++row)
        for (unsigned column = 0; column < 3; ++column)
            relative_rotation[row * 3 + column] = float(
                hardware::dot_product(reinterpret_cast<float *>(bytes(first) + 692 + row * 12),
                                  reinterpret_cast<float *>(bytes(second) + 692 + column * 12)));
    for (unsigned row = 0; row < 3; ++row)
        for (unsigned column = 0; column < 3; ++column)
            scaled_second[row * 3 + column] = float(X(relative_rotation[row * 3 + column]) *
                                                    at<float>(second, 120 + column * 4));
    for (unsigned row = 0; row < 3; ++row)
        for (unsigned column = 0; column < 3; ++column)
            scaled_first[row * 3 + column] =
                float(X(relative_rotation[row * 3 + column]) * at<float>(first, 120 + row * 4));
    int result = 0;
    const float faces[6][3] = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}, {-1, 0, 0}, {0, -1, 0}, {0, 0, -1}};
    for (unsigned pass = 0; pass < 30; ++pass) {
        float difference[3];
        for (unsigned i = 0; i < 3; ++i)
            difference[i] =
                float(X(at<float>(second, 680 + i * 4)) - at<float>(first, 680 + i * 4));
        for (unsigned i = 0; i < 3; ++i)
            relative_first[i] = float(hardware::dot_product(
                reinterpret_cast<float *>(bytes(first) + 692 + i * 12), difference));
        for (unsigned i = 0; i < 3; ++i)
            relative_second[i] = float(hardware::dot_product(
                reinterpret_cast<float *>(bytes(second) + 692 + i * 12), difference));
        bool found = false;
        for (const auto &face : faces)
            if (second_face(first, second, face[0], face[1], face[2])) {
                found = true;
                break;
            }
        if (!found) {
            for (float x = -1; x <= 1 && !found; x += 0.5f)
                for (float y = -1; y <= 1 && !found; y += 0.5f)
                    for (float z = -1; z <= 1 && !found; z += 0.5f) {
                        for (unsigned axis = 0; axis < 3; ++axis)
                            probe_reversed[2 - axis] =
                                float(X(scaled_second[axis * 3 + 2]) * z +
                                      X(scaled_second[axis * 3]) * x +
                                      X(scaled_second[axis * 3 + 1]) * y + relative_first[axis]);
                        if (inside(first)) {
                            for (unsigned axis = 0; axis < 3; ++axis)
                                collision_point[axis] =
                                    float(X(at<float>(second, 120)) *
                                              at<float>(second, 692 + axis * 4) * x +
                                          X(at<float>(second, 124)) *
                                              at<float>(second, 704 + axis * 4) * y +
                                          X(at<float>(second, 128)) *
                                              at<float>(second, 716 + axis * 4) * z +
                                          at<float>(second, 680 + axis * 4));
                            select_normal(first, second, 0);
                            found = true;
                        }
                    }
        }
        if (!found)
            for (const auto &face : faces)
                if (first_face(first, second, face[0], face[1], face[2])) {
                    found = true;
                    break;
                }
        if (!found)
            for (float x = -1; x <= 1 && !found; x += 0.5f)
                for (float y = -1; y <= 1 && !found; y += 0.5f)
                    for (float z = -1; z <= 1 && !found; z += 0.5f)
                        if (first_sample(first, second, x, y, z)) {
                            select_normal(first, second, 1);
                            found = true;
                        }
        if (!found)
            return result;
        Byte frozen_first = at<Byte>(first, 730), frozen_second = at<Byte>(second, 730);
        if (frozen_first && frozen_second)
            frozen_first = frozen_second = 0;
        if (at<float>(second, 600) < at<float>(first, 60))
            frozen_first = 1;
        if (at<float>(first, 600) < at<float>(second, 60))
            frozen_second = 1;
        if (frozen_first && frozen_second)
            frozen_first = frozen_second = 0;
        float direction[3], first_move[3], second_move[3];
        copy_triplet(direction, difference);
        hardware::normalize_collision(direction);
        const float inverse = float(X(1) / (X(at<float>(second, 32)) + at<float>(first, 32)));
        scale_vectors(1, direction, float(-(X(inverse) * at<float>(second, 32) * 0.0333333350718021392822265625f)),
                      first_move);
        scale_vectors(1, direction, float(X(inverse) * at<float>(first, 32) * 0.0333333350718021392822265625f),
                      second_move);
        if (!frozen_first) {
            float *position = reinterpret_cast<float *>(bytes(first) + 680);
            accumulate_triplet(position, first_move);
            if (!frozen_second)
                accumulate_triplet(reinterpret_cast<float *>(bytes(second) + 680), second_move);
            else
                for (unsigned i = 0; i < 3; ++i)
                    position[i] = float(X(position[i]) - second_move[i]);
        } else {
            float *position = reinterpret_cast<float *>(bytes(second) + 680);
            for (unsigned i = 0; i < 3; ++i)
                position[i] = float(X(position[i]) - first_move[i]);
            accumulate_triplet(position, second_move);
        }
        result = 1;
    }
    return 1;
}
} // namespace nfs::game::runtime::physics::pending
