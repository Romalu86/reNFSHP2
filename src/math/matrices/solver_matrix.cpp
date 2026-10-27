#include "nfs/physics/solver/solver_runtime.hpp"
#include <cstring>
#include <intrin.h>
static_assert(__LDBL_MANT_DIG__ == 64, "This translation unit requires native x87 intermediates");
namespace nfs::game::runtime::matrices {
// Preserve the read/write order for overlapping matrices.
void transpose(const float *input, float *output) {
    std::memcpy(output, input, 4);
    std::memcpy(output + 4, input + 4, 4);
    std::memcpy(output + 8, input + 8, 4);
    Dword direct;
    std::memcpy(&direct, input + 3, 4);
    float exchanged = input[1];
    std::memcpy(output + 1, &direct, 4);
    output[3] = exchanged;
    exchanged = input[2];
    std::memcpy(&direct, input + 6, 4);
    output[6] = exchanged;
    std::memcpy(output + 2, &direct, 4);
    exchanged = input[5];
    std::memcpy(&direct, input + 7, 4);
    output[7] = exchanged;
    std::memcpy(output + 5, &direct, 4);
}
// Only exact input/output equality triggers the saved scratch copy.
void multiply(const float *first, const float *second, float *output) {
    float scratch[9];
    const float *left = first;
    if (first == output) {
        __movsd(reinterpret_cast<unsigned long *>(scratch),
                reinterpret_cast<const unsigned long *>(first), 9);
        left = scratch;
    }
    if (second == output) {
        if (first != second)
            __movsd(reinterpret_cast<unsigned long *>(scratch),
                    reinterpret_cast<const unsigned long *>(second), 9);
        second = scratch;
    }
    auto X = [](float value) { return static_cast<long double>(value); };
    {
        const auto first_term = X(second[6]) * left[2];
        const auto second_term = X(left[1]) * second[3];
        const auto pair = first_term + second_term;
        const auto third_term = X(second[0]) * left[0];
        output[0] = float(pair + third_term);
    }
    {
        const auto first_term = X(second[1]) * left[0];
        const auto second_term = X(second[7]) * left[2];
        const auto pair = first_term + second_term;
        const auto third_term = X(left[1]) * second[4];
        output[1] = float(pair + third_term);
    }
    {
        const auto first_term = X(second[2]) * left[0];
        const auto second_term = X(second[8]) * left[2];
        const auto pair = first_term + second_term;
        const auto third_term = X(left[1]) * second[5];
        output[2] = float(pair + third_term);
    }
    {
        const auto first_term = X(second[0]) * left[3];
        const auto second_term = X(second[6]) * left[5];
        const auto pair = first_term + second_term;
        const auto third_term = X(second[3]) * left[4];
        output[3] = float(pair + third_term);
    }
    {
        const auto first_term = X(second[7]) * left[5];
        const auto second_term = X(second[4]) * left[4];
        const auto pair = first_term + second_term;
        const auto third_term = X(second[1]) * left[3];
        output[4] = float(pair + third_term);
    }
    {
        const auto first_term = X(second[8]) * left[5];
        const auto second_term = X(second[5]) * left[4];
        const auto pair = first_term + second_term;
        const auto third_term = X(second[2]) * left[3];
        output[5] = float(pair + third_term);
    }
    {
        const auto first_term = X(left[7]) * second[3];
        const auto second_term = X(second[0]) * left[6];
        const auto pair = first_term + second_term;
        const auto third_term = X(second[6]) * left[8];
        output[6] = float(pair + third_term);
    }
    {
        const auto first_term = X(second[1]) * left[6];
        const auto second_term = X(left[8]) * second[7];
        const auto pair = first_term + second_term;
        const auto third_term = X(left[7]) * second[4];
        output[7] = float(pair + third_term);
    }
    {
        const auto first_term = X(second[2]) * left[6];
        const auto second_term = X(left[8]) * second[8];
        const auto pair = first_term + second_term;
        const auto third_term = X(left[7]) * second[5];
        output[8] = float(pair + third_term);
    }
}

void rotate_x(float *output, float angle) {
    float sine, cosine;
    turn_sine_cosine(&angle, &sine, &cosine);
    output[4] = cosine;
    output[1] = output[2] = output[3] = output[6] = 0;
    output[7] = -sine;
    output[0] = 1;
    output[5] = sine;
    output[8] = cosine;
}

void rotate_y(float *output, float angle) {
    float sine, cosine;
    turn_sine_cosine(&angle, &sine, &cosine);
    output[0] = cosine;
    output[1] = output[3] = output[5] = output[7] = 0;
    output[4] = 1;
    output[2] = -sine;
    output[6] = sine;
    output[8] = cosine;
}

void rotate_z(float *output, float angle) {
    float sine, cosine;
    turn_sine_cosine(&angle, &sine, &cosine);
    output[0] = cosine;
    output[1] = sine;
    output[3] = -sine;
    output[2] = 0;
    output[4] = cosine;
    output[5] = output[6] = output[7] = 0;
    output[8] = 1;
}

void rotate(const float *input, float x, float y, float z, float *output) {
    float rx[9], ry[9], rz[9], xy[9], rotation[9];
    rotate_x(rx, x);
    rotate_y(ry, y);
    rotate_z(rz, z);
    multiply(rx, ry, xy);
    multiply(xy, rz, rotation);
    multiply(input, rotation, output);
}
// Exact nine saved coefficients at ...
void orthonormalize(float *matrix) {
    const float identity[9] = {1, 0, 0, 0, 1, 0, 0, 0, 1};
    const float coefficients[9] = {-0.5f,   0.375f,   -0.3125f,  0.2734375f,   -0.24609375f,
                                   0.2255859375f, -0.20947265625f, 0.196380615234375f, -0.1854705810546875f};
    float residual[9], power[9], correction[9], temporary[9];
    transpose(matrix, residual);
    multiply(residual, matrix, residual);
    for (int i = 0; i < 9; ++i) {
        residual[i] -= identity[i];
        power[i] = correction[i] = identity[i];
    }
    for (float coefficient : coefficients) {
        multiply(power, residual, power);
        for (int i = 0; i < 9; ++i)
            temporary[i] = power[i] * coefficient;
        for (int i = 0; i < 9; ++i)
            correction[i] += temporary[i];
    }
    multiply(matrix, correction, matrix);
}
} // namespace nfs::game::runtime::matrices
