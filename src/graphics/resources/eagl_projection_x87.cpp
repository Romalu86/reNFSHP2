#include "nfs/graphics/math/eagl_math.hpp"
#include "../../math/precision/frame_x87_bridge.hpp"

static_assert(__LDBL_MANT_DIG__ == 64 && __LDBL_MAX_EXP__ == 16384,
              "Projection arithmetic requires native x87 intermediates");

namespace nfs::graphics::math {
// Depth remains retained after the first F32 store.
void orthographic(float *matrix, float width, float height, float near_plane, float far_plane) {
    matrix[0] = static_cast<float>(2.0L / width);
    for (unsigned column = 1; column < 5; ++column)
        matrix[column] = 0;
    matrix[5] = static_cast<float>(2.0L / height);
    for (unsigned column = 6; column < 10; ++column)
        matrix[column] = 0;
    const auto depth_scale = 1.0L / (static_cast<long double>(near_plane) - far_plane);
    matrix[10] = static_cast<float>(depth_scale);
    matrix[11] = 0;
    matrix[12] = 0;
    matrix[13] = 0;
    matrix[14] = static_cast<float>(depth_scale * near_plane);
    matrix[15] = 1;
}

// Only the vertical and depth reciprocals pass through F32.
void orthographic_bounds(float *matrix, float left, float right, float bottom, float top,
                         float near_plane, float far_plane) {
    const auto inverse_width = 1.0L / (static_cast<long double>(right) - left);
    const float inverse_height = static_cast<float>(1.0L / (static_cast<long double>(top) - bottom));
    matrix[0] = static_cast<float>(inverse_width * 2.f);
    for (unsigned column = 1; column < 5; ++column)
        matrix[column] = 0;
    matrix[5] = static_cast<float>(static_cast<long double>(inverse_height) * 2.f);
    for (unsigned column = 6; column < 10; ++column)
        matrix[column] = 0;
    const float depth_scale = static_cast<float>(1.0L / (static_cast<long double>(near_plane) - far_plane));
    matrix[10] = depth_scale;
    matrix[11] = 0;
    matrix[12] = static_cast<float>(-((static_cast<long double>(left) + right) * inverse_width));
    matrix[13] = static_cast<float>(-((static_cast<long double>(bottom) + top) * inverse_height));
    matrix[14] = static_cast<float>(static_cast<long double>(depth_scale) * near_plane);
    matrix[15] = 1;
}

// The retained axis-x and explicit F32 spills are distinct.
void append_axis_rotation(float *matrix, float degrees, float axis_x, float axis_y,
                            float axis_z) {
    const auto retained_radians = static_cast<long double>(3.1415927410125732421875f) *
                                  0.0055555556900799274444580078125f * degrees;
    const float radians = static_cast<float>(retained_radians);
    float rotation[16];
    if (retained_radians == 0.0L)
        graphics::math::diagonal(rotation, 1, 1, 1, 1);
    else {
        const auto sine = game::runtime::actor_sine_retained(radians);
        const auto cosine = game::runtime::actor_cosine_retained(radians);
        const auto square = (static_cast<long double>(axis_x) * axis_x +
                             static_cast<long double>(axis_y) * axis_y) +
                            static_cast<long double>(axis_z) * axis_z;
        const auto inverse = 1.0L / __builtin_sqrtl(square);
        const auto x = static_cast<long double>(axis_x) * inverse;
        const float y = static_cast<float>(inverse * axis_y);
        const float z = static_cast<float>(inverse * axis_z);
        const auto complement = 1.0L - cosine;
        const auto product_x = complement * x;
        const float product_y = static_cast<float>(complement * y);
        const float product_z = static_cast<float>(complement * z);
        const float sine_x = static_cast<float>(sine * x);
        const float sine_y = static_cast<float>(sine * y);
        const float sine_z = static_cast<float>(sine * z);
        rotation[0] = static_cast<float>(product_x * x + cosine);
        rotation[1] = static_cast<float>(static_cast<long double>(y) * product_x + sine_z);
        rotation[2] = static_cast<float>(product_x * z - sine_y);
        rotation[3] = 0;
        rotation[4] = static_cast<float>(static_cast<long double>(product_y) * x - sine_z);
        rotation[5] = static_cast<float>(static_cast<long double>(product_y) * y + cosine);
        rotation[6] = static_cast<float>(static_cast<long double>(product_y) * z + sine_x);
        rotation[7] = 0;
        rotation[8] = static_cast<float>(static_cast<long double>(product_z) * x + sine_y);
        rotation[9] = static_cast<float>(static_cast<long double>(product_z) * y - sine_x);
        rotation[10] = static_cast<float>(static_cast<long double>(product_z) * z + cosine);
        rotation[11] = rotation[12] = rotation[13] = rotation[14] = 0;
        rotation[15] = 1;
    }
    append(matrix, rotation);
}
} // namespace nfs::graphics::math
