#include "nfs/graphics/math/eagl_math.hpp"
#include "nfs/core/memory/memory_primitives.hpp"
#include <cstring>
#include <intrin.h>

namespace nfs::graphics::math {
Matrix multiply_scratch{};

// Only right == output is copied to the shared scratch.
void multiply(const float *left, const float *right, float *output) {
    if (right == output) {
        __movsd(reinterpret_cast<unsigned long *>(multiply_scratch),
                reinterpret_cast<const unsigned long *>(right), 16);
        right = multiply_scratch;
    }
    for (unsigned row = 0; row != 4; ++row)
        transform_row(left + row * 4, right, output + row * 4);
}


void append(float *matrix, const float *right) {
    Matrix result;
    multiply(matrix, right, result);
    kernel::memory_copy(matrix, result, sizeof result);
}

// Distinct entry with the same matrix-combine body.
void append_projection(float *matrix, const float *right) {
    Matrix result;
    multiply(matrix, right, result);
    kernel::memory_copy(matrix, result, sizeof result);
}


void diagonal(float *matrix, float x, float y, float z, float w) {
    using kernel::Dword;
    Dword horizontal, vertical, depth, homogeneous;
    std::memcpy(&horizontal, &x, sizeof(horizontal));
    std::memcpy(&vertical, &y, sizeof(vertical));
    auto *output = reinterpret_cast<volatile Dword *>(matrix);
    output[0] = horizontal;
    output[1] = 0;
    output[2] = 0;
    output[3] = 0;
    output[4] = 0;
    output[5] = vertical;
    std::memcpy(&depth, &z, sizeof(depth));
    output[6] = 0;
    output[7] = 0;
    output[8] = 0;
    output[9] = 0;
    output[11] = 0;
    output[12] = 0;
    output[13] = 0;
    output[14] = 0;
    std::memcpy(&homogeneous, &w, sizeof(homogeneous));
    output[10] = depth;
    output[15] = homogeneous;
}


void translation(float *matrix, float x, float y, float z) {
    using kernel::Dword;
    constexpr Dword identity_bits = 1065353216u;
    auto *output = reinterpret_cast<volatile Dword *>(matrix);
    output[1] = 0;
    output[2] = 0;
    output[3] = 0;
    output[4] = 0;
    output[6] = 0;
    output[7] = 0;
    output[8] = 0;
    output[9] = 0;
    output[11] = 0;
    Dword horizontal, vertical, depth;
    std::memcpy(&horizontal, &x, sizeof(horizontal));
    output[12] = horizontal;
    std::memcpy(&vertical, &y, sizeof(vertical));
    output[13] = vertical;
    std::memcpy(&depth, &z, sizeof(depth));
    output[0] = identity_bits;
    output[5] = identity_bits;
    output[10] = identity_bits;
    output[14] = depth;
    output[15] = identity_bits;
}


void copy(float *output, const float *input) {
    __movsd(reinterpret_cast<unsigned long *>(output),
            reinterpret_cast<const unsigned long *>(input), 16);
}



} // namespace nfs::graphics::math
