#include "nfs/graphics/math/eagl_math.hpp"
#include <cmath>

static_assert(__LDBL_MANT_DIG__ == 64 && __LDBL_MAX_EXP__ == 16384,
              "Perspective arithmetic requires retained x87 intermediates");

namespace nfs::graphics::math {

void perspective(float *matrix, float radians, float aspect, float near_plane, float far_plane) {
    auto X = [](float value) { return static_cast<long double>(value); };
    const float half_angle = float(X(radians) * 0.5f);
    // Separate sine and cosine calls supply the projection factors.
    const float cosine = float(std::cos(double(half_angle)));
    const float sine = float(std::sin(double(half_angle)));
    const auto cotangent = X(cosine) / X(sine);
    reinterpret_cast<volatile kernel::Dword *>(matrix)[11] = 3212836864u;
    matrix[0] = float(cotangent / X(aspect));
    for (unsigned column = 1; column < 5; ++column)
        matrix[column] = 0.0f;
    matrix[5] = float(cotangent);
    for (unsigned column = 6; column < 10; ++column)
        matrix[column] = 0.0f;
    const auto depth_scale = X(far_plane) / (X(near_plane) - far_plane);
    matrix[10] = float(depth_scale); // FST preserves the extended value for the next product.
    matrix[12] = 0.0f;
    matrix[13] = 0.0f;
    matrix[14] = float(depth_scale * near_plane);
    matrix[15] = 0.0f;
}

} // namespace nfs::graphics::math
