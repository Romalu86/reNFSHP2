#include "d3dx_matrix_dispatch.hpp"
#include "../../math/precision/extended_float.hpp"
#include <cstring>
#include <xmmintrin.h>
#include <mm3dnow.h>
namespace nfs::graphics::d3dx {
Multiply &matrix_multiply = *reinterpret_cast<Multiply *>(&pending::dispatch[3]);
Transpose &matrix_transpose = *reinterpret_cast<Transpose *>(&pending::dispatch[37]);

float *__stdcall multiply_bootstrap(float *out, const float *left, const float *right) {
    pending::initialize_dispatch(1);
    return matrix_multiply(out, left, right);
}

void __stdcall transpose_bootstrap(float *out, const float *input) {
    pending::initialize_dispatch(1);
    matrix_transpose(out, input);
}

float *__stdcall multiply_baseline(float *out, const float *left, const float *right) {
    float saved[16];
    auto format = kernel::numeric::current_format();
    auto X = [&](float f) { return kernel::numeric::Extended(f, format); };
    if (right == out) {
        if (left != out) {
            for (unsigned column = 0; column < 4; ++column) {
                float source[4] = {right[column], right[column + 4], right[column + 8],
                                   right[column + 12]};
                for (unsigned row = 0; row < 4; ++row)
                    out[row * 4 + column] =
                        float((X(source[2]) * left[row * 4 + 2] + X(source[0]) * left[row * 4]) +
                              (X(source[1]) * left[row * 4 + 1] + X(source[3]) * left[row * 4 + 3]));
            }
            return out;
        }
        std::memcpy(saved, right, 64);
        right = saved;
    }
    for (unsigned row = 0; row < 4; ++row) {
        float source[4] = {left[row * 4], left[row * 4 + 1], left[row * 4 + 2], left[row * 4 + 3]};
        for (unsigned column = 0; column < 4; ++column)
            out[row * 4 + column] =
                float((X(source[2]) * right[column + 8] + X(source[0]) * right[column]) +
                      (X(source[1]) * right[column + 4] + X(source[3]) * right[column + 12]));
    }
    return out;
}
namespace {
void multiply_vectors(float *out, const float *left, const float *right) {
    __m128 rows[4] = {_mm_loadu_ps(right), _mm_loadu_ps(right + 4), _mm_loadu_ps(right + 8),
                      _mm_loadu_ps(right + 12)};
    float input[16];
    // SSE bodies load all four left rows before writing output.
    std::memcpy(input, left, 64);
    for (unsigned row = 0; row < 4; ++row) {
        __m128 sum = _mm_mul_ps(_mm_set1_ps(input[row * 4]), rows[0]);
        sum = _mm_add_ps(sum, _mm_mul_ps(_mm_set1_ps(input[row * 4 + 1]), rows[1]));
        sum = _mm_add_ps(sum, _mm_mul_ps(_mm_set1_ps(input[row * 4 + 2]), rows[2]));
        sum = _mm_add_ps(sum, _mm_mul_ps(_mm_set1_ps(input[row * 4 + 3]), rows[3]));
        _mm_storeu_ps(out + row * 4, sum);
    }
}
} // namespace

float *__stdcall multiply_sse(float *out, const float *left, const float *right) {
    multiply_vectors(out, left, right);
    return out;
}

float *__stdcall multiply_sse2(float *out, const float *left, const float *right) {
    multiply_vectors(out, left, right);
    return out;
}

float *__stdcall multiply_packed(float *out, const float *left, const float *right) {
    _m_femms();
    // The body transposes right into registers before the first write.
    Dword columns[4][4];
    for (unsigned column = 0; column < 4; ++column)
        for (unsigned row = 0; row < 4; ++row)
            std::memcpy(&columns[column][row], right + row * 4 + column, 4);
    for (unsigned row = 0; row < 4; ++row) {
        __m64 first = *reinterpret_cast<const __m64 *>(left + row * 4);
        __m64 second = *reinterpret_cast<const __m64 *>(left + row * 4 + 2);
        __m64 a[4], b[4];
        for (unsigned column = 0; column < 4; ++column) {
            a[column] = _m_pfmul(first, *reinterpret_cast<const __m64 *>(columns[column]));
            b[column] = _m_pfmul(second, *reinterpret_cast<const __m64 *>(columns[column] + 2));
        }
        *reinterpret_cast<__m64 *>(out + row * 4) =
            _m_pfadd(_m_pfacc(a[0], a[1]), _m_pfacc(b[0], b[1]));
        *reinterpret_cast<__m64 *>(out + row * 4 + 2) =
            _m_pfadd(_m_pfacc(a[2], a[3]), _m_pfacc(b[2], b[3]));
    }
    _m_femms();
    return out;
}
} // namespace nfs::graphics::d3dx
namespace nfs::graphics::model_render::pending {

float *__stdcall d3dx_multiply(float *output, const float *left, const float *right) {
    return d3dx::matrix_multiply(output, left, right);
}

void __stdcall d3dx_transpose(float *output, const float *input) {
    d3dx::matrix_transpose(output, input);
}
} // namespace nfs::graphics::model_render::pending
namespace nfs::game::world::vehicle_render::pending {
// The saved wrapper reverses its two input operands.
float *multiply_skeleton_matrix(float *output, const float *left, const float *right) {
    return graphics::model_render::pending::d3dx_multiply(output, right, left);
}
} // namespace nfs::game::world::vehicle_render::pending
