#include "nfs/physics/frame/dynamics_runtime.hpp"
#include <cstring>
#include <intrin.h>
static_assert(__LDBL_MANT_DIG__ == 64, "This translation unit requires native x87 intermediates");
namespace nfs::game::runtime::physics {
namespace {
long double E(float value) {
    return static_cast<long double>(value);
}
} // namespace
// Preserve sequential reads/stores when the triples overlap.
void subtract_triplets(const float *first, const float *second, float *output) {
    for (unsigned axis = 0; axis < 3; ++axis)
        output[axis] = float(E(first[axis]) - second[axis]);
}
// Read Z before writing Y, as in the saved copy constructor.
float *copy_triplet(float *destination, const float *source) {
    std::memcpy(destination, source, 4);
    Dword y, z;
    std::memcpy(&y, source + 1, 4);
    std::memcpy(&z, source + 2, 4);
    std::memcpy(destination + 1, &y, 4);
    std::memcpy(destination + 2, &z, 4);
    return destination;
}

float *construct_triplet(float *destination, float x, float y, float z) {
    __movsd(reinterpret_cast<unsigned long *>(destination),
            reinterpret_cast<const unsigned long *>(&x), 1);
    __movsd(reinterpret_cast<unsigned long *>(destination + 1),
            reinterpret_cast<const unsigned long *>(&y), 1);
    __movsd(reinterpret_cast<unsigned long *>(destination + 2),
            reinterpret_cast<const unsigned long *>(&z), 1);
    return destination;
}

float *zero_triplet(float *destination) {
    const Dword zero = 0;
    std::memcpy(destination, &zero, 4);
    std::memcpy(destination + 1, &zero, 4);
    std::memcpy(destination + 2, &zero, 4);
    return destination;
}

float *accumulate_triplet(float *destination, const float *source) {
    for (unsigned i = 0; i < 3; ++i)
        destination[i] = float(E(source[i]) + destination[i]);
    return const_cast<float *>(source);
}

float *sum_triplets(float *destination, const float *first, const float *second) {
    for (unsigned i = 0; i < 3; ++i)
        destination[i] = float(E(first[i]) + second[i]);
    return destination;
}

float *scale_triplet(float *destination, const float *source, float scale) {
    for (unsigned i = 0; i < 3; ++i)
        destination[i] = float(E(scale) * source[i]);
    return destination;
}
// Reciprocal is retained across all three products.
float *divide_triplet(float *destination, const float *source, float divisor) {
    const auto reciprocal = E(1.f) / E(divisor);
    for (unsigned i = 0; i < 3; ++i)
        destination[i] = float(reciprocal * source[i]);
    return destination;
}
// The three saved lanes have distinct intermediate stores.
float *weighted_history(const float *history, float *destination) {
    const auto third_x = E(history[6]) + history[6];
    const auto third_y = E(history[7]) + history[7];
    const float third_z = float(E(history[8]) + history[8]);
    const auto previous_x = E(history[3]) + history[3];
    const auto previous_y = E(history[4]) + history[4];
    const float previous_z = float(E(history[5]) + history[5]);
    const float first_x = float(previous_x + history[0]);
    const auto first_y = previous_y + history[1];
    const float first_z = float(E(previous_z) + history[2]);
    const float second_x = float(E(first_x) + third_x);
    const auto second_y = first_y + third_y;
    const auto second_z = E(first_z) + third_z;
    const float sum_x = float(E(second_x) + history[9]);
    const float sum_y = float(second_y + history[10]);
    const auto sum_z = second_z + history[11];
    destination[0] = float(E(sum_x) * 0.16666667163372039794921875f);
    destination[1] = float(E(sum_y) * 0.16666667163372039794921875f);
    destination[2] = float(sum_z * 0.16666667163372039794921875f);
    return destination;
}
} // namespace nfs::game::runtime::physics
