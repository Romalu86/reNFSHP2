#include "d3dx_dispatch_kernels.hpp"
#include "../../math/precision/extended_float.hpp"
namespace nfs::graphics::d3dx {
using kernel::numeric::Extended;

float *__stdcall affine_matrix_scalar(float *out, float scale, const float *rotation_center,
                                      const float *rotation, const float *translation) {
    auto format = kernel::numeric::current_format();
    auto X = [&](float value) { return Extended(value, format); };
    for (unsigned i = 0; i < 16; ++i)
        out[i] = 0;
    out[0] = out[5] = out[10] = scale;
    out[15] = 1;
    if (rotation) {
        float matrix[16];
        quaternion_matrix_bootstrap(matrix, rotation);
        if (rotation_center)
            for (unsigned i = 0; i < 3; ++i)
                out[12 + i] = float(X(out[12 + i]) - rotation_center[i]);
        multiply_bootstrap(out, out, matrix);
        if (rotation_center)
            for (unsigned i = 0; i < 3; ++i)
                out[12 + i] = float(X(out[12 + i]) + rotation_center[i]);
    }
    if (translation)
        for (unsigned i = 0; i < 3; ++i)
            out[12 + i] = float(X(translation[i]) + out[12 + i]);
    return out;
}

float *__stdcall transformation_matrix_scalar(float *out, const float *scaling_center,
                                              const float *scaling_rotation, const float *scale,
                                              const float *rotation_center, const float *rotation,
                                              const float *translation) {
    auto format = kernel::numeric::current_format();
    auto X = [&](float value) { return Extended(value, format); };
    if (!scale) {
        for (unsigned i = 0; i < 16; ++i)
            out[i] = 0;
        out[0] = out[5] = out[10] = out[15] = 1;
    } else if (!scaling_rotation) {
        for (unsigned i = 0; i < 16; ++i)
            out[i] = 0;
        out[0] = scale[0];
        out[5] = scale[1];
        out[10] = scale[2];
        out[15] = 1;
    } else {
        float scaling[16] = {}, orientation[16], inverse[16];
        scaling[0] = scale[0];
        scaling[5] = scale[1];
        scaling[10] = scale[2];
        scaling[15] = 1;
        quaternion_matrix_bootstrap(orientation, scaling_rotation);
        if (scaling_center) {
            transpose_bootstrap(inverse, orientation);
            for (unsigned i = 0; i < 16; ++i)
                out[i] = 0;
            out[0] = out[5] = out[10] = out[15] = 1;
            for (unsigned i = 0; i < 3; ++i)
                out[12 + i] = -scaling_center[i];
            multiply_bootstrap(out, out, inverse);
            multiply_bootstrap(out, out, scaling);
            multiply_bootstrap(out, out, orientation);
            for (unsigned i = 0; i < 3; ++i)
                out[12 + i] = float(X(out[12 + i]) + scaling_center[i]);
        } else {
            transpose_bootstrap(out, orientation);
            multiply_bootstrap(out, out, scaling);
            multiply_bootstrap(out, out, orientation);
        }
    }
    if (rotation) {
        float orientation[16];
        quaternion_matrix_bootstrap(orientation, rotation);
        if (rotation_center)
            for (unsigned i = 0; i < 3; ++i)
                out[12 + i] = float(X(out[12 + i]) - rotation_center[i]);
        multiply_bootstrap(out, out, orientation);
        if (rotation_center)
            for (unsigned i = 0; i < 3; ++i)
                out[12 + i] = float(X(out[12 + i]) + rotation_center[i]);
    }
    if (translation)
        for (unsigned i = 0; i < 3; ++i)
            out[12 + i] = float(X(out[12 + i]) + translation[i]);
    return out;
}
} // namespace nfs::graphics::d3dx
