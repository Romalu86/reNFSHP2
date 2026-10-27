#include "vehicle_edge_queries.hpp"
static_assert(__LDBL_MANT_DIG__ == 64 && __LDBL_MAX_EXP__ == 16384,
              "These edge queries require native x87 intermediates");
namespace nfs::game::runtime::physics::edges {
namespace {
long double retained(float value) {
    return static_cast<long double>(value);
}
bool ordered_le(long double left, long double right) {
    return left <= right;
}
bool less_or_unordered(long double left, long double right) {
    return !(left >= right);
}
} // namespace

Dword crosses_directed_edge(const float *first, const float *second, const float *start,
                           const float *end) {
    const auto edge_x = retained(first[0]) - second[0];
    const auto edge_z = retained(first[2]) - second[2];
    const auto start_z = (retained(start[2]) - second[2]) * edge_x;
    const auto start_x = (retained(start[0]) - second[0]) * edge_z;
    if (less_or_unordered(start_x, start_z))
        return 0;
    const auto end_z = (retained(end[2]) - second[2]) * edge_x;
    const auto end_x = (retained(end[0]) - second[0]) * edge_z;
    if (ordered_le(end_z, end_x))
        return 0;
    const auto motion_x = retained(start[0]) - end[0];
    const auto motion_z = retained(start[2]) - end[2];
    const auto first_z = (retained(first[2]) - end[2]) * motion_x;
    const auto first_x = (retained(first[0]) - end[0]) * motion_z;
    if (ordered_le(first_z, first_x))
        return 0;
    const auto second_z = (retained(second[2]) - end[2]) * motion_x;
    const auto second_x = (retained(second[0]) - end[0]) * motion_z;
    if (less_or_unordered(second_x, second_z))
        return 0;
    return 1;
}
// Callers consume AL; Y of the query is not read.
Byte contains_point(Dword *grid, const float *point) {
    void *entry = track::spatial_cell(grid, point);
    if (!entry)
        return 0;
    const unsigned count = at<std::uint16_t>(entry, 0);
    const auto *indices = at<const std::uint16_t *>(entry, 4);
    for (unsigned i = 0; i < count; ++i) {
        float *a, *b, *c, *d;
        track::polygon_vertices(grid, indices + i, &a, &b, &c, &d);
        if (!ordered_le((retained(a[0]) - b[0]) * (retained(point[2]) - b[2]),
                        (retained(point[0]) - b[0]) * (retained(a[2]) - b[2])))
            continue;
        if (!ordered_le((retained(point[2]) - c[2]) * (retained(b[0]) - c[0]),
                        (retained(point[0]) - c[0]) * (retained(b[2]) - c[2])))
            continue;
        if (!ordered_le((retained(point[2]) - d[2]) * (retained(c[0]) - d[0]),
                        (retained(c[2]) - d[2]) * (retained(point[0]) - d[0])))
            continue;
        if (ordered_le((retained(d[0]) - a[0]) * (retained(point[2]) - a[2]),
                       (retained(point[0]) - a[0]) * (retained(d[2]) - a[2])))
            return 1;
    }
    return 0;
}

Byte conditional_edge_blocked(const float *first, const float *second, void *const *neighbors) {
    const float dx = float(retained(second[0]) - first[0]);
    const auto retained_dz = retained(second[2]) - first[2];
    const float dz = float(retained_dz);
    if (retained_dz == 0 && dx == 0)
        return 0;
    const float squared = float(retained(dz) * dz + retained(dx) * dx);
    const float length = float(__builtin_sqrtl(retained(squared)));
    if (length == 0)
        return 0;
    const auto inverse = 1.0L / length;
    float point[3];
    point[0] = float(retained(dz) * inverse * 0.1f + retained(dx) * 0.5f + first[0]);
    point[2] = float(retained(dz) * 0.5f + first[2] - inverse * dx * 0.1f);
    for (unsigned neighbor = 0; neighbor < 5; ++neighbor)
        if (neighbors[neighbor] && contains_point(static_cast<Dword *>(neighbors[neighbor]), point))
            return 0;
    return 1;
}

int collect_boundary_edge(void *polygon, void *const *neighbors, float *const *corners,
                          const float *start, const float *end) {
    for (unsigned edge = 0; edge < 4; ++edge) {
        const auto code = at<std::int8_t>(polygon, 60 + edge);
        const Dword material = at<Byte>(polygon, 59) & 15;
        if (!code || (surface_filter != material && surface_filter && material))
            continue;
        const float *a = corners[edge], *b = corners[(edge + 1) & 3];
        // Each retained difference is reused by both endpoint comparisons.
        const auto edge_x = retained(a[0]) - b[0];
        const auto edge_z = retained(a[2]) - b[2];
        const auto start_z = (retained(start[2]) - b[2]) * edge_x;
        const auto start_x = (retained(start[0]) - b[0]) * edge_z;
        if (!ordered_le(start_z, start_x))
            continue;
        const auto end_z = (retained(end[2]) - b[2]) * edge_x;
        const auto end_x = (retained(end[0]) - b[0]) * edge_z;
        if (!less_or_unordered(end_x, end_z))
            continue;
        const auto motion_x = retained(start[0]) - end[0];
        const auto motion_z = retained(start[2]) - end[2];
        const auto first_z = (retained(a[2]) - end[2]) * motion_x;
        const auto first_x = (retained(a[0]) - end[0]) * motion_z;
        if (!less_or_unordered(first_x, first_z))
            continue;
        const auto second_z = (retained(b[2]) - end[2]) * motion_x;
        const auto second_x = (retained(b[0]) - end[0]) * motion_z;
        if (!ordered_le(second_z, second_x))
            continue;
        if (code != -1 || conditional_edge_blocked(a, b, neighbors))
            return int(edge);
    }
    return -1;
}
} // namespace nfs::game::runtime::physics::edges
