#include "nfs/physics/frame/dynamics_runtime.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
#include <cmath>
#include <intrin.h>
namespace nfs::game::runtime::physics::pending {
// Shape-specific support-triangle tables.
Byte collide_shape(void *body) {
    matrices::transform_positions(26, reinterpret_cast<float *>(bytes(body) + 140),
                                  reinterpret_cast<float *>(bytes(body) + 692),
                                  reinterpret_cast<float *>(bytes(body) + 680),
                                  shape_vertices, 12);
    if (!at<Byte>(body, 5))
        return 0;
    const Dword *triangles = nullptr;
    unsigned count = 0;
    switch (at<Dword>(body, 136)) {
    case 0:
    case 2:
        count = 12;
        triangles = support_triangles + 40 * 3;
        break;
    case 1:
        count = 6;
        triangles = support_triangles + 58 * 3;
        break;
    case 3:
    case 4:
        count = 40;
        triangles = support_triangles;
        break;
    case 5:
        count = 6;
        triangles = support_triangles + 52 * 3;
        break;
    default:
        error_source = shape_source_path;
        physics_assertion_line = 1769;
        prepare_error();
        report_runtime_error(shape_error_message);
        break;
    }
    if (!count)
        return 0;
    const float x = at<float>(body, 680), z = at<float>(body, 688);
    const auto X = [](float value) { return static_cast<long double>(value); };
    const auto at_most = [](long double left, long double right) {
        return left <= right;
    };
    for (unsigned i = 0; i < count; ++i, triangles += 3) {
        const float *a = shape_vertices + triangles[0] * 3,
                    *b = shape_vertices + triangles[1] * 3,
                    *c = shape_vertices + triangles[2] * 3;
        if (at_most((X(z) - b[2]) * (X(c[0]) - b[0]), (X(c[2]) - b[2]) * (X(x) - b[0])) &&
            at_most((X(z) - a[2]) * (X(b[0]) - a[0]), (X(x) - a[0]) * (X(b[2]) - a[2])) &&
            at_most((X(a[0]) - c[0]) * (X(z) - c[2]), (X(a[2]) - c[2]) * (X(x) - c[0]))) {
            float triangle[9];
            __movsd(reinterpret_cast<unsigned long *>(triangle),
                    reinterpret_cast<const unsigned long *>(a), 3);
            __movsd(reinterpret_cast<unsigned long *>(triangle + 3),
                    reinterpret_cast<const unsigned long *>(b), 3);
            __movsd(reinterpret_cast<unsigned long *>(triangle + 6),
                    reinterpret_cast<const unsigned long *>(c), 3);
            return Byte(triangle_support(body, triangle, 0.076200000941753387451171875f));
        }
    }
    return 0;
}
} // namespace nfs::game::runtime::physics::pending
