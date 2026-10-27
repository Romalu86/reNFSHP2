#include "d3dx_dispatch_kernels.hpp"
#include <xmmintrin.h>
namespace nfs::graphics::d3dx {
namespace {
// Both saved SSE/SSE2 pairs have the same arithmetic and shuffle sequence.
// Their separate constant addresses contain identical four-word vectors.
__m128 cyclic(__m128 value) {
    return _mm_shuffle_ps(value, value, 57);
}
__m128 halves(__m128 value) {
    return _mm_shuffle_ps(value, value, 78);
}
__m128 cofactor_products(__m128 row, __m128 other, __m128 last) {
    row = cyclic(row);
    other = cyclic(other);
    auto opposite = halves(last);
    auto paired = _mm_mul_ps(opposite, other);
    auto direct = _mm_mul_ps(last, other);
    auto crossing = _mm_mul_ps(other, cyclic(opposite));
    auto first = _mm_mul_ps(_mm_sub_ps(halves(crossing), crossing), cyclic(row));
    auto second = _mm_mul_ps(row, _mm_sub_ps(cyclic(paired), halves(direct)));
    auto third = _mm_mul_ps(cyclic(cyclic(row)), _mm_sub_ps(paired, cyclic(direct)));
    return _mm_add_ps(_mm_add_ps(second, first), third);
}
float determinant_vectors(const float *matrix) {
    auto products = _mm_mul_ps(cofactor_products(_mm_loadu_ps(matrix + 4), _mm_loadu_ps(matrix + 8),
                                                 _mm_loadu_ps(matrix + 12)),
                               _mm_loadu_ps(matrix));
    auto sums = _mm_add_ps(products, _mm_movehl_ps(products, products));
    return _mm_cvtss_f32(_mm_sub_ss(sums, _mm_shuffle_ps(sums, sums, 1)));
}
void cross_vectors(float *out, const float *first, const float *second, const float *third) {
    auto result = cofactor_products(_mm_loadu_ps(first), _mm_loadu_ps(second), _mm_loadu_ps(third));
    alignas(16) const Dword signs[4] = {0, 2147483648u, 0, 2147483648u};
    _mm_storeu_ps(out, _mm_xor_ps(result, _mm_load_ps(reinterpret_cast<const float *>(signs))));
}
__m128 load_zxy(const float *point) {
    return _mm_loadh_pi(_mm_load_ss(point + 2), reinterpret_cast<const __m64 *>(point));
}
void plane_points(float *out, const float *first, const float *second, const float *third) {
    auto point = load_zxy(first);
    auto a = _mm_sub_ps(point, load_zxy(third)), b = _mm_sub_ps(point, load_zxy(second));
    auto normal = _mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(a, a, 120), _mm_shuffle_ps(b, b, 99)),
                             _mm_mul_ps(_mm_shuffle_ps(b, b, 120), _mm_shuffle_ps(a, a, 99)));
    auto square = _mm_mul_ps(normal, normal);
    auto sum = _mm_add_ps(square, _mm_movehl_ps(square, square));
    sum = _mm_add_ss(sum, _mm_shuffle_ps(sum, sum, 1));
    auto estimate = _mm_rsqrt_ss(sum);
    auto factor =
        _mm_mul_ss(_mm_mul_ss(_mm_set_ss(.5f), estimate),
                   _mm_sub_ss(_mm_set_ss(3), _mm_mul_ss(_mm_mul_ss(sum, estimate), estimate)));
    normal = _mm_mul_ps(normal, _mm_shuffle_ps(factor, factor, 0));
    auto dot = _mm_mul_ps(_mm_shuffle_ps(point, point, 14), normal);
    _mm_storeu_ps(out, normal);
    auto yz = _mm_add_ss(_mm_movehl_ps(dot, dot), _mm_shuffle_ps(dot, dot, 85));
    auto distance = _mm_add_ss(dot, yz);
    _mm_store_ss(out + 3, _mm_xor_ps(distance, _mm_set_ss(-0.0f)));
}
} // namespace

float __stdcall matrix_determinant_sse(const float *matrix) {
    return determinant_vectors(matrix);
}

float __stdcall matrix_determinant_sse2(const float *matrix) {
    return determinant_vectors(matrix);
}

void __stdcall cross_vector4_sse(float *out, const float *first, const float *second,
                                 const float *third) {
    cross_vectors(out, first, second, third);
}

void __stdcall cross_vector4_sse2(float *out, const float *first, const float *second,
                                  const float *third) {
    cross_vectors(out, first, second, third);
}

void __stdcall plane_from_points_sse(float *out, const float *first, const float *second,
                                     const float *third) {
    plane_points(out, first, second, third);
}

void __stdcall plane_from_points_sse2(float *out, const float *first, const float *second,
                                      const float *third) {
    plane_points(out, first, second, third);
}

float *__stdcall intersect_plane_line_sse(float *out, const float *plane, const float *first,
                                          const float *second) {
    auto normal = _mm_loadu_ps(plane);
    auto distance = _mm_shuffle_ps(normal, normal, 255);
    normal = halves(normal);
    auto a = load_zxy(first), b = load_zxy(second);
    auto dot = [&](const __m128 &point) {
        auto product = _mm_mul_ps(point, normal);
        auto sum = _mm_add_ps(product, _mm_movehl_ps(product, product));
        return _mm_add_ss(sum, _mm_shuffle_ps(sum, sum, 85));
    };
    auto first_dot = dot(a), denominator = _mm_sub_ss(first_dot, dot(b));
    auto numerator = _mm_add_ss(first_dot, distance);
    // COMISS sets ZF for both equal and unordered operands.
    if (_mm_comieq_ss(denominator, _mm_setzero_ps()))
        return nullptr;
    auto estimate = _mm_rcp_ss(denominator);
    auto inverse = _mm_sub_ss(_mm_add_ss(estimate, estimate),
                              _mm_mul_ss(_mm_mul_ss(denominator, estimate), estimate));
    auto fraction = _mm_mul_ss(numerator, inverse);
    auto result =
        _mm_add_ps(a, _mm_mul_ps(_mm_shuffle_ps(fraction, fraction, 0), _mm_sub_ps(b, a)));
    _mm_storeh_pi(reinterpret_cast<__m64 *>(out), result);
    _mm_store_ss(out + 2, result);
    return out;
}
} // namespace nfs::graphics::d3dx
