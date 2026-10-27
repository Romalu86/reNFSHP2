#include "d3dx_dispatch_kernels.hpp"
#include <xmmintrin.h>
namespace nfs::graphics::d3dx {
namespace {
void normalize_three(float *output, const float *input, bool plane) {
    auto x = _mm_set_ss(input[0]), y = _mm_set_ss(input[1]), z = _mm_set_ss(input[2]);
    auto square = _mm_add_ss(_mm_add_ss(_mm_mul_ss(x, x), _mm_mul_ss(y, y)), _mm_mul_ss(z, z));
    float length_square;
    _mm_store_ss(&length_square, square);

    if (length_square < 0.0000000000000142108547152020037174224853515625f) {
        output[0] = 0;
        output[1] = 0;
        output[2] = 0;
        if (plane)
            output[3] = 0;
        return;
    }
    auto reciprocal = _mm_rsqrt_ss(square);
    auto product = _mm_mul_ss(_mm_mul_ss(square, reciprocal), reciprocal);
    auto half_reciprocal = _mm_mul_ss(_mm_set_ss(.5f), reciprocal);
    auto factor = _mm_mul_ss(half_reciprocal, _mm_sub_ss(_mm_set_ss(3), product));
    _mm_store_ss(output, _mm_mul_ss(factor, x));
    _mm_store_ss(output + 1, _mm_mul_ss(factor, y));
    _mm_store_ss(output + 2, _mm_mul_ss(factor, z));
    if (plane)
        _mm_store_ss(output + 3, _mm_mul_ss(factor, _mm_set_ss(input[3])));
}
void normalize_four(float *output, const float *input) {
    auto x = _mm_set_ss(input[0]), y = _mm_set_ss(input[1]), z = _mm_set_ss(input[2]),
         w = _mm_set_ss(input[3]);
    auto square = _mm_add_ss(_mm_add_ss(_mm_mul_ss(x, x), _mm_mul_ss(y, y)),
                             _mm_add_ss(_mm_mul_ss(z, z), _mm_mul_ss(w, w)));
    auto reciprocal = _mm_rsqrt_ss(square);
    auto product = _mm_mul_ss(square, _mm_mul_ss(reciprocal, reciprocal));
    auto factor =
        _mm_mul_ss(_mm_mul_ss(_mm_sub_ss(_mm_set_ss(3), product), _mm_set_ss(.5f)), reciprocal);
    _mm_store_ss(output, _mm_mul_ss(x, factor));
    _mm_store_ss(output + 1, _mm_mul_ss(y, factor));
    _mm_store_ss(output + 2, _mm_mul_ss(z, factor));
    _mm_store_ss(output + 3, _mm_mul_ss(w, factor));
}
} // namespace

void __stdcall normalize_vector3_sse(float *output, const float *input) {
    normalize_three(output, input, false);
}

void __stdcall normalize_vector3_sse2(float *output, const float *input) {
    normalize_three(output, input, false);
}

void __stdcall normalize_plane_sse(float *output, const float *input) {
    normalize_three(output, input, true);
}

void __stdcall normalize_plane_sse2(float *output, const float *input) {
    normalize_three(output, input, true);
}

void __stdcall normalize_vector4_sse(float *output, const float *input) {
    normalize_four(output, input);
}

void __stdcall normalize_quaternion_sse(float *output, const float *input) {
    normalize_four(output, input);
}

float *__stdcall multiply_quaternion_sse(float *output, const float *first, const float *second) {
    auto a = _mm_loadu_ps(first), b = _mm_loadu_ps(second);
    auto result = _mm_mul_ps(_mm_shuffle_ps(b, b, 255), a);
    auto reversed = _mm_shuffle_ps(a, a, 27);
    auto signed_x = _mm_xor_ps(_mm_shuffle_ps(b, b, 0), _mm_setr_ps(0, -0.0f, 0, -0.0f));
    result = _mm_add_ps(result, _mm_mul_ps(signed_x, reversed));
    reversed = _mm_shuffle_ps(reversed, reversed, 177);
    auto signed_y = _mm_xor_ps(_mm_shuffle_ps(b, b, 85), _mm_setr_ps(0, 0, -0.0f, -0.0f));
    result = _mm_add_ps(result, _mm_mul_ps(signed_y, reversed));
    reversed = _mm_shuffle_ps(reversed, reversed, 27);
    auto signed_z = _mm_xor_ps(_mm_shuffle_ps(b, b, 170), _mm_setr_ps(-0.0f, 0, 0, -0.0f));
    result = _mm_add_ps(result, _mm_mul_ps(signed_z, reversed));
    _mm_storeu_ps(output, result);
    return output;
}
} // namespace nfs::graphics::d3dx
