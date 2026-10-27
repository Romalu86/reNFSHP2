#include "ai_route_planner.hpp"
#include "../../math/vectors/retained_vector.hpp"
#include "nfs/physics/contacts/vehicle_contacts.hpp"
#include <cmath>
namespace nfs::game::runtime::ai::entities::pending {
using kernel::numeric::Extended;

void plan_graph_route(void *graph, void *current, Dword current_sample, const float *position,
                      void *target, int target_sample, const float *destination, void *output,
                      int *endpoint, float *distance) {
    auto *route = static_cast<RouteCandidates *>(output);
    const auto format = kernel::numeric::current_format();
    auto projection = [&](void *section, int sample, const float *point) {
        float tangent[3], delta[3];
        track::path_direction(section, sample, tangent);
        const auto *node = static_cast<const float *>(dependencies::section_node(section, sample));
        for (unsigned axis = 0; axis < 3; ++axis)
            delta[axis] = float(Extended(point[axis], format) - node[axis]);
        return runtime::retained_dot_product(delta, tangent);
    };
    const float destination_projection = float(projection(target, target_sample, destination));
    const float current_projection = float(projection(current, int(current_sample), position));
    if (current == target) {
        route->count = 0;
        *endpoint = -1;
        const float target_distance = float(
            Extended(at<float>(dependencies::section_node(target, target_sample), 28), format) +
            destination_projection);
        *distance =
            float(Extended(target_distance, format) -
                  at<float>(dependencies::section_node(current, int(current_sample)), 28) -
                  current_projection);
        return;
    }
    if (at<Byte>(graph, 16860)) {
        at<std::uint16_t>(graph, 42076) = 0;
        for (unsigned index = 0; index < at<std::uint16_t>(graph, 16858); ++index)
            append_route_vertex(bytes(graph) + 16868,
                                static_cast<float *>(dependencies::path_group_entry(graph, index)));
        for (unsigned index = 0; index < at<std::uint16_t>(graph, 16856); ++index)
            connect_route_vertices(bytes(graph) + 16868, at<void *>(graph, 15656 + index * 4));
        at<Byte>(graph, 16860) = 0;
    }
    const float current_begin =
        float(Extended(at<float>(dependencies::section_node(current, int(current_sample)), 28),
                       format) +
              current_projection);
    const float current_end = float(Extended(at<float>(current, 44), format) - current_begin);
    const float target_begin =
        float(Extended(at<float>(dependencies::section_node(target, target_sample), 28), format) +
              destination_projection);
    const float target_end = float(Extended(at<float>(target, 44), format) - target_begin);
    auto magnitude = [&](Extended value) { return value < Extended(0.f, format) ? -value : value; };
    void *vertices = bytes(graph) + 16868;
    *distance = float(magnitude(
        find_vertex_route(vertices, at<int>(current, 20), at<int>(target, 20), route) +
        target_begin + current_begin));
    RouteCandidates candidates[3];
    for (auto &candidate : candidates)
        construct_route_candidates(&candidate);
    int selected = -1;
    auto candidate = magnitude(
        find_vertex_route(vertices, at<int>(current, 20), at<int>(target, 24), candidates) +
        target_end + current_begin);
    if (candidate < Extended(*distance, format)) {
        *distance = float(candidate);
        selected = 0;
    }
    candidate = magnitude(
        find_vertex_route(vertices, at<int>(current, 24), at<int>(target, 20), candidates + 1) +
        target_begin + current_end);
    if (candidate < Extended(*distance, format)) {
        *distance = float(candidate);
        selected = 1;
    }
    candidate = magnitude(
        find_vertex_route(vertices, at<int>(current, 24), at<int>(target, 24), candidates + 2) +
        target_end + current_end);
    // The fourth saved comparison selects the route without storing a new distance.
    if (candidate < Extended(*distance, format))
        selected = 2;
    if (selected != -1 && route != candidates + selected) {
        const auto count = candidates[selected].count;
        route->count = 0;
        for (unsigned index = 0; index < count; ++index)
            if (route->count < 300)
                route->sections[route->count++] = candidates[selected].sections[index];
    }
    *endpoint = at<int>(current, selected > 0 ? 24 : 20);
    for (int index = 2; index >= 0; --index)
        destroy_route_candidates(candidates + index);
}
} // namespace nfs::game::runtime::ai::entities::pending
