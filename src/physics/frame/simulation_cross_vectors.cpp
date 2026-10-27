#include "nfs/physics/frame/simulation_runtime.hpp"
static_assert(__LDBL_MANT_DIG__ == 64, "This translation unit requires native x87 intermediates");
namespace nfs::game::runtime {
// Save operands before the first possibly overlapping write.
const float *cross_vectors(const float *first, const float *second, float *output) {
    auto X = [](float component) { return static_cast<long double>(component); };
    const auto y_z = X(first[1]) * second[2];
    const auto z_x = X(first[2]) * second[0];
    const auto x_y = X(first[0]) * second[1];
    const auto z_y = X(first[2]) * second[1];
    const auto x_z = X(first[0]) * second[2];
    const auto y_x = X(first[1]) * second[0];
    const auto result_x = y_z - z_y;
    const auto result_y = z_x - x_z;
    const auto result_z = x_y - y_x;
    output[0] = float(result_x);
    output[1] = float(result_y);
    output[2] = float(result_z);
    return first + 3;
}

const float *cross_vectors_entry(const float *first, const float *second, float *output) {
    return cross_vectors(first, second, output);
}
} // namespace nfs::game::runtime
