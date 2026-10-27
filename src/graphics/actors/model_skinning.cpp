#include "model_render_runtime.hpp"
#include "../../math/precision/extended_float.hpp"
#include <xmmintrin.h>
#include <mm3dnow.h>
#include <cstring>
namespace nfs::graphics::model_render {
alignas(16) float skin_matrices[129][16]{};
Dword *blend_weights{};
int blend_count{}, three_weights{}, two_weights{}, one_weight{};
namespace {
float weight(Dword bits) {
    float value;
    std::memcpy(&value, &bits, 4);
    return value;
}
void blend_scalar(int index, const float *matrices, float *output, const Dword *weights,
                  unsigned count) {
    const Dword *entry = weights + Dword(index) * 4;
    const float *inputs[3];
    float amounts[3];
    for (unsigned i = 0; i < count; ++i) {
        inputs[i] = matrices + (entry[i] & 255) * 16;
        amounts[i] = weight(entry[i]);
    }
    auto format = kernel::numeric::current_format();
    auto X = [&](float f) { return kernel::numeric::Extended(f, format); };
    // Each partial sum is stored in the destination in the x87 path.
    for (unsigned column = 0; column < 4; ++column)
        for (unsigned row = 0; row < 4; ++row) {
            unsigned offset = row * 4 + column;
            output[offset] = float(X(amounts[0]) * inputs[0][offset]);
            for (unsigned i = 1; i < count; ++i)
                output[offset] = float(X(amounts[i]) * inputs[i][offset] + output[offset]);
        }
}
void blend_sse(int index, const float *matrices, float *output, const Dword *weights,
               unsigned count) {
    const Dword *entry = weights + Dword(index) * 4;
    const float *inputs[3];
    __m128 amounts[3];
    for (unsigned i = 0; i < count; ++i) {
        inputs[i] = matrices + (entry[i] & 255) * 16;
        amounts[i] = _mm_set1_ps(weight(entry[i]));
    }
    for (unsigned row = 0; row < 4; ++row) {
        __m128 value = _mm_mul_ps(_mm_loadu_ps(inputs[0] + row * 4), amounts[0]);
        for (unsigned i = 1; i < count; ++i)
            value = _mm_add_ps(value, _mm_mul_ps(_mm_loadu_ps(inputs[i] + row * 4), amounts[i]));
        _mm_storeu_ps(output + row * 4, value);
    }
}
void blend_packed(int index, const float *matrices, float *output, const Dword *weights,
                  unsigned count) {
    const Dword *entry = weights + Dword(index) * 4;
    const __m64 *inputs[3];
    __m64 amounts[3];
    for (unsigned i = 0; i < count; ++i) {
        inputs[i] = reinterpret_cast<const __m64 *>(matrices + (entry[i] & 255) * 16);
        amounts[i] = _m_punpckldq(_m_from_int(int(entry[i])), _m_from_int(int(entry[i])));
    }
    for (unsigned pair = 0; pair < 8; ++pair) {
        __m64 value = _m_pfmul(inputs[0][pair], amounts[0]);
        for (unsigned i = 1; i < count; ++i)
            value = _m_pfadd(value, _m_pfmul(inputs[i][pair], amounts[i]));
        reinterpret_cast<__m64 *>(output)[pair] = value;
    }
}
void evaluate_palette(unsigned arithmetic, bool transpose, bool upload) {
    int count = static_cast<int>(commands[2]) >> 2;
    if (blend_count < count)
        count = blend_count;
    const float *matrices = pointer<float>(parameters[1]);
    const int three = three_weights, two_end = int(Dword(three) + Dword(two_weights));
    const int end = int(Dword(two_end) + Dword(one_weight));
    const Dword *weights = blend_weights;
    int index = 0;
    for (; index < three; ++index) {
        if (arithmetic == 0)
            blend_scalar(index, matrices, skin_matrices[index], weights, 3);
        else if (arithmetic == 1)
            blend_sse(index, matrices, skin_matrices[index], weights, 3);
        else
            blend_packed(index, matrices, skin_matrices[index], weights, 3);
    }
    for (; index < two_end; ++index) {
        if (arithmetic == 0)
            blend_scalar(index, matrices, skin_matrices[index], weights, 2);
        else if (arithmetic == 1)
            blend_sse(index, matrices, skin_matrices[index], weights, 2);
        else
            blend_packed(index, matrices, skin_matrices[index], weights, 2);
    }
    for (; index < end; ++index)
        kernel::memory_copy(skin_matrices[index], matrices + (weights[Dword(index) * 4] & 255) * 16,
                            64);
    if (transpose)
        for (int matrix = 0; matrix < count; ++matrix)
            pending::d3dx_transpose(skin_matrices[matrix], skin_matrices[matrix]);
    if (upload)
        backend::vertex_constants(commands[1], reinterpret_cast<Dword>(skin_matrices),
                                  Dword(count) * 4);
}
} // namespace

void blend_three_matrices(int index, const float *matrices, float *output, const Dword *weights) {
    blend_scalar(index, matrices, output, weights, 3);
}

void blend_two_matrices(int index, const float *matrices, float *output, const Dword *weights) {
    blend_scalar(index, matrices, output, weights, 2);
}

void copy_single_matrix(int index, const float *matrices, float *output, const Dword *weights) {
    kernel::memory_copy(output, matrices + (weights[Dword(index) * 4] & 255) * 16, 64);
}

void blend_three_matrices_packed(int index, const float *matrices, float *output,
                                 const Dword *weights) {
    blend_packed(index, matrices, output, weights, 3);
}

void upload_skin_constants() {
    evaluate_palette(0, false, true);
}

void upload_skin_constants_sse() {
    evaluate_palette(1, false, true);
}

void upload_skin_constants_packed() {
    evaluate_palette(2, false, true);
}

void transpose_skin_constants() {
    evaluate_palette(0, true, false);
}

void transpose_skin_constants_sse() {
    evaluate_palette(1, true, false);
}

void transpose_skin_constants_packed() {
    evaluate_palette(2, true, false);
}

void set_skin_palette() {
    blend_weights = pointer<Dword>(parameters[1]);
    blend_count = int(parameters[0]);
    three_weights = int(commands[1]);
    two_weights = int(commands[2]);
    one_weight = int(commands[3]);
}
// Alternate entry uses the same palette operations.
void upload_skin_constants_variant() {
    upload_skin_constants();
}
// Alternate entry uses the same palette operations.
void upload_skin_constants_sse_variant() {
    upload_skin_constants_sse();
}
// Alternate entry uses the same palette operations.
void upload_skin_constants_packed_variant() {
    upload_skin_constants_packed();
}
// Alternate entry uses the same palette operations.
void transpose_skin_constants_variant() {
    transpose_skin_constants();
}
// Alternate entry uses the same palette operations.
void transpose_skin_constants_sse_variant() {
    transpose_skin_constants_sse();
}
// Alternate entry uses the same palette operations.
void transpose_skin_constants_packed_variant() {
    transpose_skin_constants_packed();
}
} // namespace nfs::graphics::model_render
