#include "d3dx_dispatch_kernels.hpp"
#include <xmmintrin.h>
#include <cstring>
namespace nfs::graphics::d3dx {
namespace {
__m128 normal3(const float *input, const float *matrix) {
    auto x = _mm_mul_ps(_mm_set1_ps(input[0]), _mm_loadu_ps(matrix));
    auto y = _mm_mul_ps(_mm_set1_ps(input[1]), _mm_loadu_ps(matrix + 4));
    auto z = _mm_mul_ps(_mm_set1_ps(input[2]), _mm_loadu_ps(matrix + 8));
    return _mm_add_ps(_mm_add_ps(x, y), z);
}
__m128 point3(const float *input, const float *matrix) {
    auto xy = _mm_add_ps(_mm_mul_ps(_mm_set1_ps(input[0]), _mm_loadu_ps(matrix)),
                         _mm_mul_ps(_mm_set1_ps(input[1]), _mm_loadu_ps(matrix + 4)));
    auto z_translation = _mm_add_ps(_mm_mul_ps(_mm_set1_ps(input[2]), _mm_loadu_ps(matrix + 8)),
                                    _mm_loadu_ps(matrix + 12));
    return _mm_add_ps(xy, z_translation);
}
__m128 vector4(const float *input, const float *matrix) {
    auto x = _mm_mul_ps(_mm_set1_ps(input[0]), _mm_loadu_ps(matrix));
    auto y = _mm_mul_ps(_mm_set1_ps(input[1]), _mm_loadu_ps(matrix + 4));
    auto z = _mm_mul_ps(_mm_set1_ps(input[2]), _mm_loadu_ps(matrix + 8));
    auto w = _mm_mul_ps(_mm_set1_ps(input[3]), _mm_loadu_ps(matrix + 12));
    return _mm_add_ps(_mm_add_ps(x, y), _mm_add_ps(z, w));
}
__m128 point2(const float *input, const float *matrix) {
    auto x = _mm_mul_ps(_mm_set1_ps(input[0]), _mm_loadu_ps(matrix));
    auto y = _mm_mul_ps(_mm_set1_ps(input[1]), _mm_loadu_ps(matrix + 4));
    return _mm_add_ps(_mm_add_ps(x, _mm_loadu_ps(matrix + 12)), y);
}
void store_three(float *output, __m128 value) {
    float values[4];
    _mm_storeu_ps(values, value);
    std::memcpy(output, values, 8);
    std::memcpy(output + 2, values + 2, 4);
}
void coordinate3(float *output, const float *input, const float *matrix) {
    // The saved coordinate body groups z*row2+row3 before adding x/y.
    auto z_translation = _mm_add_ps(_mm_mul_ps(_mm_set1_ps(input[2]), _mm_loadu_ps(matrix + 8)),
                                    _mm_loadu_ps(matrix + 12));
    auto xy = _mm_add_ps(_mm_mul_ps(_mm_set1_ps(input[0]), _mm_loadu_ps(matrix)),
                         _mm_mul_ps(_mm_set1_ps(input[1]), _mm_loadu_ps(matrix + 4)));
    auto value = _mm_add_ps(xy, z_translation);
    auto reciprocal = _mm_rcp_ps(value);
    auto correction = _mm_mul_ps(_mm_mul_ps(reciprocal, value), reciprocal);
    reciprocal = _mm_sub_ps(_mm_add_ps(reciprocal, reciprocal), correction);
    reciprocal = _mm_shuffle_ps(reciprocal, reciprocal, 255);
    store_three(output, _mm_mul_ps(value, reciprocal));
}
void transposed_product(float *output, const float *left, const float *right) {
    __m128 rows[4] = {_mm_loadu_ps(right), _mm_loadu_ps(right + 4), _mm_loadu_ps(right + 8),
                      _mm_loadu_ps(right + 12)};
    float input[16];
    std::memcpy(input, left, 64);
    __m128 product[4];
    for (unsigned row = 0; row < 4; ++row) {
        product[row] = _mm_add_ps(_mm_mul_ps(_mm_set1_ps(input[row * 4]), rows[0]),
                                  _mm_mul_ps(_mm_set1_ps(input[row * 4 + 1]), rows[1]));
        product[row] =
            _mm_add_ps(product[row], _mm_mul_ps(_mm_set1_ps(input[row * 4 + 2]), rows[2]));
        product[row] =
            _mm_add_ps(product[row], _mm_mul_ps(_mm_set1_ps(input[row * 4 + 3]), rows[3]));
    }
    _MM_TRANSPOSE4_PS(product[0], product[1], product[2], product[3]);
    float transposed[16];
    for (unsigned row = 0; row < 4; ++row)
        _mm_storeu_ps(transposed + row * 4, product[row]);
    for (unsigned row = 0; row < 4; ++row)
        std::memcpy(output + row * 4, transposed + row * 4, 8);
    for (unsigned row = 0; row < 4; ++row)
        std::memcpy(output + row * 4 + 2, transposed + row * 4 + 2, 8);
}
} // namespace

void __stdcall transform_point2_sse(float *output, const float *input, const float *matrix) {
    _mm_storeu_ps(output, point2(input, matrix));
}

void __stdcall transform_coordinate2_sse(float *output, const float *input, const float *matrix) {
    auto value = point2(input, matrix);
    auto reciprocal = _mm_rcp_ps(value);
    auto reciprocal_product = _mm_mul_ps(_mm_mul_ps(reciprocal, reciprocal), value);
    auto refined = _mm_sub_ps(_mm_add_ps(reciprocal, reciprocal), reciprocal_product);
    (void)refined;

    auto scale = _mm_shuffle_ps(reciprocal_product, reciprocal_product, 255);
    float result[4];
    _mm_storeu_ps(result, _mm_mul_ps(value, scale));
    std::memcpy(output, result, 8);
}

void __stdcall transform_normal2_sse(float *output, const float *input, const float *matrix) {
    auto x = _mm_mul_ps(_mm_set1_ps(input[0]), _mm_loadu_ps(matrix));
    auto y = _mm_mul_ps(_mm_set1_ps(input[1]), _mm_loadu_ps(matrix + 4));
    float result[4];
    _mm_storeu_ps(result, _mm_add_ps(x, y));
    std::memcpy(output, result, 8);
}

void __stdcall transform_normal3_sse(float *output, const float *input, const float *matrix) {
    store_three(output, normal3(input, matrix));
}

void __stdcall transform_normal3_sse2(float *output, const float *input, const float *matrix) {
    store_three(output, normal3(input, matrix));
}

void __stdcall transform_point3_sse(float *output, const float *input, const float *matrix) {
    _mm_storeu_ps(output, point3(input, matrix));
}

void __stdcall transform_point3_sse2(float *output, const float *input, const float *matrix) {
    _mm_storeu_ps(output, point3(input, matrix));
}

void __stdcall transform_coordinate3_sse(float *output, const float *input, const float *matrix) {
    coordinate3(output, input, matrix);
}

void __stdcall transform_coordinate3_sse2(float *output, const float *input, const float *matrix) {
    coordinate3(output, input, matrix);
}

void __stdcall transform_vector4_sse(float *output, const float *input, const float *matrix) {
    _mm_storeu_ps(output, vector4(input, matrix));
}

void __stdcall transform_vector4_sse2(float *output, const float *input, const float *matrix) {
    _mm_storeu_ps(output, vector4(input, matrix));
}

void __stdcall multiply_transposed_sse(float *output, const float *left, const float *right) {
    transposed_product(output, left, right);
}

void __stdcall multiply_transposed_sse2(float *output, const float *left, const float *right) {
    transposed_product(output, left, right);
}
} // namespace nfs::graphics::d3dx
