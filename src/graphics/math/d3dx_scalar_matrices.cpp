#include "d3dx_dispatch_kernels.hpp"
#include "../../math/precision/extended_float.hpp"
#include <cmath>
#include <cstring>
#include <utility>
#include <initializer_list>
namespace nfs::graphics::d3dx {
using kernel::numeric::Extended;
namespace {
void multiply_scalar_matrix(float *output, const float *left, const float *right, bool transposed) {
    // Accumulation orders, independently derived from the two scalar bodies.
    static constexpr unsigned char multiply_order[16][4] = {
        {3, 0, 2, 1}, {1, 3, 2, 0}, {3, 0, 2, 1}, {3, 2, 0, 1}, {3, 0, 1, 2}, {2, 3, 1, 0},
        {1, 3, 0, 2}, {1, 3, 2, 0}, {3, 0, 1, 2}, {3, 2, 1, 0}, {1, 0, 3, 2}, {3, 2, 1, 0},
        {3, 0, 1, 2}, {3, 2, 1, 0}, {1, 0, 3, 2}, {3, 2, 1, 0}};
    static constexpr unsigned char transpose_order[16][4] = {
        {3, 0, 2, 1}, {0, 3, 2, 1}, {2, 1, 3, 0}, {1, 3, 0, 2}, {2, 0, 3, 1}, {3, 2, 1, 0},
        {1, 3, 2, 0}, {1, 3, 2, 0}, {2, 0, 1, 3}, {3, 2, 1, 0}, {1, 3, 2, 0}, {1, 3, 2, 0},
        {2, 0, 1, 3}, {3, 2, 1, 0}, {1, 3, 2, 0}, {1, 3, 2, 0}};
    float temporary[16];
    float *destination = output == left || output == right ? temporary : output;
    auto format = kernel::numeric::current_format();
    for (unsigned index = 0; index < 16; ++index) {
        unsigned row = transposed ? index % 4 : index / 4;
        unsigned column = transposed ? index / 4 : index % 4;
        auto order = transposed ? transpose_order[index] : multiply_order[index];
        auto sum = Extended(left[row * 4 + order[0]], format) * right[order[0] * 4 + column];
        for (unsigned i = 1; i < 4; ++i)
            sum = sum + Extended(left[row * 4 + order[i]], format) * right[order[i] * 4 + column];
        destination[index] = float(sum);
    }
    if (destination != output)
        for (unsigned i = 0; i < 16; ++i)
            output[i] = destination[i];
}
} // namespace

void __stdcall multiply_scalar(float *output, const float *left, const float *right) {
    multiply_scalar_matrix(output, left, right, false);
}

void __stdcall multiply_transposed_scalar(float *output, const float *left, const float *right) {
    multiply_scalar_matrix(output, left, right, true);
}
 
float __stdcall matrix_determinant_scalar(const float *m) {
    auto format = kernel::numeric::current_format();
    auto X = [&](float f) { return Extended(f, format); };
    auto a = X(m[10]) * m[15] - X(m[14]) * m[11];
    auto b = X(m[15]) * m[9] - X(m[13]) * m[11];
    auto c = X(m[14]) * m[9] - X(m[10]) * m[13];
    auto d = X(m[8]) * m[15] - X(m[12]) * m[11];
    auto e = X(m[14]) * m[8] - X(m[12]) * m[10];
    float f = float(X(m[8]) * m[13] - X(m[12]) * m[9]);
    return float((a * m[5] - b * m[6] + c * m[7]) * m[0] - (a * m[4] - d * m[6] + e * m[7]) * m[1] +
                 (b * m[4] - d * m[5] + X(f) * m[7]) * m[2] -
                 (c * m[4] - e * m[5] + X(f) * m[6]) * m[3]);
}

void __stdcall transpose_scalar(float *output, const float *input) {
    for (auto pair :
         {std::pair<unsigned, unsigned>{1, 4}, {2, 8}, {3, 12}, {6, 9}, {7, 13}, {11, 14}}) {
        Dword word;
        std::memcpy(&word, input + pair.first, 4);
        std::memcpy(output + pair.first, input + pair.second, 4);
        std::memcpy(output + pair.second, &word, 4);
    }
    if (output != input)
        for (unsigned i : {0u, 5u, 10u, 15u})
            std::memcpy(output + i, input + i, 4);
}

void __stdcall translation_matrix_scalar(float *output, float x, float y, float z) {
    output[11] = 0;
    output[12] = x;
    output[9] = 0;
    output[13] = y;
    output[8] = 0;
    output[14] = z;
    output[7] = 0;
    output[6] = 0;
    output[4] = 0;
    output[3] = 0;
    output[2] = 0;
    output[1] = 0;
    output[15] = 1;
    output[10] = 1;
    output[5] = 1;
    output[0] = 1;
}

void __stdcall scaling_matrix_scalar(float *output, float x, float y, float z) {
    output[14] = 0;
    output[0] = x;
    output[13] = 0;
    output[5] = y;
    output[12] = 0;
    output[10] = z;
    output[11] = 0;
    output[9] = 0;
    output[8] = 0;
    output[7] = 0;
    output[6] = 0;
    output[4] = 0;
    output[3] = 0;
    output[2] = 0;
    output[1] = 0;
    output[15] = 1;
}

void __stdcall rotation_x_matrix_scalar(float *output, float angle) {
    double cosine = std::cos(double(angle)), sine = std::sin(double(angle));
    output[0] = 1;
    output[1] = 0;
    output[6] = float(sine);
    output[2] = 0;
    output[3] = 0;
    output[4] = 0;
    output[5] = float(cosine);
    output[7] = 0;
    output[8] = 0;
    output[9] = -float(sine);
    output[10] = float(cosine);
    output[11] = 0;
    output[12] = 0;
    output[13] = 0;
    output[14] = 0;
    output[15] = 1;
}

void __stdcall rotation_y_matrix_scalar(float *output, float angle) {
    double cosine = std::cos(double(angle)), sine = std::sin(double(angle));
    output[0] = float(cosine);
    output[1] = 0;
    output[2] = -float(sine);
    output[3] = 0;
    output[4] = 0;
    output[5] = 1;
    output[6] = 0;
    output[7] = 0;
    output[8] = float(sine);
    output[9] = 0;
    output[10] = float(cosine);
    output[11] = 0;
    output[12] = 0;
    output[13] = 0;
    output[14] = 0;
    output[15] = 1;
}

void __stdcall rotation_z_matrix_scalar(float *output, float angle) {
    double cosine = std::cos(double(angle)), sine = std::sin(double(angle));
    output[0] = float(cosine);
    output[1] = float(sine);
    output[2] = 0;
    output[3] = 0;
    output[4] = -float(sine);
    output[5] = float(cosine);
    output[6] = 0;
    output[7] = 0;
    output[8] = 0;
    output[9] = 0;
    output[10] = 1;
    output[11] = 0;
    output[12] = 0;
    output[13] = 0;
    output[14] = 0;
    output[15] = 1;
}
} // namespace nfs::graphics::d3dx
