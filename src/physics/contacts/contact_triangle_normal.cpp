#include "nfs/physics/frame/dynamics_runtime.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
namespace nfs::game::runtime::physics {
// Degenerate edge pairs are retried in the order.
float *triangle_normal(float *output, const float *triangle) {
    float first[3], second[3], normal[3];
    const unsigned pairs[3][4] = {{0, 1, 0, 2}, {0, 1, 1, 2}, {1, 2, 0, 2}};
    for (unsigned attempt = 0; attempt < 3; ++attempt) {
        const unsigned *pair = pairs[attempt];
        for (unsigned i = 0; i < 3; ++i)
            first[i] = float(static_cast<long double>(triangle[pair[0] * 3 + i]) -
                             triangle[pair[1] * 3 + i]);
        hardware::normalize_collision(first);
        for (unsigned i = 0; i < 3; ++i)
            second[i] = float(static_cast<long double>(triangle[pair[2] * 3 + i]) -
                              triangle[pair[3] * 3 + i]);
        hardware::normalize_collision(second);
        cross_vectors(first, second, normal);
        if (attempt == 2)
            break;
        const auto length = hardware::vector_length(normal);
        if (length != 0.0L)
            break;
    }
    hardware::normalize_collision(normal);
    if (normal[1] < 0)
        for (float &component : normal)
            component = -component;
    return copy_triplet(output, normal);
}
} // namespace nfs::game::runtime::physics
