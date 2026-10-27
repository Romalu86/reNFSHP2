#include "ai_car_behavior.hpp"
#include "ai_path_frame_x87.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
#include "nfs/world/resources/runtime_resources.hpp"
#include <cfloat>
#include <cmath>
#include <cstring>
namespace nfs::game::runtime::ai::lifecycle::pending::path_frame {
void closest_sample(void *section, const float *position, int *sample, float *distance) {
    const int count = at<int>(section, 36);
    const float *nodes = count > 0 ? at<float *>(section, 40) : nullptr;
    int closest_sample = -1;
    float closest_squared = FLT_MAX;
    for (int i = 0; i < count; ++i) {
        const auto candidate = hardware::distance_squared(nodes + Dword(i) * 8, position);
        if (candidate < static_cast<long double>(closest_squared)) {
            closest_squared = float(candidate);
            closest_sample = i;
        }
    }
    const float *segment_nodes =
        count > 1 ? *reinterpret_cast<const float *const volatile *>(bytes(section) + 40)
                  : nullptr;
    for (int i = 1; i < count; ++i) {
        const float *previous = segment_nodes + Dword(i - 1) * 8, *current = segment_nodes + Dword(i) * 8;
        float delta[3], segment[3];
        for (unsigned axis = 0; axis < 3; ++axis)
            delta[axis] =
                float(static_cast<long double>(position[axis]) - static_cast<long double>(previous[axis]));
        for (unsigned axis = 0; axis < 3; ++axis)
            segment[axis] =
                float(static_cast<long double>(current[axis]) - static_cast<long double>(previous[axis]));
        const auto dot = hardware::dot_product(delta, segment);
        float fraction = float(dot);
        if (!(dot <= 0.0L)) {
            const auto length = static_cast<long double>(current[7]) - static_cast<long double>(previous[7]);
            const auto length_squared = length * length;
            if (!(length_squared <= static_cast<long double>(fraction))) {
                fraction = float(static_cast<long double>(fraction) / length_squared);
                for (unsigned axis = 0; axis < 3; ++axis)
                    segment[axis] = float(static_cast<long double>(segment[axis]) * fraction);
                const auto candidate = hardware::distance_squared(delta, segment);
                if (candidate < static_cast<long double>(closest_squared)) {
                    closest_squared = float(candidate);
                    closest_sample = fraction < 0.5f ? i - 1 : i;
                }
            }
        }
    }
    *sample = closest_sample;
    // The saved result is FSQRT followed by a binary32 store. FPU traps/status remain unaccepted.
    *distance = closest_sample == -1 ? -1.f : float(__builtin_sqrtl(static_cast<long double>(closest_squared)));
}


int nearest_node(void *section, const float *position) {
    float closest = FLT_MAX;
    int selected = -1;
    const int count = at<int>(section, 36);
    const float *nodes = count > 0 ? at<const float *>(section, 40) : nullptr;
    for (int index = 0; index < count; ++index) {
        const auto distance = hardware::distance_squared(nodes + Dword(index) * 8, position);
        if (distance < static_cast<long double>(closest)) {
            closest = float(distance);
            selected = index;
        }
    }
    return selected;
}

bool inside_route_edge(const float *point, const float *first, const float *second) {
    auto X = [](float value) { return static_cast<long double>(value); };
    const auto left = (X(first[0]) - second[0]) * (X(point[2]) - second[2]);
    const auto right = (X(point[0]) - second[0]) * (X(first[2]) - second[2]);
    return left <= right;
}

Byte point_in_quad(const float *point, const float *first, const float *second,
                         const float *third, const float *fourth) {
    return inside_route_edge(point, first, second) && inside_route_edge(point, second, third) &&
           inside_route_edge(point, third, fourth) && inside_route_edge(point, fourth, first);
}

void section_edge_point(const float *pose, unsigned edge, float *point) {
    for (unsigned axis = 0; axis < 3; ++axis)
        point[axis] = float(static_cast<long double>(pose[axis]) + pose[edge + axis]);
    point[2] = -point[2];
}

void *containing_section(void *path, const float *position) {
    if (!at<Dword>(path, 24) || !at<Dword>(path, 36) || !at<Dword>(path, 24))
        return nullptr;
    void *reference;
    int sample;
    float previous[21], current[21], following[21];
    locate_path_sample(path, -1, &reference, &sample);
    sample_section_pose(at<void *>(reference, 8), sample, previous);
    locate_path_sample(path, 0, &reference, &sample);
    sample_section_pose(at<void *>(reference, 8), sample, current);
    locate_path_sample(path, 1, &reference, &sample);
    sample_section_pose(at<void *>(reference, 8), sample, following);
    float edges[6][3];
    section_edge_point(following, 12, edges[0]);
    section_edge_point(following, 15, edges[1]);
    section_edge_point(current, 15, edges[2]);
    section_edge_point(previous, 15, edges[3]);
    section_edge_point(previous, 12, edges[4]);
    section_edge_point(current, 12, edges[5]);
    const float query[] = {position[0], position[1], -position[2]};
    if (point_in_quad(query, edges[5], edges[2], edges[1], edges[0])) {
        locate_path_sample(path, 1, &reference, &sample);
        return at<void *>(reference, 8);
    }
    if (point_in_quad(query, edges[4], edges[3], edges[2], edges[5]))
        return path_section_at(path, 0);
    return nullptr;
}

void construct_route(void *graph, void *reference, Dword *sample, Dword *direction,
                             const float *heading, const float *position, void *path, Dword use_path,
                             Dword) {
    void *selected = Byte(use_path) ? containing_section(path, position) : nullptr;
    int selected_sample;
    float selected_distance, preferred_distance = -1.f;
    if (selected) {
        closest_sample(selected, position, &selected_sample, &selected_distance);
    } else {
        const unsigned count = at<std::uint16_t>(graph, 16856);
        for (unsigned index = 0; index < count; ++index) {
            void *section = at<void *>(graph, 15656 + index * 4);
            int candidate_sample;
            float distance;
            closest_sample(section, position, &candidate_sample, &distance);
            if (Byte(use_path) && (section == path_current_section(bytes(path) + 4) ||
                             section == path_current_section(bytes(path) + 16) ||
                             section == path_current_section(bytes(path) + 28))) {
                const float *node = static_cast<const float *>(
                    dependencies::section_node(section, candidate_sample));
                float width = node[4] < 0.f ? -node[4] : node[4];
                const float other_width = node[3] < 0.f ? -node[3] : node[3];
                if (other_width > width)
                    width = other_width;
                distance = float(static_cast<long double>(distance) - width);
                if (preferred_distance == -1.f || distance < preferred_distance)
                    preferred_distance = distance;
            }
            if (!selected || distance < selected_distance) {
                selected = section;
                selected_sample = candidate_sample;
                selected_distance = distance;
            }
        }
    }
    if (!selected) {
        *sample = Dword(-1);
        *direction = 0;
        reset_path_reference(reference);
        return;
    }
    *sample = Dword(selected_sample);
    float horizontal_heading[] = {heading[0], 0, heading[2]}, tangent[3];
    normalize_vector(horizontal_heading, horizontal_heading);
    dependencies::sample_horizontal_tangent(selected, int(*sample), tangent);
    const long double projection = hardware::dot_product(horizontal_heading, tangent);
    *direction = !(0.0L <= projection) ? Dword(-1) : 1;
    at<Dword>(reference, 4) = at<Dword>(selected, *direction == 1 ? 20 : 24);
    at<void *>(reference, 0) = graph;
    at<Dword>(reference, 8) = 0;
    at<std::uint16_t>(reference, 44) = 0;
    at<void *>(reference, 8) = selected;
}
void node_distance_native80(const float *node, const float *position, void *output) {
    const long double result = hardware::distance_squared(node, position);
    std::memcpy(output, &result, 10);
}
void discard_node_distance(const float *node, const float *position) {
    static_cast<void>(hardware::distance_squared(node, position));
}
bool recovery_path_moved(const float *previous, const float *position) {
    return 100.0L < hardware::distance_squared(previous, position);
}
}
namespace nfs::game::runtime::ai::entities::pending::path_frame {
using lifecycle::pending::path_frame::closest_sample;

void closest_graph_section(void *graph, const float *position, Dword *section, int *sample) {
    *section = 0;
    *sample = -1;
    float closest = FLT_MAX;
    const unsigned count = at<std::uint16_t>(graph, 16856);
    for (unsigned index = 0; index < count; ++index) {
        void *candidate = at<void *>(graph, 15656 + index * 4);
        int candidate_sample;
        float distance;
        closest_sample(candidate, position, &candidate_sample,
                                                   &distance);
        if (distance < closest) {
            closest = distance;
            *section = reinterpret_cast<Dword>(candidate);
            *sample = candidate_sample;
        }
    }
}

void select_helicopter_destination(void *entity) {
    if (!at<Dword>(entity, 408) && at<int>(entity, 380) != 2) {
        at<Dword>(entity, 440) = 0;
        at<int>(entity, 444) = -1;
        return;
    }
    Dword *owner = system(76);
    const Dword group = invoke<Dword>(entity, 15);
    void *graph = bytes(owner) + 4 + group * 42080;
    if (!at<Dword>(entity, 408)) {
        int selected = -1;
        float farthest = 0;
        for (unsigned index = 0; index < at<std::uint16_t>(graph, 16858); ++index) {
            float nearest = FLT_MAX;
            for (unsigned field = 84164; field < 84172; field += 4)
                if (void *vehicle = at<void *>(system(76), field)) {
                    const float *location =
                        static_cast<float *>(dependencies::path_group_entry(graph, index));
                    const float *position = invoke<const float *>(vehicle, 0);
                    const auto distance = hardware::distance_squared(location, position);
                    if (distance < static_cast<long double>(nearest))
                        nearest = float(distance);
                }
            if (farthest < nearest) {
                farthest = nearest;
                selected = int(index);
            }
        }
        if (selected < 0)
            selected = 0;
        std::memcpy(bytes(entity) + 424, dependencies::path_group_entry(graph, selected), 12);
    }
    closest_graph_section(graph, at<const float *>(entity, 436),
                          reinterpret_cast<Dword *>(bytes(entity) + 440),
                          reinterpret_cast<int *>(bytes(entity) + 444));
}
}
