#include "d3dx_dispatch_kernels.hpp"
#include <xmmintrin.h>
#include <cstring>
namespace nfs::graphics::d3dx {
namespace {
__m128 curve_weights(float time, bool catmull) {

    const float catmull_cube[4] = {-.5f, 1.5f, -1.5f, .5f};
    const float catmull_constant[4] = {0, 1, 0, 0};
    const float catmull_square[4] = {1, -2.5f, 2, -.5f};
    const float catmull_linear[4] = {-.5f, 0, .5f, 0};
    const float hermite_cube[4] = {2, 1, -2, 1};
    const float hermite_square[4] = {-3, -2, 3, -1};
    const float hermite_linear[4] = {0, 1, 0, 0};
    auto t = _mm_set_ss(time), square = _mm_mul_ss(t, t), cube = _mm_mul_ss(t, square);
    t = _mm_shuffle_ps(t, t, 0);
    square = _mm_shuffle_ps(square, square, 0);
    cube = _mm_shuffle_ps(cube, cube, 0);
    if (catmull) {
        auto weights = _mm_mul_ps(t, _mm_loadu_ps(catmull_linear));
        weights = _mm_add_ps(weights, _mm_loadu_ps(catmull_constant));
        weights = _mm_add_ps(weights, _mm_mul_ps(square, _mm_loadu_ps(catmull_square)));
        return _mm_add_ps(weights, _mm_mul_ps(cube, _mm_loadu_ps(catmull_cube)));
    }
    auto weights = _mm_mul_ps(t, _mm_loadu_ps(hermite_linear));
    weights = _mm_add_ss(weights, _mm_set_ss(1));
    weights = _mm_add_ps(weights, _mm_mul_ps(square, _mm_loadu_ps(hermite_square)));
    return _mm_add_ps(weights, _mm_mul_ps(cube, _mm_loadu_ps(hermite_cube)));
}
void curve(float *output, const float *first, const float *second, const float *third,
           const float *fourth, float time, unsigned dimensions, bool catmull) {
    float weights[4];
    _mm_storeu_ps(weights, curve_weights(time, catmull));
    float inputs[4][4];
    std::memcpy(inputs[0], first, dimensions * 4);
    std::memcpy(inputs[1], second, dimensions * 4);
    std::memcpy(inputs[2], third, dimensions * 4);
    std::memcpy(inputs[3], fourth, dimensions * 4);
    float result[4];
    for (unsigned axis = 0; axis < dimensions; ++axis) {
        auto a = _mm_mul_ss(_mm_set_ss(inputs[0][axis]), _mm_set_ss(weights[0]));
        auto b = _mm_mul_ss(_mm_set_ss(inputs[1][axis]), _mm_set_ss(weights[1]));
        auto c = _mm_mul_ss(_mm_set_ss(inputs[2][axis]), _mm_set_ss(weights[2]));
        auto d = _mm_mul_ss(_mm_set_ss(inputs[3][axis]), _mm_set_ss(weights[3]));
        _mm_store_ss(result + axis, dimensions == 2
                                        ? _mm_add_ss(_mm_add_ss(a, c), _mm_add_ss(b, d))
                                        : _mm_add_ss(_mm_add_ss(_mm_add_ss(a, b), c), d));
    }
    std::memcpy(output, result, dimensions * 4);
}
void barycentric(float *output, const float *first, const float *second, const float *third,
                 float u, float v, unsigned dimensions) {
    float result[4];
    for (unsigned axis = 0; axis < dimensions; ++axis) {
        auto a = _mm_set_ss(first[axis]);
        auto b = _mm_sub_ss(_mm_set_ss(second[axis]), a);
        auto c = _mm_sub_ss(_mm_set_ss(third[axis]), a);
        b = _mm_add_ss(_mm_mul_ss(b, _mm_set_ss(u)), a);
        c = _mm_mul_ss(c, _mm_set_ss(v));
        _mm_store_ss(result + axis, _mm_add_ss(b, c));
    }
    if (dimensions == 3) {
        output[2] = result[2];
        std::memcpy(output, result, 8);
    } else
        std::memcpy(output, result, dimensions * 4);
}
} // namespace

void __stdcall catmull_vector3_sse(float *out, const float *a, const float *b, const float *c,
                                   const float *d, float t) {
    curve(out, a, b, c, d, t, 3, true);
}

void __stdcall catmull_vector2_sse(float *out, const float *a, const float *b, const float *c,
                                   const float *d, float t) {
    curve(out, a, b, c, d, t, 2, true);
}

void __stdcall catmull_vector4_sse(float *out, const float *a, const float *b, const float *c,
                                   const float *d, float t) {
    curve(out, a, b, c, d, t, 4, true);
}

void __stdcall hermite_vector3_sse(float *out, const float *a, const float *b, const float *c,
                                   const float *d, float t) {
    curve(out, a, b, c, d, t, 3, false);
}

void __stdcall hermite_vector4_sse(float *out, const float *a, const float *b, const float *c,
                                   const float *d, float t) {
    curve(out, a, b, c, d, t, 4, false);
}

void __stdcall hermite_vector2_sse(float *out, const float *a, const float *b, const float *c,
                                   const float *d, float t) {
    curve(out, a, b, c, d, t, 2, false);
}

void __stdcall barycentric_vector3_sse(float *out, const float *a, const float *b, const float *c,
                                       float u, float v) {
    barycentric(out, a, b, c, u, v, 3);
}

void __stdcall barycentric_vector4_sse(float *out, const float *a, const float *b, const float *c,
                                       float u, float v) {
    barycentric(out, a, b, c, u, v, 4);
}

void __stdcall barycentric_vector2_sse(float *out, const float *a, const float *b, const float *c,
                                       float u, float v) {
    barycentric(out, a, b, c, u, v, 2);
}
} // namespace nfs::graphics::d3dx
