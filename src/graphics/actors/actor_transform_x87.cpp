#include "nfs/graphics/math/eagl_math.hpp"
#include "../../math/precision/frame_x87_bridge.hpp"

static_assert(__LDBL_MANT_DIG__ == 64 && __LDBL_MAX_EXP__ == 16384,
              "Actor transforms require the native x87 intermediate format");

namespace nfs::game::world::actor_resources {
// The retained axis-x and explicit F32 spills are distinct.
void prepend_actor_rotation(float *matrix, float degrees, float axis_x, float axis_y,
                            float axis_z) {
    const auto retained_radians = static_cast<long double>(3.1415927410125732421875f) *
                                  0.0055555556900799274444580078125f * degrees;
    const float radians = static_cast<float>(retained_radians);
    float rotation[16];
    if (retained_radians == 0.0L)
        graphics::math::diagonal(rotation, 1, 1, 1, 1);
    else {
        const auto sine = runtime::actor_sine_retained(radians);
        const auto cosine = runtime::actor_cosine_retained(radians);
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
    float result[16];
    graphics::math::multiply(rotation, matrix, result);
    graphics::math::copy(matrix, result);
}
} // namespace nfs::game::world::actor_resources
