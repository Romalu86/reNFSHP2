#include "nfs/ai/lifecycle/ai_dependencies.hpp"
#include <initializer_list>
#include <utility>

namespace nfs::game::runtime::ai::dependencies {
namespace {
bool nonzero_component(float value) {
    // TEST AH,44h / JP also selects the unordered comparison.
    return value != 0;
}
} // namespace

void sample_section_tangent(void *section, int index, float *output) {
    const float *point =
        reinterpret_cast<const float *>(bytes(at<void *>(section, 40)) + index * 32);
    const int count = at<int>(section, 36);
    if (count < 2) {
        for (unsigned axis = 0; axis < 3; ++axis)
            output[axis] = saved::zero_vector[axis];
        return;
    }
    const float *end = count - 1 <= index ? point : point + 8;
    const float *begin = count - 1 <= index ? point - 8 : point;
    for (unsigned axis = 0; axis < 3; ++axis)
        output[axis] = end[axis] - begin[axis];
}

void sample_horizontal_tangent(void *section, int index, float *output) {
    const float *point =
        reinterpret_cast<const float *>(bytes(at<void *>(section, 40)) + index * 32);
    const int count = at<int>(section, 36);
    if (count < 2)
        for (unsigned axis = 0; axis < 3; ++axis)
            output[axis] = 0;
    else {
        const float *end = index < count - 1 ? point + 8 : point;
        const float *begin = index < count - 1 ? point : point - 8;
        for (unsigned axis = 0; axis < 3; ++axis)
            output[axis] = end[axis] - begin[axis];
    }
    output[1] = 0;
    if (nonzero_component(output[0]) || nonzero_component(output[2]))
        normalize_vector(output, output);
}

void sample_section_pose(void *section, int index, float *output) {
    const auto *point = reinterpret_cast<const float *>(section_node(section, index));
    output[0] = point[0];
    const float y = point[1], z = point[2];
    output[1] = y;
    output[2] = z;
    sample_section_tangent(section, index, output + 3);
    if (nonzero_component(output[3]) || nonzero_component(output[4]) ||
        nonzero_component(output[5]))
        normalize_vector(output + 3, output + 3);
    output[6] = output[3];
    output[7] = 0;
    output[8] = output[5];
    if (nonzero_component(output[6]) || nonzero_component(output[8]))
        normalize_vector(output + 6, output + 6);
    output[9] = output[8];
    output[10] = 0;
    output[13] = 0;
    output[16] = 0;
    output[19] = 0;
    output[11] = -output[6];
    output[12] = point[3] * output[9];
    output[14] = point[3] * output[11];
    output[15] = point[4] * output[9];
    output[17] = point[4] * output[11];
    output[18] = point[5] * output[9];
    output[20] = point[5] * output[11];
}

void reverse_section_pose(float *pose) {
    for (unsigned index : {3u, 4u, 5u, 6u, 8u, 9u, 11u})
        pose[index] = -pose[index];
    std::swap(pose[12], pose[15]);
    std::swap(pose[14], pose[17]);
}

void sample_route_pose(void *section, int index, float *position, float *orientation,
                       Byte reverse) {
    float pose[21];
    sample_section_pose(section, index, pose);
    if (reverse)
        reverse_section_pose(pose);
    position[0] = pose[0];
    position[1] = pose[1];
    position[2] = -pose[2];
    float heading[3] = {pose[3], 0, -pose[5]};
    if (!nonzero_component(heading[0]) && !nonzero_component(heading[2]))
        heading[0] = 1;
    normalize_vector(heading, heading);
    orientation[0] = heading[2];
    orientation[1] = 0;
    orientation[2] = -heading[0];
    orientation[3] = 0;
    orientation[4] = 1;
    orientation[5] = 0;
    orientation[6] = heading[0];
    orientation[7] = 0;
    orientation[8] = heading[2];
}
} // namespace nfs::game::runtime::ai::dependencies
