#include "nfs/graphics/math/eagl_math.hpp"

static_assert(__LDBL_MANT_DIG__ == 64 && __LDBL_MAX_EXP__ == 16384,
              "Matrix arithmetic requires the native x87 intermediate format");

namespace nfs::graphics::math {
// All four results precede the first output store.
void transform_row(const float *row, const float *matrix, float *output) {
    const auto x_w = static_cast<long double>(row[0]) * matrix[3];
    const auto x_z = static_cast<long double>(row[0]) * matrix[2];
    const auto x_y = static_cast<long double>(row[0]) * matrix[1];
    const auto x_x = static_cast<long double>(row[0]) * matrix[0];
    const auto y_x = static_cast<long double>(row[1]) * matrix[4];
    const auto y_z = static_cast<long double>(row[1]) * matrix[6];
    const auto y_y = static_cast<long double>(row[1]) * matrix[5];
    const auto y_w = static_cast<long double>(row[1]) * matrix[7];
    auto result_x = x_x + y_x;
    auto result_y = x_y + y_y;
    auto result_z = x_z + y_z;
    auto result_w = x_w + y_w;
    const auto z_x = static_cast<long double>(row[2]) * matrix[8];
    const auto z_z = static_cast<long double>(row[2]) * matrix[10];
    const auto z_y = static_cast<long double>(row[2]) * matrix[9];
    const auto z_w = static_cast<long double>(row[2]) * matrix[11];
    result_x += z_x;
    result_y += z_y;
    result_z += z_z;
    result_w += z_w;
    const auto w_x = static_cast<long double>(row[3]) * matrix[12];
    const auto w_z = static_cast<long double>(row[3]) * matrix[14];
    const auto w_y = static_cast<long double>(row[3]) * matrix[13];
    const auto w_w = static_cast<long double>(row[3]) * matrix[15];
    result_x += w_x;
    result_y += w_y;
    result_z += w_z;
    result_w += w_w;
    output[0] = static_cast<float>(result_x);
    output[1] = static_cast<float>(result_y);
    output[2] = static_cast<float>(result_z);
    output[3] = static_cast<float>(result_w);
}

// Translation is added to the retained three-product sums.
void transform_position(const float *position, const float *matrix, float *output) {
    const auto x_y = static_cast<long double>(position[0]) * matrix[1];
    const auto x_z = static_cast<long double>(position[0]) * matrix[2];
    const auto x_x = static_cast<long double>(position[0]) * matrix[0];
    const auto y_x = static_cast<long double>(position[1]) * matrix[4];
    const auto y_y = static_cast<long double>(position[1]) * matrix[5];
    const auto y_z = static_cast<long double>(position[1]) * matrix[6];
    auto result_x = x_x + y_x;
    auto result_y = x_y + y_y;
    auto result_z = x_z + y_z;
    const auto z_x = static_cast<long double>(position[2]) * matrix[8];
    const auto z_y = static_cast<long double>(position[2]) * matrix[9];
    const auto z_z = static_cast<long double>(position[2]) * matrix[10];
    result_x += z_x;
    result_y += z_y;
    result_z += z_z;
    result_x += matrix[12];
    result_y += matrix[13];
    result_z += matrix[14];
    output[0] = static_cast<float>(result_x);
    output[1] = static_cast<float>(result_y);
    output[2] = static_cast<float>(result_z);
}

// Each component is stored before the next input loads.
void cross(const float *left, const float *right, float *output) {
    const auto right_z_left_y = static_cast<long double>(right[2]) * left[1];
    const auto left_z_right_y = static_cast<long double>(left[2]) * right[1];
    output[0] = static_cast<float>(right_z_left_y - left_z_right_y);
    const auto left_z_right_x = static_cast<long double>(left[2]) * right[0];
    const auto left_x_right_z = static_cast<long double>(left[0]) * right[2];
    output[1] = static_cast<float>(left_z_right_x - left_x_right_z);
    const auto left_x_right_y = static_cast<long double>(left[0]) * right[1];
    const auto right_x_left_y = static_cast<long double>(right[0]) * left[1];
    output[2] = static_cast<float>(left_x_right_y - right_x_left_y);
    output[3] = static_cast<float>(static_cast<long double>(left[3]) * right[3]);
}
} // namespace nfs::graphics::math
