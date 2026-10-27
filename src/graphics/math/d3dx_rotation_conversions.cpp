#include "d3dx_dispatch_kernels.hpp"
#include "../../math/precision/extended_float.hpp"
#include <cmath>
namespace nfs::graphics::d3dx {
using kernel::numeric::Extended;

void __stdcall quaternion_axis_angle_scalar(const float *rotation, float *axis, float *angle) {
    auto format = kernel::numeric::current_format();
    auto X = [&](float value) { return Extended(value, format); };
    auto square = X(rotation[0]) * rotation[0] + X(rotation[1]) * rotation[1] +
                  X(rotation[2]) * rotation[2] + X(rotation[3]) * rotation[3];
    if (X(0.000000000000000000000000000000000000011754943508222875079687365372222456778186655567720875215087517062784172594547271728515625f) < square) {
        if (axis) {
            auto root = square.square_root();
            auto inverse = X(1) / root;
            for (unsigned i = 0; i < 3; ++i)
                axis[i] = float(inverse * rotation[i]);
        }
        if (angle) {
            auto value = Extended::from_double(std::acos(double(rotation[3])), format);
            *angle = float(value + value);
        }
    } else {
        if (axis) {
            axis[0] = 1;
            axis[1] = 0;
            axis[2] = 0;
        }
        if (angle)
            *angle = 0;
    }
}

void __stdcall quaternion_from_matrix_scalar(float *out, const float *matrix) {
    auto format = kernel::numeric::current_format();
    auto X = [&](float value) { return Extended(value, format); };
    auto trace = X(matrix[5]) + matrix[0] + matrix[10];
    if (X(0) < trace) {
        auto root = (trace + 1.0f).square_root() * .5f;
        out[3] = float(root);
        auto factor = X(.25f) / root;
        out[0] = float((X(matrix[6]) - matrix[9]) * factor);
        out[1] = float((X(matrix[8]) - matrix[2]) * factor);
        out[2] = float((X(matrix[1]) - matrix[4]) * factor);
    } else {
        auto first_diagonal = X(matrix[0]) - matrix[5] - matrix[10];
        float diagonal[3] = {float(first_diagonal), float(X(matrix[5]) - matrix[0] - matrix[10]),
                             float(X(matrix[10]) - (X(matrix[5]) + matrix[0]))};
        unsigned largest = first_diagonal < X(diagonal[1]) || std::isnan(double(first_diagonal)) ||
                                   std::isnan(diagonal[1])
                               ? 1
                               : 0;
        if (diagonal[largest] < diagonal[2] || std::isnan(diagonal[largest]) ||
            std::isnan(diagonal[2]))
            largest = 2;
        constexpr unsigned next[3] = {1, 2, 0};
        unsigned second = next[largest], third = next[second];
        auto root = (X(diagonal[largest]) + 1.0f).square_root() * .5f;
        out[largest] = float(root);
        auto factor = X(.25f) / root;
        out[second] =
            float((X(matrix[largest + second * 4]) + matrix[second + largest * 4]) * factor);
        out[third] = float((X(matrix[largest + third * 4]) + matrix[largest * 4 + third]) * factor);
        out[3] = float((X(matrix[second * 4 + third]) - matrix[third * 4 + second]) * factor);
    }
}

void __stdcall axis_rotation_matrix_scalar(float *out, const float *axis, float angle) {
    auto format = kernel::numeric::current_format();
    auto X = [&](float value) { return Extended(value, format); };
    float cosine = float(std::cos(double(angle))), sine = float(std::sin(double(angle)));
    float complement = float(X(1) - cosine), normal[3] = {axis[0], axis[1], axis[2]};
    normalize_vector3_bootstrap(normal, normal);
    float xy = float(X(normal[1]) * normal[0] * complement);
    auto yz = X(normal[1]) * normal[2] * complement;
    auto zx = X(normal[2]) * normal[0] * complement;
    auto y_sine = X(normal[1]) * sine;
    float stored_y_sine = float(y_sine);
    out[0] = float(X(normal[0]) * normal[0] * complement + cosine);
    out[1] = float(X(xy) + X(normal[2]) * sine);
    out[2] = float(zx - y_sine);
    out[3] = 0;
    out[4] = float(X(xy) - X(normal[2]) * sine);
    out[5] = float(X(normal[1]) * normal[1] * complement + cosine);
    out[6] = float(X(normal[0]) * sine + yz);
    out[7] = 0;
    out[8] = float(X(stored_y_sine) + zx);
    out[9] = float(yz - X(normal[0]) * sine);
    out[10] = float(X(normal[2]) * normal[2] * complement + cosine);
    out[11] = out[12] = out[13] = out[14] = 0;
    out[15] = 1;
}
namespace {
void quaternion_matrix_diagonal(float *out, const float *matrix) {
    auto format = kernel::numeric::current_format();
    auto X = [&](float value) { return Extended(value, format); };
    auto trace = X(matrix[0]) + matrix[5] + matrix[10];
    // FCOMIP/JB takes this branch for an unordered trace as well.
    if (X(0) < trace || std::isnan(double(trace))) {
        auto root = (trace + 1.0f).square_root() * .5f;
        auto factor = X(.25f) / root;
        out[3] = float(root);
        out[0] = float((X(matrix[6]) - matrix[9]) * factor);
        out[1] = float((X(matrix[8]) - matrix[2]) * factor);
        out[2] = float((X(matrix[1]) - matrix[4]) * factor);
        return;
    }
    float diagonal[3] = {matrix[0], matrix[5], matrix[10]};
    unsigned largest = matrix[0] < matrix[5] ? 1 : 0;
    if (!(matrix[10] <= diagonal[largest]))
        largest = 2;
    constexpr unsigned next[3] = {1, 2, 0};
    unsigned second = next[largest], third = next[second];
    auto square = X(1) + diagonal[largest] - diagonal[second] - diagonal[third];
    auto root = square.square_root() * .5f;
    auto factor = X(.25f) / root;
    out[largest] = float(root);
    out[second] = float((X(matrix[largest * 4 + second]) + matrix[largest + second * 4]) * factor);
    out[third] = float((X(matrix[third * 4 + largest]) + matrix[largest * 4 + third]) * factor);
    out[3] = float((X(matrix[second * 4 + third]) - matrix[second + third * 4]) * factor);
}
} // namespace

void __stdcall quaternion_from_matrix_sse(float *out, const float *matrix) {
    quaternion_matrix_diagonal(out, matrix);
}

void __stdcall quaternion_from_matrix_sse2(float *out, const float *matrix) {
    quaternion_matrix_diagonal(out, matrix);
}
} // namespace nfs::graphics::d3dx
