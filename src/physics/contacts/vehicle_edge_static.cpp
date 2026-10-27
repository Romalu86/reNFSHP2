#include "vehicle_edge_queries.hpp"
#include <cstring>
static_assert(__LDBL_MANT_DIG__ == 64 && __LDBL_MAX_EXP__ == 16384,
              "These edge queries require native x87 intermediates");
namespace nfs::game::runtime::physics::edges {
namespace {
long double retained(float value) {
    return static_cast<long double>(value);
}

bool inlined_crossing(const float *a, const float *b, const float *start, const float *end) {
    auto ordered_le = [](long double left, long double right) {
        return left <= right;
    };
    auto less_or_unordered = [](long double left, long double right) {
        return !(left >= right);
    };
    const auto edge_x = retained(a[0]) - b[0];
    const auto edge_z = retained(a[2]) - b[2];
    const auto start_z = (retained(start[2]) - b[2]) * edge_x;
    const auto start_x = (retained(start[0]) - b[0]) * edge_z;
    if (!ordered_le(start_z, start_x))
        return false;
    const auto end_z = (retained(end[2]) - b[2]) * edge_x;
    const auto end_x = (retained(end[0]) - b[0]) * edge_z;
    if (!less_or_unordered(end_x, end_z))
        return false;
    const auto motion_x = retained(start[0]) - end[0];
    const auto motion_z = retained(start[2]) - end[2];
    const auto first_z = (retained(a[2]) - end[2]) * motion_x;
    const auto first_x = (retained(a[0]) - end[0]) * motion_z;
    if (!less_or_unordered(first_x, first_z))
        return false;
    const auto second_z = (retained(b[2]) - end[2]) * motion_x;
    const auto second_x = (retained(b[0]) - end[0]) * motion_z;
    return ordered_le(second_z, second_x);
}
} // namespace

Byte *static_cell(const float *position) {
    int x, z;
    cell_coordinates(position, &x, &z);
    return pending::collision_grid + (x * 64 + z) * 24;
}

Dword intersect_static_shape(const void *shape, const float *start, const float *end, float *normal,
                            float *point, Dword *reference) {
    float corners[4][3];
    const auto height = retained(start[1]);
    for (unsigned i = 0; i < 4; ++i) {
        const Dword corner_x = at<Dword>(shape, i * 8);
        corners[i][1] = float(height);
        std::memcpy(corners[i], &corner_x, sizeof corner_x);
        const Dword corner_z = at<Dword>(shape, i * 8 + 4);
        std::memcpy(corners[i] + 2, &corner_z, sizeof corner_z);
    }
    for (unsigned edge = 0; edge < 4; ++edge) {
        const float *a = corners[edge], *b = corners[(edge + 1) & 3];
        if (!(edge == 3 ? crosses_directed_edge(a, b, start, end)
                        : inlined_crossing(a, b, start, end)))
            continue;
        const auto normal_x = retained(a[2]) - b[2];
        Dword point_x, point_y, point_z;
        std::memcpy(&point_x, a, sizeof point_x);
        normal[0] = float(normal_x);
        std::memcpy(point, &point_x, sizeof point_x);
        std::memcpy(&point_y, a + 1, sizeof point_y);
        const auto next_x = retained(b[0]);
        std::memcpy(&point_z, a + 2, sizeof point_z);
        const auto normal_z = next_x - a[0];
        // Output vectors may overlap; the original writes the point between normal stores.
        std::memcpy(point + 1, &point_y, sizeof point_y);
        std::memcpy(point + 2, &point_z, sizeof point_z);
        normal[2] = float(normal_z);
        *reference = 1176256512;
        const Dword zero = 0;
        std::memcpy(normal + 1, &zero, sizeof zero);
        const long double length = pending::normalize_collision_entry(normal);
        return length != 0;
    }
    return 0;
}
} // namespace nfs::game::runtime::physics::edges
