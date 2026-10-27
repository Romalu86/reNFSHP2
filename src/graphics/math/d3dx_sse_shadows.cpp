#include "d3dx_dispatch_kernels.hpp"
#include <cstdint>
#include <xmmintrin.h>
namespace nfs::graphics::d3dx {
namespace shadow_constants {
std::uint8_t initialized;
alignas(16) float half[4];
alignas(16) float three[4];
} // namespace shadow_constants
namespace {
void shadow_vectors(float *out, const float *light, const float *plane) {
    alignas(16) const Dword xyz_mask[4] = {4294967295u, 4294967295u, 4294967295u, 0};
    alignas(16) const Dword y_mask[4] = {0, 4294967295u, 0, 0};
    alignas(16) const Dword z_mask[4] = {0, 0, 4294967295u, 0};
    alignas(16) const Dword w_mask[4] = {0, 0, 0, 4294967295u};
    auto input = _mm_loadu_ps(plane);
    auto xyz = _mm_and_ps(input, _mm_load_ps(reinterpret_cast<const float *>(xyz_mask)));
    auto square = _mm_mul_ps(xyz, xyz);
    auto high = _mm_movehl_ps(_mm_setzero_ps(), square);
    auto sum = _mm_add_ps(square, high);
    auto normal = _mm_add_ss(_mm_shuffle_ps(sum, sum, 1), sum);
    if (!_mm_comieq_ss(normal, _mm_setzero_ps())) {
        __m128 half, three;
        if (!(shadow_constants::initialized & 1)) {
            shadow_constants::initialized |= 1;
            half = _mm_set_ss(.5f);
            _mm_store_ps(shadow_constants::half, half);
        } else
            half = _mm_load_ps(shadow_constants::half);
        if (!(shadow_constants::initialized & 2)) {
            shadow_constants::initialized |= 2;
            three = _mm_set_ss(3);
            _mm_store_ps(shadow_constants::three, three);
        } else
            three = _mm_load_ps(shadow_constants::three);
        auto estimate = _mm_rsqrt_ss(normal);
        auto correction = _mm_sub_ss(three, _mm_mul_ss(_mm_mul_ss(normal, estimate), estimate));
        auto inverse = _mm_mul_ss(_mm_mul_ss(half, estimate), correction);
        normal = _mm_mul_ps(input, _mm_shuffle_ps(inverse, inverse, 0));
    }
    auto source = _mm_loadu_ps(light);
    auto negative = _mm_xor_ps(_mm_set1_ps(-0.0f), normal);
    auto product = _mm_mul_ps(normal, source);
    auto dot = _mm_add_ps(product, _mm_movehl_ps(product, product));
    dot = _mm_add_ss(dot, _mm_shuffle_ps(dot, dot, 1));
    dot = _mm_shuffle_ps(dot, dot, 0);
    auto first = _mm_mul_ps(_mm_shuffle_ps(negative, negative, 0), source);
    auto second = _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(negative, negative, 85), source),
                             _mm_and_ps(dot, _mm_load_ps(reinterpret_cast<const float *>(y_mask))));
    auto third = _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(negative, negative, 170), source),
                            _mm_and_ps(dot, _mm_load_ps(reinterpret_cast<const float *>(z_mask))));
    auto fourth = _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(negative, negative, 255), source),
                             _mm_and_ps(dot, _mm_load_ps(reinterpret_cast<const float *>(w_mask))));
    _mm_storeu_ps(out, _mm_add_ss(first, dot));
    _mm_storeu_ps(out + 4, second);
    _mm_storeu_ps(out + 8, third);
    _mm_storeu_ps(out + 12, fourth);
}
} // namespace

void __stdcall shadow_matrix_sse(float *out, const float *light, const float *plane) {
    shadow_vectors(out, light, plane);
}

void __stdcall shadow_matrix_sse2(float *out, const float *light, const float *plane) {
    shadow_vectors(out, light, plane);
}
} // namespace nfs::graphics::d3dx
