#include "ai_route_planner.hpp"
#include "ai_path_frame_x87.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
#include <cmath>
#include <cstring>
namespace nfs::game::runtime::ai::entities::pending::path_frame {
static long double horizontal_distance_squared(const float *first, const float *second) {
    const auto x = static_cast<long double>(second[0]) - first[0];
    const auto z = static_cast<long double>(second[2]) - first[2];
    const auto squared_z = z * z;
    const auto squared_x = x * x;
    return squared_z + squared_x;
}

long double advance_distance(void *path, float distance, int *offset, Dword *point) {
    const float *previous = static_cast<float *>(
        dependencies::section_node(at<void *>(path, 24), at<int>(path, 16)));
    const int count = lifecycle::path_length(path, 0);
    float traveled = 0;
    *offset = 1;
    while (*offset < count) {
        const float *next = lifecycle::pending::path_position(path, *offset);
        if (previous == next)
            break;
        const float squared = float(horizontal_distance_squared(previous, next));
        const float segment = float(__builtin_sqrtl(static_cast<long double>(squared)));
        const auto total = static_cast<long double>(segment) + traveled;
        traveled = float(total);
        if (static_cast<long double>(distance) < total)
            break;
        ++*offset;
        previous = next;
    }
    const int selected_offset = *offset;
    const Dword selected_point =
        reinterpret_cast<Dword>(lifecycle::pending::path_position(path, selected_offset));
    at<int>(path, 0) = selected_offset;
    *point = selected_point;
    return static_cast<long double>(traveled);
}

Dword sample_target(void *path, float *output, float *width, Dword *point, float *pose,
                        float distance, const float *position, Byte reverse) {
    *width = 0;
    const Dword section = at<Dword>(path, 24);
    *point = 0;
    if (!section || distance < 0)
        return 0;
    auto X = [](float value) { return static_cast<long double>(value); };
    float current_pose[21];
    lifecycle::sample_path(path, 0, current_pose);
    float delta[3];
    for (unsigned axis = 3; axis-- > 0;)
        delta[axis] = float(X(current_pose[axis]) - position[axis]);
    auto projection = hardware::dot_product(delta, current_pose + 3);
    if (at<int>(path, 20) < 0)
        projection = projection * -1.f;
    if (reverse)
        projection = projection * -1.f;
    distance = float(X(distance) - projection);
    int offset = 0;
    float traveled = 0;
    if (!(distance < 0)) {
        const float *previous = static_cast<float *>(
            dependencies::section_node(at<void *>(path, 24), at<int>(path, 16)));
        const int count = lifecycle::path_length(path, 0);
        offset = 1;
        while (offset < count) {
            const float *next = lifecycle::pending::path_position(path, offset);
            if (previous == next)
                break;
            const float squared = float(horizontal_distance_squared(previous, next));
            const float segment = float(__builtin_sqrtl(X(squared)));
            const auto total = X(segment) + traveled;
            traveled = float(total);
            if (X(distance) < total)
                break;
            ++offset;
            previous = next;
        }
    }
    *point = reinterpret_cast<Dword>(lifecycle::pending::path_position(path, offset));
    lifecycle::sample_path(path, offset, pose);
    const float node_width = at<float>(lifecycle::pending::path_position(path, offset), 24);
    float selected_width = minimum_path_width < node_width ? node_width : minimum_path_width;
    if (distance < traveled) {
        traveled = float(X(traveled) - distance);
        float previous_pose[21];
        lifecycle::sample_path(path, offset - 1, previous_pose);
        const float previous_node_width =
            at<float>(lifecycle::pending::path_position(path, offset - 1), 24);
        const float previous_width =
            minimum_path_width < previous_node_width ? previous_node_width : minimum_path_width;
        float segment_delta[3];
        for (unsigned axis = 0; axis < 3; ++axis)
            segment_delta[axis] = float(X(pose[axis]) - previous_pose[axis]);
        const auto segment = hardware::vector_length(segment_delta);
        if (X(traveled) < segment) {
            const float previous_weight = float(X(traveled) / segment);
            const float current_weight = float(X(1.f) - previous_weight);
            // Each product triplet and then the sum triplet has its own F32 stores.
            float current[3], previous[3], blended[3];
            for (unsigned vector : {0u, 18u, 12u, 15u, 3u, 6u}) {
                for (unsigned axis = 3; axis-- > 0;)
                    current[axis] = float(X(pose[vector + axis]) * current_weight);
                for (unsigned axis = 3; axis-- > 0;)
                    previous[axis] = float(X(previous_pose[vector + axis]) * previous_weight);
                for (unsigned axis = 3; axis-- > 0;)
                    blended[axis] = float(X(current[axis]) + previous[axis]);
                std::memcpy(pose + vector, blended, sizeof(blended));
            }
            for (unsigned axis = 3; axis-- > 0;)
                current[axis] = float(X(pose[9 + axis]) * current_weight);
            for (unsigned axis = 0; axis < 3; ++axis)
                previous[axis] = float(X(previous_weight) * previous_pose[9 + axis]);
            for (unsigned axis = 0; axis < 3; ++axis)
                blended[axis] = float(X(current[axis]) + previous[axis]);
            const auto blended_width =
                X(previous_weight) * previous_width + X(current_weight) * selected_width;
            std::memcpy(pose + 9, blended, sizeof(blended));
            selected_width = float(blended_width);
            physics::pending::normalize_collision_entry(pose + 3);
            physics::pending::normalize_collision_entry(pose + 6);
            physics::pending::normalize_collision_entry(pose + 9);
        }
    }
    for (unsigned axis = 0; axis < 3; ++axis)
        output[axis] = float(X(pose[axis]) + pose[18 + axis]);
    *width = selected_width;
    return 1;
}
void horizontal_distance_native80(const float *first, const float *second, void *output) {
    const long double result = horizontal_distance_squared(first, second);
    std::memcpy(output, &result, 10);
}
void advance_distance_native80(void *path, float distance, int *offset, Dword *point, void *output) {
    const long double result = advance_distance(path, distance, offset, point);
    std::memcpy(output, &result, 10);
}
void discard_advance_distance(void *path, float distance, int *offset, Dword *point) {
    static_cast<void>(advance_distance(path, distance, offset, point));
}
}
