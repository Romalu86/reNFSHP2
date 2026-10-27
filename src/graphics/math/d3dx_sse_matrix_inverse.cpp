#include "d3dx_dispatch_kernels.hpp"
#include <xmmintrin.h>
namespace nfs::graphics::d3dx {
namespace {
__m128 exchange_pairs(__m128 value) {
    return _mm_shuffle_ps(value, value, 177);
}
__m128 exchange_halves(__m128 value) {
    return _mm_shuffle_ps(value, value, 78);
}
float *inverse_vectors(float *out, float *determinant, const float *matrix) {
    auto column0 = _mm_loadu_ps(matrix), column1 = _mm_loadu_ps(matrix + 4);
    auto column2 = _mm_loadu_ps(matrix + 8), column3 = _mm_loadu_ps(matrix + 12);
    _MM_TRANSPOSE4_PS(column0, column1, column2, column3);
    column1 = exchange_halves(column1);
    column3 = exchange_halves(column3);
    auto product23 = exchange_pairs(_mm_mul_ps(column3, column2));
    auto opposite23 = exchange_halves(product23);
    auto lower23 = _mm_mul_ps(opposite23, column0), upper23 = _mm_mul_ps(product23, column0);
    auto first = _mm_sub_ps(_mm_mul_ps(column1, opposite23), _mm_mul_ps(column1, product23));
    auto product12 = exchange_pairs(_mm_mul_ps(column1, column2));
    auto opposite12 = exchange_halves(product12);
    auto lower12 = _mm_mul_ps(product12, column0), upper12 = _mm_mul_ps(opposite12, column0);
    first = _mm_add_ps(_mm_mul_ps(column3, product12), first);
    first = _mm_sub_ps(first, _mm_mul_ps(column3, opposite12));
    auto opposite_column2 = exchange_halves(column2);
    auto product13 = exchange_pairs(_mm_mul_ps(exchange_halves(column1), column3));
    auto opposite13 = exchange_halves(product13);
    first = _mm_add_ps(_mm_mul_ps(opposite_column2, product13), first);
    first = _mm_sub_ps(first, _mm_mul_ps(opposite_column2, opposite13));
    auto fourth = exchange_halves(
        _mm_sub_ps(_mm_mul_ps(opposite13, column0), _mm_mul_ps(product13, column0)));
    auto products = _mm_mul_ps(column0, first);
    auto sum = _mm_add_ps(exchange_halves(products), products);
    auto value = _mm_add_ss(exchange_pairs(sum), sum);
    auto product01 = exchange_pairs(_mm_mul_ps(column0, column1));
    auto opposite01 = exchange_halves(product01);
    auto third = _mm_sub_ps(_mm_mul_ps(column3, opposite01),
                            _mm_add_ps(_mm_mul_ps(column3, product01), fourth));
    auto product03 = exchange_pairs(_mm_mul_ps(column3, column0));
    auto opposite03 = exchange_halves(product03);
    third = _mm_add_ps(_mm_mul_ps(column1, product03), third);
    third = _mm_sub_ps(third, _mm_mul_ps(column1, opposite03));
    auto second = exchange_halves(_mm_sub_ps(lower23, upper23));
    second = _mm_sub_ps(second, _mm_mul_ps(product03, opposite_column2));
    second = _mm_add_ps(second, _mm_mul_ps(opposite03, opposite_column2));
    auto product02 = exchange_pairs(_mm_mul_ps(column0, opposite_column2));
    auto opposite02 = exchange_halves(product02);
    second = _mm_add_ps(second, _mm_mul_ps(column3, product02));
    second = _mm_sub_ps(second, _mm_mul_ps(column3, opposite02));
    fourth = exchange_halves(_mm_sub_ps(upper12, lower12));
    fourth = _mm_sub_ps(_mm_mul_ps(product01, opposite_column2), fourth);
    fourth = _mm_sub_ps(fourth, _mm_mul_ps(opposite01, opposite_column2));
    fourth = _mm_sub_ps(fourth, _mm_mul_ps(product02, column1));
    fourth = _mm_add_ps(_mm_mul_ps(column1, opposite02), fourth);
    // Both implementations leave both destinations untouched on COMISS ZF.
    if (_mm_comieq_ss(value, _mm_setzero_ps()))
        return nullptr;
    if (determinant)
        _mm_store_ss(determinant, value);
    auto estimate = _mm_rcp_ss(value);
    auto inverse = _mm_sub_ss(_mm_add_ss(estimate, estimate),
                              _mm_mul_ss(value, _mm_mul_ss(estimate, estimate)));
    inverse = _mm_shuffle_ps(inverse, inverse, 0);
    _mm_storeu_ps(out, _mm_mul_ps(first, inverse));
    _mm_storeu_ps(out + 4, _mm_mul_ps(second, inverse));
    _mm_storeu_ps(out + 8, _mm_mul_ps(third, inverse));
    _mm_storeu_ps(out + 12, _mm_mul_ps(inverse, fourth));
    return out;
}
} // namespace

float *__stdcall invert_matrix_sse(float *out, float *determinant, const float *matrix) {
    return inverse_vectors(out, determinant, matrix);
}

float *__stdcall invert_matrix_sse2(float *out, float *determinant, const float *matrix) {
    return inverse_vectors(out, determinant, matrix);
}
} // namespace nfs::graphics::d3dx
