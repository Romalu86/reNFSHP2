#include "model_render_runtime.hpp"
#include "nfs/core/platform/cpu.hpp"
#include "../../math/precision/extended_float.hpp"
#include <xmmintrin.h>
#include <mm3dnow.h>
#include <cstring>
namespace nfs::graphics::model_render {
namespace {
__m64 duplicate(float value) {
    Dword bits;
    std::memcpy(&bits, &value, 4);
    __m64 word = _m_from_int(int(bits));
    return _m_punpckldq(word, word);
}
void transform_skinned_vertex(const float *input, float *output, bool normal, unsigned arithmetic) {
    Dword matrix_index;
    std::memcpy(&matrix_index, input + 3, 4);
    const float *matrix = skin_matrices[matrix_index];
    Dword colour;
    if (normal && (arithmetic != 0))
        std::memcpy(&colour, output + 7, 4);
    if (arithmetic == 1) {
        __m128 rows[4] = {_mm_load_ps(matrix), _mm_load_ps(matrix + 4), _mm_load_ps(matrix + 8),
                          _mm_load_ps(matrix + 12)};
        __m128 x = _mm_mul_ps(rows[0], _mm_set1_ps(input[0]));
        __m128 y = _mm_mul_ps(rows[1], _mm_set1_ps(input[1]));
        __m128 z = _mm_mul_ps(rows[2], _mm_set1_ps(input[2]));
        _mm_storeu_ps(output, _mm_add_ps(_mm_add_ps(x, y), _mm_add_ps(z, rows[3])));
        if (normal) {
            __m128 nx = _mm_mul_ps(rows[0], _mm_set1_ps(input[4]));
            __m128 ny = _mm_mul_ps(rows[1], _mm_set1_ps(input[5]));
            __m128 nz = _mm_mul_ps(rows[2], _mm_set1_ps(input[6]));
            _mm_storeu_ps(output + 4, _mm_add_ps(_mm_add_ps(nx, ny), nz));
        }
        _m_empty();
        _m_empty();
    } else if (arithmetic == 2) {
        _m_femms();
        __m64 x = duplicate(input[0]), y = duplicate(input[1]), z = duplicate(input[2]);
        const __m64 *rows = reinterpret_cast<const __m64 *>(matrix);
        for (unsigned pair = 0; pair < 2; ++pair) {
            __m64 first = _m_pfadd(_m_pfmul(rows[pair], x), _m_pfmul(rows[pair + 2], y));
            __m64 second = _m_pfadd(_m_pfmul(rows[pair + 4], z), rows[pair + 6]);
            reinterpret_cast<__m64 *>(output)[pair] = _m_pfadd(first, second);
        }
        if (normal) {
            x = duplicate(input[4]);
            y = duplicate(input[5]);
            z = duplicate(input[6]);
            for (unsigned pair = 0; pair < 2; ++pair) {
                __m64 first = _m_pfadd(_m_pfmul(rows[pair], x), _m_pfmul(rows[pair + 2], y));
                reinterpret_cast<__m64 *>(output + 4)[pair] =
                    _m_pfadd(first, _m_pfmul(rows[pair + 4], z));
            }
        }
        _m_femms();
    } else {
        math::transform_position(input, matrix, output);
        if (normal) {
            auto format = kernel::numeric::current_format();
            auto X = [&](float f) { return kernel::numeric::Extended(f, format); };
            for (unsigned column = 0; column < 3; ++column)
                output[4 + column] =
                    float((X(input[4]) * matrix[column] + X(input[5]) * matrix[4 + column]) +
                          X(input[6]) * matrix[8 + column]);
        }
    }
    if (normal && (arithmetic != 0))
        std::memcpy(output + 7, &colour, 4);
}
} // namespace

void pending::skin_vertex_stream() {
    Dword stride = commands[2];
    bool normals = (commands[3] & 16) != 0;
    Dword source = commands[6];
    if (int(commands[8]) >= 0)
        source += commands[7] * buffers::selection_index * stride;
    int count = int(commands[7]);
    Dword *buffer = pointer<Dword>(commands[4]);
    const float *input = pointer<float>(commands[10]);
    Dword mapped;
    index_offset = buffers::lock_segment(buffer, 0, Dword(count) * stride, &mapped, source);
    auto *output = pointer<Byte>(mapped);
    unsigned arithmetic = kernel::cpu_sse ? 1u : kernel::cpu::amd_3d ? 2u : 0u;
    for (int vertex = 0; vertex < count; ++vertex) {
        transform_skinned_vertex(input, reinterpret_cast<float *>(output), normals, arithmetic);
        input += normals ? 8 : 4;
        output += stride;
    }
    buffers::unlock(buffer);
    backend::bind_stream(commands[1], buffer);
}
} // namespace nfs::graphics::model_render
