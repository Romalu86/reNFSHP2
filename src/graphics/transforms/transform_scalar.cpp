#include "nfs/graphics/transforms/transform_handlers.hpp"
#include <cfenv>
namespace nfs::game::runtime::matrices::handlers {
namespace {
float multiply(float a, float b) {
    volatile float result = float(static_cast<long double>(a) * b);
    return result;
}
float add(float a, float b) {
    volatile float result = float(static_cast<long double>(a) + b);
    return result;
}
void affine_points(int count, const void *input, void *output, const float *position, bool indexed,
                   bool project) {

    // stores express its normal finite arithmetic. Raw x87 word/status, extended
    // exponent underflow/overflow and trap behavior remain unaccepted.
    std::fenv_t environment;
    std::feholdexcept(&environment);
    const auto *indices = static_cast<const Dword *>(matrices::pending::transform_vertex_buffer);
    auto *source = static_cast<const Byte *>(input);
    auto *destination = static_cast<Byte *>(output);
    do {
        Dword index = indexed ? *indices++ : 0;
        const float *in = reinterpret_cast<const float *>(
            indexed ? static_cast<const Byte *>(input) + index * input_stride : source);
        float *out = reinterpret_cast<float *>(
            indexed ? static_cast<Byte *>(output) + index * output_stride : destination);
        float x = in[0], y = in[1], z = in[2];
        float value[3];
        for (unsigned axis = 0; axis < 3; ++axis)
            value[axis] = add(add(add(multiply(x, matrix[axis]), multiply(y, matrix[3 + axis])),
                                  multiply(z, matrix[6 + axis])),
                              position[axis]);
        if (project) {
            const float reciprocal = float(static_cast<long double>(reciprocal_scale) / value[2]);
            const float depth = multiply(add(value[2], -near_plane), depth_scale);
            const float px = add(multiply(multiply(value[0], screen_x), reciprocal), offset_x);
            const float py = add(offset_y, -multiply(multiply(value[1], screen_y), reciprocal));
            out[3] = reciprocal;
            out[2] = depth;
            out[1] = py;
            out[0] = px;
        } else {
            out[2] = value[2];
            out[0] = value[0];
            out[1] = value[1];
        }
        source += input_stride;
        destination += output_stride;
    } while (--count);
    std::fesetenv(&environment);
    std::feclearexcept(FE_ALL_EXCEPT);
}
} // namespace
// Semantic C++ arguments replace the internal register ABI.
void scalar_affine(int n, const void *i, void *o, const float *p) {
    affine_points(n, i, o, p, false, false);
}

void scalar_affine_project(int n, const void *i, void *o, const float *p) {
    affine_points(n, i, o, p, false, true);
}

void scalar_affine_indexed(int n, const void *i, void *o, const float *p) {
    affine_points(n, i, o, p, true, false);
}

void scalar_affine_project_indexed(int n, const void *i, void *o, const float *p) {
    affine_points(n, i, o, p, true, true);
}
} // namespace nfs::game::runtime::matrices::handlers
