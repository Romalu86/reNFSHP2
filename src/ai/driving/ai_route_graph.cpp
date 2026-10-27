#include "ai_route_planner.hpp"
#include <cfloat>
#include <cmath>
namespace nfs::game::runtime::ai::entities::pending {
using kernel::numeric::Extended;

RouteCandidates *construct_route_candidates(RouteCandidates *route) {
    route->count = 0;
    return route;
}
// Complete RET leaf.
void destroy_route_candidates(RouteCandidates *) {
}

void append_route_vertex(void *graph, const float *position) {
    auto &count = at<std::uint16_t>(graph, 25208);
    if (count >= 300)
        return;
    void *vertex = bytes(graph) + 8 + count++ * 84;
    for (unsigned axis = 0; axis < 3; ++axis)
        at<float>(vertex, axis * 4) = position[axis];
    at<Byte>(vertex, 78) = 0;
    at<float>(vertex, 80) = FLT_MAX;
    at<std::uint16_t>(vertex, 76) = 0;
}

void connect_route_vertices(void *graph, void *section) {
    void *first = bytes(graph) + 8 + at<Dword>(section, 20) * 84;
    void *second = bytes(graph) + 8 + at<Dword>(section, 24) * 84;
    auto connect = [&](void *from, void *to) {
        auto &count = at<std::uint16_t>(from, 76);
        if (count < 8) {
            at<void *>(from, 12 + count * 8) = to;
            at<void *>(from, 16 + count * 8) = section;
            ++count;
        }
    };
    connect(first, second);
    connect(second, first);
}

Extended find_vertex_route(void *graph, int start, int finish, RouteCandidates *route) {
    const auto format = kernel::numeric::current_format();
    const auto count = at<std::uint16_t>(graph, 25208);
    for (unsigned index = 0; index < count; ++index) {
        void *vertex = bytes(graph) + 8 + index * 84;
        at<Byte>(vertex, 78) = 0;
        at<float>(vertex, 80) = FLT_MAX;
    }
    void *root = bytes(graph) + 8 + Dword(start) * 84;
    at<void *>(graph, 4) = root;
    at<Dword>(root, 80) = 0;
    for (;;) {
        void *selected = nullptr;
        for (unsigned index = 0; index < count; ++index) {
            void *vertex = bytes(graph) + 8 + index * 84;
            if (!at<Byte>(vertex, 78) &&
                (!selected || at<float>(vertex, 80) < at<float>(selected, 80)))
                selected = vertex;
        }
        if (!selected)
            break;
        const auto neighbors = at<std::uint16_t>(selected, 76);
        const float distance = at<float>(selected, 80);
        at<Byte>(selected, 78) = 1;
        for (unsigned edge = 0; edge < neighbors; ++edge) {
            void *neighbor = at<void *>(selected, 12 + edge * 8);
            void *section = at<void *>(selected, 16 + edge * 8);
            const auto candidate = Extended(at<float>(section, 44), format) + distance + 0.01f;
            if (candidate < Extended(at<float>(neighbor, 80), format))
                at<float>(neighbor, 80) = float(candidate);
        }
    }
    return trace_vertex_route(graph, finish, route);
}

Extended trace_vertex_route(void *graph, int finish, RouteCandidates *route) {
    const auto format = kernel::numeric::current_format();
    void *target = bytes(graph) + 8 + Dword(finish) * 84;
    RouteCandidates reverse;
    construct_route_candidates(&reverse);
    if (at<float>(target, 80) < FLT_MAX) {
        void *root = at<void *>(graph, 4);
        void *cursor = target;
        while (cursor != root) {
            const auto count = at<std::uint16_t>(cursor, 76);
            const float threshold = float(Extended(at<float>(cursor, 80), format) + 0.01f);
            for (unsigned edge = 0; edge < count; ++edge) {
                void *neighbor = at<void *>(cursor, 12 + edge * 8);
                void *section = at<void *>(cursor, 16 + edge * 8);
                const auto distance =
                    Extended(at<float>(neighbor, 80), format) + at<float>(section, 44) + 0.01f;
                if (!(Extended(threshold, format) < distance) && !std::isnan(float(distance)) &&
                    !std::isnan(threshold)) {
                    cursor = neighbor;
                    if (reverse.count < 300)
                        reverse.sections[reverse.count++] = reinterpret_cast<Dword>(section);
                    break;
                }
            }
        }
    }
    route->count = 0;
    for (int index = int(reverse.count) - 1; index >= 0; --index)
        if (route->count < 300)
            route->sections[route->count++] = reverse.sections[index];
    const Extended distance(at<float>(target, 80), format);
    destroy_route_candidates(&reverse);
    return distance;
}
} // namespace nfs::game::runtime::ai::entities::pending
