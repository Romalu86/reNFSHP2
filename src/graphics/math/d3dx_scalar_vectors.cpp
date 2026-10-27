#include "d3dx_dispatch_kernels.hpp"
#include "../../math/precision/extended_float.hpp"
#include <cmath>
#include <cstring>
namespace nfs::graphics::d3dx {
using kernel::numeric::Extended;
namespace {
bool near_one(float value) {
    auto format = kernel::numeric::current_format();
    auto difference = Extended(value, format) - 1.0f;
    return !std::isnan(float(difference)) && !(difference < Extended(-0.00000011920928955078125f, format)) &&
           !(Extended(0.00000011920928955078125f, format) < difference);
}
void normalize_components(float *output, const float *input, unsigned length, bool plane) {
    auto format = kernel::numeric::current_format();
    auto X = [&](float f) { return Extended(f, format); };
    unsigned measured = plane ? 3 : length;
    auto sum = X(input[0]) * input[0];
    for (unsigned i = 1; i < measured; ++i)
        sum = sum + X(input[i]) * input[i];
    float square = float(sum);
    if (near_one(square)) {
        if (output != input)
            for (unsigned i = 0; i < length; ++i)
                output[i] = input[i];
        return;
    }
    if (!(0.000000000000000000000000000000000000011754943508222875079687365372222456778186655567720875215087517062784172594547271728515625f < square)) {
        for (unsigned i = 0; i < length; ++i)
            output[i] = 0;
        return;
    }
    auto factor = X(1) / X(square).square_root();
    if (plane) {
        for (unsigned i = 0; i < length; ++i)
            output[i] = float(factor * input[i]);
    } else {
        float temporary[4];
        for (unsigned i = 0; i < length; ++i)
            temporary[i] = float(factor * input[i]);
        for (unsigned i = 0; i < length; ++i)
            output[i] = temporary[i];
    }
}
} // namespace

int approximately_equal(float first, float second) {
    auto format = kernel::numeric::current_format();
    auto difference = Extended(first, format) - second;
    return !std::isnan(float(difference)) && !(difference < Extended(-0.00000011920928955078125f, format)) &&
           !(Extended(0.00000011920928955078125f, format) < difference);
}

void __stdcall transform_point2_scalar(float *output, const float *input, const float *matrix) {
    auto format = kernel::numeric::current_format();
    auto X = [&](float f) { return Extended(f, format); };
    float result[4];
    for (unsigned column = 0; column < 4; ++column)
        result[column] = float(X(matrix[column]) * input[0] + X(matrix[column + 4]) * input[1] +
                               matrix[column + 12]);
    for (unsigned i = 0; i < 4; ++i)
        output[i] = result[i];
}

void __stdcall transform_point3_scalar(float *output, const float *input, const float *matrix) {
    auto format = kernel::numeric::current_format();
    auto X = [&](float f) { return Extended(f, format); };
    float result[4];
    for (unsigned column = 0; column < 4; ++column)
        result[column] = float(X(matrix[column + 8]) * input[2] + X(matrix[column + 4]) * input[1] +
                               X(matrix[column]) * input[0] + matrix[column + 12]);
    for (unsigned i = 0; i < 4; ++i)
        output[i] = result[i];
}

void __stdcall transform_vector4_scalar(float *output, const float *input, const float *matrix) {
    auto format = kernel::numeric::current_format();
    auto X = [&](float f) { return Extended(f, format); };
    float result[4];
    result[0] = float(X(matrix[12]) * input[3] + X(matrix[4]) * input[1] + X(matrix[8]) * input[2] +
                      X(input[0]) * matrix[0]);
    for (unsigned column = 1; column < 4; ++column)
        result[column] =
            float(X(matrix[column + 12]) * input[3] + X(matrix[column + 4]) * input[1] +
                  X(matrix[column]) * input[0] + X(matrix[column + 8]) * input[2]);
    for (unsigned i = 0; i < 4; ++i)
        output[i] = result[i];
}

void __stdcall transform_normal2_scalar(float *output, const float *input, const float *matrix) {
    auto format = kernel::numeric::current_format();
    auto first = Extended(matrix[4], format) * input[1] + Extended(input[0], format) * matrix[0];
    float saved_first = float(first);
    auto second = Extended(matrix[1], format) * input[0] + Extended(matrix[5], format) * input[1];
    output[0] = saved_first;
    output[1] = float(second);
}

void __stdcall transform_normal3_scalar(float *output, const float *input, const float *matrix) {
    auto format = kernel::numeric::current_format();
    auto X = [&](float value) { return Extended(value, format); };
    float result[3];
    for (unsigned column = 0; column < 3; ++column)
        result[column] = float(X(matrix[column + 8]) * input[2] + X(matrix[column + 4]) * input[1] +
                               X(matrix[column]) * input[0]);
    for (unsigned i = 0; i < 3; ++i)
        output[i] = result[i];
}

void __stdcall transform_coordinate2_scalar(float *output, const float *input,
                                            const float *matrix) {
    auto format = kernel::numeric::current_format();
    auto X = [&](float f) { return Extended(f, format); };
    auto x = X(input[0]) * matrix[0] + X(input[1]) * matrix[4] + matrix[12];
    auto y = X(input[0]) * matrix[1] + X(input[1]) * matrix[5] + matrix[13];
    float w = float(X(input[0]) * matrix[3] + X(input[1]) * matrix[7] + matrix[15]);
    output[0] = float(x);
    output[1] = float(y);
    if (!approximately_equal(w, 1)) {
        auto reciprocal = X(1) / X(w);
        output[0] = float(reciprocal * output[0]);
        output[1] = float(reciprocal * output[1]);
    }
}

void __stdcall transform_coordinate3_scalar(float *output, const float *input,
                                            const float *matrix) {
    auto format = kernel::numeric::current_format();
    auto X = [&](float f) { return Extended(f, format); };
    auto x =
        X(input[0]) * matrix[0] + X(input[1]) * matrix[4] + X(input[2]) * matrix[8] + matrix[12];
    auto y =
        X(input[0]) * matrix[1] + X(input[1]) * matrix[5] + X(input[2]) * matrix[9] + matrix[13];
    auto z =
        X(input[0]) * matrix[2] + X(input[1]) * matrix[6] + X(input[2]) * matrix[10] + matrix[14];
    float w = float(X(input[0]) * matrix[3] + X(input[1]) * matrix[7] + X(input[2]) * matrix[11] +
                    matrix[15]);
    output[0] = float(x);
    output[1] = float(y);
    output[2] = float(z);
    if (!approximately_equal(w, 1)) {
        auto reciprocal = X(1) / X(w);
        for (unsigned i = 0; i < 3; ++i)
            output[i] = float(reciprocal * output[i]);
    }
}

void __stdcall normalize_vector3_scalar(float *output, const float *input) {
    normalize_components(output, input, 3, false);
}

void __stdcall normalize_vector4_scalar(float *output, const float *input) {
    normalize_components(output, input, 4, false);
}

void __stdcall normalize_quaternion_scalar(float *output, const float *input) {
    normalize_components(output, input, 4, false);
}

void __stdcall normalize_plane_scalar(float *output, const float *input) {
    normalize_components(output, input, 4, true);
}

void __stdcall normalize_vector2_scalar(float *output, const float *input) {
    normalize_components(output, input, 2, false);
}

void __stdcall plane_from_point_normal_scalar(float *output, const float *point,
                                              const float *normal) {
    auto format = kernel::numeric::current_format();
    output[0] = normal[0];
    output[1] = normal[1];
    output[2] = normal[2];
    output[3] =
        float(-(Extended(point[1], format) * normal[1] + Extended(point[2], format) * normal[2] +
                Extended(point[0], format) * normal[0]));
}
} // namespace nfs::graphics::d3dx
