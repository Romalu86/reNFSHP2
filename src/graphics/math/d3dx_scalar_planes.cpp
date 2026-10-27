#include "d3dx_dispatch_kernels.hpp"
#include "../../math/precision/extended_float.hpp"
#include <cmath>
namespace nfs::graphics::d3dx {
using kernel::numeric::Extended;

void __stdcall plane_from_points_scalar(float *out, const float *first, const float *second,
                                        const float *third) {
    auto format = kernel::numeric::current_format();
    auto X = [&](float f) { return Extended(f, format); };
    auto a0 = X(first[0]) - second[0], a1 = X(first[1]) - second[1], a2 = X(first[2]) - second[2];
    float b0 = float(X(first[0]) - third[0]), b1 = float(X(first[1]) - third[1]);
    auto b2 = X(first[2]) - third[2];
    float result[3] = {float(b2 * a1 - X(b1) * a2), float(X(b0) * a2 - b2 * a0),
                       float(X(b1) * a0 - X(b0) * a1)};
    out[0] = result[0];
    out[1] = result[1];
    out[2] = result[2];
    normalize_vector3_bootstrap(out, out);
    out[3] = float(-(X(out[2]) * first[2] + X(out[1]) * first[1] + X(out[0]) * first[0]));
}

void __stdcall reflection_matrix_scalar(float *out, const float *plane) {
    float normalized[4];
    normalize_plane_bootstrap(normalized, plane);
    auto format = kernel::numeric::current_format();
    auto X = [&](float f) { return Extended(f, format); };
    auto x = X(normalized[0]) * -2.0f, y = X(normalized[1]) * -2.0f, z = X(normalized[2]) * -2.0f;
    float stored_z = float(z);
    out[0] = float(X(normalized[0]) * x + 1.0f);
    out[1] = float(y * normalized[0]);
    out[2] = float(z * normalized[0]);
    out[3] = 0;
    out[4] = float(X(normalized[1]) * x);
    out[5] = float(X(normalized[1]) * y + 1.0f);
    out[6] = float(z * normalized[1]);
    out[7] = 0;
    out[8] = float(X(normalized[2]) * x);
    out[9] = float(X(normalized[2]) * y);
    out[10] = float(X(normalized[2]) * z + 1.0f);
    out[11] = 0;
    out[12] = float(X(normalized[3]) * x);
    out[13] = float(X(normalized[3]) * y);
    out[14] = float(X(normalized[3]) * stored_z);
    out[15] = 1;
}

void __stdcall shadow_matrix_scalar(float *out, const float *light, const float *plane) {
    float p[4];
    normalize_plane_bootstrap(p, plane);
    auto format = kernel::numeric::current_format();
    auto X = [&](float f) { return Extended(f, format); };
    auto dot = X(p[1]) * light[1] + X(p[0]) * light[0] + X(p[3]) * light[3] + X(p[2]) * light[2];
    for (unsigned i = 0; i < 4; ++i)
        p[i] = -p[i];
    for (unsigned column = 0; column < 4; ++column)
        for (unsigned row = 0; row < 4; ++row) {
            auto value = X(p[row]) * light[column];
            out[row * 4 + column] = float(row == column ? value + dot : value);
        }
}

float *__stdcall intersect_plane_line_scalar(float *out, const float *plane, const float *first,
                                             const float *second) {
    auto format = kernel::numeric::current_format();
    auto X = [&](float f) { return Extended(f, format); };
    auto first_dot = X(plane[0]) * first[0] + X(plane[1]) * first[1] + X(first[2]) * plane[2];
    auto denominator =
        first_dot - (X(plane[0]) * second[0] + X(second[2]) * plane[2] + X(plane[1]) * second[1]);
    if (std::isnan(float(denominator)) || denominator < X(0) || X(0) < denominator) {
        float fraction = float((first_dot + plane[3]) / denominator);
        if (std::isfinite(double(fraction))) {
            for (unsigned axis = 0; axis < 3; ++axis)
                out[axis] = float((X(second[axis]) - first[axis]) * fraction + first[axis]);
            return out;
        }
    }
    return nullptr;
}
} // namespace nfs::graphics::d3dx
