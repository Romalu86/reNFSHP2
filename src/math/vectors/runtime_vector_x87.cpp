#include "nfs/world/resources/runtime_resources.hpp"
#include "frame_x87_private.hpp"
#include "retained_vector_providers.hpp"
#include <cstring>
static_assert(__LDBL_MANT_DIG__ == 64, "This translation unit requires native x87 intermediates");
namespace nfs::game::runtime {

namespace hardware {
void store_dot_product(const float *first, const float *second, void *output) {
    const auto value = dot_product(first, second);
    std::memcpy(output, &value, 10);
}

void store_vector_length(const float *components, void *output) {
    const auto value = vector_length(components);
    std::memcpy(output, &value, 10);
}

void store_point_distance(const float *first, const float *second, void *output) {
    float delta[3];
    for (unsigned axis = 0; axis < 3; ++axis)
        delta[axis] = float(static_cast<long double>(second[axis]) - first[axis]);
    const auto value = vector_length(delta);
    std::memcpy(output, &value, 10);
}

void store_distance_squared(const float *first, const float *second, void *output) {
    const auto value = distance_squared(first, second);
    std::memcpy(output, &value, 10);
}
} // namespace hardware

void scale_vectors(int count, const float *input, float scale, float *output) {
    if (count <= 0)
        return;
    const auto retained_scale = static_cast<long double>(scale);
    do {
        output[0] = float(retained_scale * input[0]);
        output[1] = float(retained_scale * input[1]);
        output[2] = float(retained_scale * input[2]);
        input += 3;
        output += 3;
    } while (--count);
}

void normalize_vector(const float *input, float *output) {
    auto X = [](float component) { return static_cast<long double>(component); };
    const auto y = X(input[1]), x = X(input[0]), z = X(input[2]);
    const auto xx = x * x;
    const auto yy = y * y;
    const auto xy_sum = xx + yy;
    const auto zz = z * z;

    const auto inverse = X(1) / __builtin_sqrtl(xy_sum + zz);
    output[0] = float(inverse * input[0]);
    output[1] = float(inverse * input[1]);
    output[2] = float(inverse * input[2]);
}
} // namespace nfs::game::runtime
