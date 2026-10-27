#include "vehicle_edge_queries.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
#include <cstring>
namespace nfs::game::runtime::physics::edges {
Dword surface_filter{};
namespace {
int finish_polygon_contact(void *polygon, unsigned edge, float *normal) {
    float surface_normal[3];
    contact::decode_normal(polygon, surface_normal);
    const long double horizontal_length = hardware::normalize_collision(normal);
    if (!horizontal_length)
        return 0;
    normal[1] = float(-hardware::dot_product(surface_normal, normal));
    const long double length = hardware::normalize_collision(normal);
    return length != 0 ? at<Byte>(polygon, 60 + edge) : 0;
}

int starting_polygon_contact(void *polygon, unsigned edge, float *const *corners, float *normal,
                             float *point, Dword *reference) {
    const float *a = corners[edge], *b = corners[(edge + 1) & 3];
    Dword point_x, point_y, point_z;
    std::memcpy(&point_x, a, sizeof point_x);
    std::memcpy(point, &point_x, sizeof point_x);
    std::memcpy(&point_y, a + 1, sizeof point_y);
    std::memcpy(&point_z, a + 2, sizeof point_z);
    std::memcpy(point + 1, &point_y, sizeof point_y);
    std::memcpy(point + 2, &point_z, sizeof point_z);
    const auto normal_x = static_cast<long double>(a[2]) - b[2];
    *reference = 1176256512;
    const Dword zero = 0;
    std::memcpy(normal + 1, &zero, sizeof zero);
    normal[0] = float(normal_x);
    normal[2] = float(static_cast<long double>(b[0]) - a[0]);
    return finish_polygon_contact(polygon, edge, normal);
}

int ending_polygon_contact(void *polygon, unsigned edge, float *const *corners, float *normal,
                           float *point, Dword *reference) {
    const float *a = corners[edge], *b = corners[(edge + 1) & 3];
    const auto first_z = static_cast<long double>(a[2]);
    Dword point_x, point_y, point_z;
    std::memcpy(&point_x, a, sizeof point_x);
    std::memcpy(point, &point_x, sizeof point_x);
    std::memcpy(&point_y, a + 1, sizeof point_y);
    std::memcpy(&point_z, a + 2, sizeof point_z);
    const auto normal_x = first_z - b[2];
    std::memcpy(point + 1, &point_y, sizeof point_y);
    std::memcpy(point + 2, &point_z, sizeof point_z);
    normal[0] = float(normal_x);
    normal[2] = float(static_cast<long double>(b[0]) - a[0]);
    *reference = 1176256512;
    const Dword zero = 0;
    std::memcpy(normal + 1, &zero, sizeof zero);
    return finish_polygon_contact(polygon, edge, normal);
}
} // namespace
} // namespace nfs::game::runtime::physics::edges
namespace nfs::game::runtime::physics::contacts_pending {
// Edge codes are zero-extended into EAX.
int find_body_edge_contact(const float *start, const float *end, float *normal, float *point,
                           Dword *reference, Dword player, Dword surface) {
    using namespace edges;
    surface_filter = surface;
    Byte *start_static = static_cell(start);
    auto visit_static = [&](Byte *cell) {
        if (!cell)
            return false;
        const int count = at<int>(cell, 0);
        const auto *indices = at<const std::uint16_t *>(cell, 4);
        for (int index = 0; index < count; ++index)
            if (intersect_static_shape(
                    world::dependency::static_track_shape(world::subsystem(24), indices[index]),
                    start, end, normal, point, reference))
                return true;
        return false;
    };
    if (visit_static(start_static))
        return 1;
    Byte *end_static = static_cell(end);
    if (start_static != end_static && visit_static(end_static))
        return 1;
    void *boundaries[6];
    boundaries[3] = boundaries[4] = boundaries[5] = nullptr;
    const int selected_player = static_cast<std::int32_t>(player) < 0 ? 0 : int(player);
    for (int side = 0; side < 3; ++side)
        boundaries[side] = track::boundary(static_cast<Dword *>(world::subsystem(36)),
                                           2 - side, selected_player);
    if (static_cast<std::int32_t>(player) < 0 && split_screen())
        for (int side = 0; side < 3; ++side)
            boundaries[3 + side] =
                track::boundary(static_cast<Dword *>(world::subsystem(36)), 2 - side, 1);
    for (unsigned slot = 0; slot < 6; ++slot) {
        auto *grid = static_cast<Dword *>(boundaries[slot]);
        if (!grid)
            continue;
        const unsigned cursor = slot + 4;
        void *neighbors[5] = {boundaries[(cursor - 3) % 6], boundaries[(cursor - 2) % 6],
                              boundaries[(cursor - 1) % 6], boundaries[cursor % 6],
                              boundaries[(cursor + 1) % 6]};
        void *start_cell = track::spatial_cell(grid, start);
        if (start_cell) {
            const unsigned count = at<std::uint16_t>(start_cell, 2);
            const auto *indices = at<const std::uint16_t *>(start_cell, 4);
            for (unsigned index = 0; index < count; ++index) {
                void *polygon = contact::polygon_at(grid, indices[index]);
                float *corners[4];
                contact::polygon_corners(polygon, corners);
                const int edge = collect_boundary_edge(polygon, neighbors, corners, start, end);
                if (edge != -1)
                    if (const int result = starting_polygon_contact(
                            polygon, unsigned(edge), corners, normal, point, reference))
                        return result;
            }
        }
        void *end_cell = track::spatial_cell(grid, end);
        if (end_cell && start_cell != end_cell) {
            const unsigned count = at<std::uint16_t>(end_cell, 2);
            const auto *indices = at<const std::uint16_t *>(end_cell, 4);
            for (unsigned index = 0; index < count; ++index) {
                void *polygon = contact::polygon_at(grid, indices[index]);
                float *corners[4];
                contact::polygon_corners(polygon, corners);
                for (unsigned edge = 0; edge < 4; ++edge) {
                    const auto code = at<std::int8_t>(polygon, 60 + edge);
                    const Dword material = at<Byte>(polygon, 59) & 15;
                    if (!code || (surface_filter != material && surface_filter && material))
                        continue;
                    const unsigned next = (edge + 1) & 3;
                    if (!crosses_directed_edge(corners[edge], corners[next], start, end))
                        continue;
                    if (code == -1 &&
                        !conditional_edge_blocked(corners[edge], corners[next], neighbors))
                        continue;
                    if (const int result =
                            ending_polygon_contact(polygon, edge, corners, normal, point, reference))
                        return result;
                    break;
                }
            }
        }
    }
    return 0;
}
} // namespace nfs::game::runtime::physics::contacts_pending
