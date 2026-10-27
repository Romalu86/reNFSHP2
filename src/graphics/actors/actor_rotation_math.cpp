#include "nfs/core/lifecycle/system_construction.hpp"
#include "../../math/precision/frame_x87_bridge.hpp"
#include "../../math/vectors/frame_x87_private.hpp"

namespace nfs::game::world::actor_resources {

void actor_axis_rotation(float *matrix, float degrees, float x, float y, float z) {
    const auto retained_angle = static_cast<long double>(3.1415927410125732f) *
                                0.0055555556900799274f * degrees;
    const float angle = float(retained_angle);
    if (retained_angle == 0.0L) {
        lifecycle::construction::identity_matrix(reinterpret_cast<kernel::Dword *>(matrix));
        return;
    }
    const auto sine = runtime::actor_sine_retained(angle);
    matrix[3] = 0.0f;
    matrix[7] = 0.0f;
    const auto cosine = runtime::actor_cosine_retained(angle);
    const auto squared_x = static_cast<long double>(x) * x;
    const auto squared_y = static_cast<long double>(y) * y;
    const auto squared_xy = squared_x + squared_y;
    const auto squared_z = static_cast<long double>(z) * z;
    const auto reciprocal = 1.0L / __builtin_sqrtl(squared_xy + squared_z);
    const auto axis_x = static_cast<long double>(x) * reciprocal;
    const float axis_y = float(reciprocal * y);
    const float axis_z = float(reciprocal * z);
    const auto complement = 1.0L - cosine;
    const auto product_x = complement * axis_x;
    const float product_y = float(complement * axis_y);
    const float product_z = float(complement * axis_z);
    const float sine_x = float(sine * axis_x);
    const float sine_y = float(sine * axis_y);
    const float sine_z = float(sine * axis_z);
    matrix[0] = float(product_x * axis_x + cosine);
    matrix[1] = float(static_cast<long double>(axis_y) * product_x + sine_z);
    matrix[2] = float(product_x * axis_z - sine_y);
    matrix[4] = float(static_cast<long double>(product_y) * axis_x - sine_z);
    matrix[5] = float(static_cast<long double>(product_y) * axis_y + cosine);
    matrix[6] = float(static_cast<long double>(product_y) * axis_z + sine_x);
    matrix[8] = float(static_cast<long double>(product_z) * axis_x + sine_y);
    matrix[9] = float(static_cast<long double>(product_z) * axis_y - sine_x);
    const auto final_product = static_cast<long double>(product_z) * axis_z;
    matrix[11] = 0.0f;
    matrix[12] = 0.0f;
    matrix[13] = 0.0f;
    const auto final_diagonal = final_product + cosine;
    matrix[14] = 0.0f;
    matrix[15] = 1.0f;
    matrix[10] = float(final_diagonal);
}
}
