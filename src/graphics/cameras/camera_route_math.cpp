#include "camera_route_math.hpp"
#include "nfs/ai/lifecycle/ai_dependencies.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
#include <cstring>

namespace nfs::game::world::camera::route_math {
namespace {
using runtime::at;
using runtime::invoke;

long double calculate_lateral_position(void *entity) {
    void *section = invoke<void *>(entity, 1);
    const int sample = invoke<int>(entity, 2);
    if (!section || sample < 0)
        return 0.0L;

    const void *node = runtime::ai::dependencies::section_node(section, sample);
    const long double retained_left = at<float>(node, 12);
    float right;
    std::memcpy(&right, runtime::bytes(node) + 16, sizeof(right));
    const float left = float(retained_left);
    if (retained_left <= static_cast<long double>(right))
        return 0.0L;

    float pose[21], delta[3];
    runtime::ai::dependencies::sample_section_pose(section, sample, pose);
    const float *position = invoke<const float *>(entity, 0);
    for (unsigned axis = 0; axis < 3; ++axis)
        delta[axis] = float(static_cast<long double>(position[axis]) - pose[axis]);
    const long double projection = runtime::hardware::dot_product(pose + 9, delta);
    const float projected = float(projection);
    long double result;
    if (projection >= static_cast<long double>(left))
        result = 1.0L;
    else if (projected <= right)
        result = -1.0L;
    else {
        const long double offset = static_cast<long double>(projected) - right;
        const long double doubled_offset = offset + offset;
        const long double width = static_cast<long double>(left) - right;
        result = doubled_offset / width - 1.0L;
    }
    if (at<int>(entity, 240) == 1)
        result = -result;
    return result;
}

long double calculate_heading_alignment(void *entity) {
    void *section = invoke<void *>(entity, 1);
    const int sample = invoke<int>(entity, 2);
    if (!section || sample < 0)
        return 1.0L;

    float heading[3], tangent[3];
    void *view = runtime::bytes(entity) + 4 + at<const int *>(entity, 4)[3];
    invoke(view, 2, heading);
    runtime::ai::dependencies::sample_horizontal_tangent(section, sample, tangent);
    long double result = runtime::hardware::dot_product(tangent, heading);
    if (at<int>(entity, 240) == -1)
        result = -result;
    return result;
}
} // namespace

void lateral_native80(void *entity, void *output) {
    const long double result = calculate_lateral_position(entity);
    std::memcpy(output, &result, 10);
}

void heading_native80(void *entity, void *output) {
    const long double result = calculate_heading_alignment(entity);
    std::memcpy(output, &result, 10);
}

bool heading_below_half(void *entity) {
    return calculate_heading_alignment(entity) < 0.5L;
}

bool heading_negative(void *entity) {
    return calculate_heading_alignment(entity) < 0.0L;
}

bool heading_at_least_half(void *entity) {
    return calculate_heading_alignment(entity) >= 0.5L;
}

bool lateral_beyond_three_quarters(void *entity) {
    long double side = calculate_lateral_position(entity);
    if (side < 0.0L)
        side = -side;
    return side > 0.75L;
}

bool lateral_negative_for_target(void *entity, void *target) {
    long double side = calculate_lateral_position(entity);
    const int target_direction = at<int>(target, 240);
    const int entity_direction = at<int>(entity, 240);
    if (target_direction != entity_direction)
        side = -side;
    return side < 0.0L;
}

bool lateral_above_three_quarters(void *entity) {
    return calculate_lateral_position(entity) > 0.75L;
}

float lateral_f32(void *entity) {
    return float(calculate_lateral_position(entity));
}
} // namespace nfs::game::world::camera::route_math
