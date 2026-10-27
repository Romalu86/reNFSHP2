#include "d3dx_matrix_dispatch.hpp"
#include "../../math/precision/extended_float.hpp"
#include <cstring>
namespace nfs::graphics::d3dx {
namespace {
const Dword reciprocal_sqrt_coefficients[1024] = {
#include "d3dx_reciprocal_sqrt.inc"
};
float floating_word(Dword word) {
    float value;
    std::memcpy(&value, &word, 4);
    return value;
}
Dword float_word(float value) {
    Dword word;
    std::memcpy(&word, &value, 4);
    return word;
}
} // namespace

float *__stdcall transform_point2(float *output, const float *point, const float *matrix) {
    auto format = kernel::numeric::current_format();
    auto X = [&](float value) { return kernel::numeric::Extended(value, format); };
    kernel::numeric::Extended result[4] = {X(0), X(0), X(0), X(0)};
    for (unsigned column = 0; column < 4; ++column)
        result[column] = (X(point[0]) * matrix[column] + X(point[1]) * matrix[4 + column]) +
                         matrix[12 + column];
    for (unsigned column = 0; column < 4; ++column)
        output[column] = float(result[column]);
    return output;
}

float *__stdcall transform_point3(float *output, const float *point, const float *matrix) {
    auto format = kernel::numeric::current_format();
    auto X = [&](float value) { return kernel::numeric::Extended(value, format); };
    kernel::numeric::Extended result[4] = {X(0), X(0), X(0), X(0)};
    for (unsigned column = 0; column < 4; ++column)
        result[column] = ((X(point[0]) * matrix[column] + X(point[1]) * matrix[4 + column]) +
                          X(point[2]) * matrix[8 + column]) + matrix[12 + column];
    for (unsigned column = 0; column < 4; ++column)
        output[column] = float(result[column]);
    return output;
}

float *__stdcall transform_vector4(float *output, const float *point, const float *matrix) {
    auto format = kernel::numeric::current_format();
    auto X = [&](float value) { return kernel::numeric::Extended(value, format); };
    kernel::numeric::Extended result[4] = {X(0), X(0), X(0), X(0)};
    for (unsigned column = 0; column < 4; ++column)
        result[column] = ((X(point[0]) * matrix[column] + X(point[1]) * matrix[4 + column]) +
                          X(point[2]) * matrix[8 + column]) + X(point[3]) * matrix[12 + column];
    for (unsigned column = 0; column < 4; ++column)
        output[column] = float(result[column]);
    return output;
}

float *__stdcall transform_normal2(float *output, const float *point, const float *matrix) {
    auto format = kernel::numeric::current_format();
    auto X = [&](float value) { return kernel::numeric::Extended(value, format); };
    auto x = X(point[0]) * matrix[0] + X(point[1]) * matrix[4];
    auto y = X(point[0]) * matrix[1] + X(point[1]) * matrix[5];
    output[0] = float(x);
    output[1] = float(y);
    return output;
}

float *__stdcall transform_normal3(float *output, const float *point, const float *matrix) {
    auto format = kernel::numeric::current_format();
    auto X = [&](float value) { return kernel::numeric::Extended(value, format); };
    kernel::numeric::Extended result[3] = {X(0), X(0), X(0)};
    for (unsigned column = 0; column < 3; ++column)
        result[column] = (X(point[0]) * matrix[column] + X(point[1]) * matrix[4 + column]) +
                         X(point[2]) * matrix[8 + column];
    for (unsigned column = 0; column < 3; ++column)
        output[column] = float(result[column]);
    return output;
}
// Table approximation, not a replacement square root.
float *__stdcall normalize_vector3(float *output, const float *vector) {
    auto format = kernel::numeric::current_format();
    auto X = [&](float value) { return kernel::numeric::Extended(value, format); };
    auto squared = X(vector[0]) * vector[0] + X(vector[1]) * vector[1] + X(vector[2]) * vector[2];
    float stored_square = float(squared);
    if (float_word(stored_square) == 0) {
        volatile float *stores = output;
        stores[0] = 0;
        stores[1] = 0;
        stores[2] = 0;
    } else if ((float_word(float(squared - X(1))) & 2147483647u) < 925353389) {
        if (output != vector)
            for (unsigned axis = 0; axis < 3; ++axis)
                output[axis] = vector[axis];
    } else {
        Dword word = float_word(stored_square), index = (word >> 12) & 4088;
        float mantissa = floating_word((word & 16777215u) | 1056964608u);
        float exponent = floating_word(((3204448255u - word) >> 1) & 4286578688u);
        auto factor = (X(mantissa) * floating_word(reciprocal_sqrt_coefficients[index / 4]) +
                       floating_word(reciprocal_sqrt_coefficients[index / 4 + 1])) *
                      exponent;
        float x = vector[0], y = vector[1], z = vector[2];
        output[0] = float(X(x) * factor);
        output[1] = float(X(y) * factor);
        output[2] = float(X(z) * factor);
    }
    return output;
}

void __stdcall multiply_transposed(float *output, const float *left, const float *right) {
    float scratch[16];
    float *destination = output == left || output == right ? scratch : output;
    auto format = kernel::numeric::current_format();
    auto X = [&](float value) { return kernel::numeric::Extended(value, format); };
    for (unsigned row = 0; row < 4; ++row) {
        float source[4] = {left[row * 4], left[row * 4 + 1], left[row * 4 + 2], left[row * 4 + 3]};
        for (unsigned column = 0; column < 4; ++column)
            destination[column * 4 + row] =
                float((X(source[2]) * right[8 + column] + X(source[0]) * right[column]) +
                      (X(source[1]) * right[4 + column] + X(source[3]) * right[12 + column]));
    }
    if (destination != output)
        for (unsigned index = 0; index < 16; ++index)
            output[index] = destination[index];
}

void pending::install_baseline(Dword *table) {
    table[0] = reinterpret_cast<Dword>(transform_point2);
    table[1] = reinterpret_cast<Dword>(transform_point3);
    table[2] = reinterpret_cast<Dword>(transform_vector4);
    table[3] = reinterpret_cast<Dword>(multiply_baseline);
    table[4] = reinterpret_cast<Dword>(multiply_transposed);
    table[5] = reinterpret_cast<Dword>(transform_normal2);
    table[6] = reinterpret_cast<Dword>(transform_normal3);
    table[7] = reinterpret_cast<Dword>(normalize_vector3);
}
} // namespace nfs::graphics::d3dx
