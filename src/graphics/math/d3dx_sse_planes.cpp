#include "d3dx_dispatch_kernels.hpp"
#include <xmmintrin.h>
#include <cstring>
namespace nfs::graphics::d3dx {
namespace {
void reflect(float *output, const float *plane) {
    auto input = _mm_loadu_ps(plane);
    auto mask = _mm_cmpeq_ps(_mm_setzero_ps(), _mm_setzero_ps());
    float mask_words[4];
    _mm_storeu_ps(mask_words, mask);
    Dword zero = 0;
    std::memcpy(mask_words + 3, &zero, 4);
    auto measured = _mm_and_ps(input, _mm_loadu_ps(mask_words));
    auto square = _mm_mul_ps(measured, measured);
    auto pairs = _mm_add_ps(square, _mm_movehl_ps(square, square));
    auto sum = _mm_add_ss(pairs, _mm_shuffle_ps(pairs, pairs, 1));
    auto reciprocal = _mm_rsqrt_ss(sum);
    auto product = _mm_mul_ss(_mm_mul_ss(sum, reciprocal), reciprocal);
    auto factor =
        _mm_mul_ss(_mm_mul_ss(_mm_set_ss(.5f), reciprocal), _mm_sub_ss(_mm_set_ss(3), product));
    auto normalized = _mm_mul_ps(input, _mm_shuffle_ps(factor, factor, 0));
    auto scaled = _mm_mul_ps(_mm_setr_ps(-2, -2, -2, 0), normalized);
    auto row0 = _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(normalized, normalized, 0), scaled),
                           _mm_setr_ps(1, 0, 0, 0));
    auto row3 = _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(normalized, normalized, 255), scaled),
                           _mm_setr_ps(0, 0, 0, 1));
    auto row2 = _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(normalized, normalized, 170), scaled),
                           _mm_setr_ps(0, 0, 1, 0));
    auto row1 = _mm_add_ps(_mm_mul_ps(scaled, _mm_shuffle_ps(normalized, normalized, 85)),
                           _mm_setr_ps(0, 1, 0, 0));
    _mm_storeu_ps(output, row0);
    _mm_storeu_ps(output + 4, row1);
    _mm_storeu_ps(output + 8, row2);
    _mm_storeu_ps(output + 12, row3);
}
} // namespace

void __stdcall reflection_matrix_sse(float *output, const float *plane) {
    reflect(output, plane);
}

void __stdcall reflection_matrix_sse2(float *output, const float *plane) {
    reflect(output, plane);
}

float *__stdcall inverse_quaternion_sse(float *output, const float *input) {
    auto value = _mm_loadu_ps(input), square = _mm_mul_ps(value, value);
    auto pairs = _mm_add_ps(square, _mm_movehl_ps(square, square));
    auto sum = _mm_add_ss(pairs, _mm_shuffle_ps(pairs, pairs, 85));
    auto conjugate = _mm_mul_ps(value, _mm_setr_ps(-1, -1, -1, 1));
    auto reciprocal = _mm_rcp_ss(sum);
    auto product = _mm_mul_ss(_mm_mul_ss(reciprocal, sum), reciprocal);
    auto refined = _mm_sub_ss(_mm_add_ss(reciprocal, reciprocal), product);
    _mm_storeu_ps(output, _mm_mul_ps(conjugate, _mm_shuffle_ps(refined, refined, 0)));
    return output;
}
} // namespace nfs::graphics::d3dx
