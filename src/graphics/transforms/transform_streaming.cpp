#include "nfs/graphics/transforms/transform_handlers.hpp"
#include <xmmintrin.h>
namespace nfs::game::runtime::matrices::handlers::pending {
namespace {
void streaming_points(int count, const void *input, void *output, const float *translation_cache,
                      bool indexed, bool project) {
    const __m128 row0 = _mm_load_ps(reinterpret_cast<const float *>(prepared_matrix)),
                 row1 = _mm_load_ps(reinterpret_cast<const float *>(prepared_matrix + 4)),
                 row2 = _mm_load_ps(reinterpret_cast<const float *>(prepared_matrix + 8)),
                 position = _mm_load_ps(translation_cache);
    const auto *indices = static_cast<const Dword *>(matrices::pending::transform_vertex_buffer);
    auto *source = static_cast<const Byte *>(input);
    auto *destination = static_cast<Byte *>(output);
    do {
        const Dword index = indexed ? *indices++ : 0;
        const auto *vertex = reinterpret_cast<const float *>(
            indexed ? static_cast<const Byte *>(input) + index * input_stride : source);
        auto *result = reinterpret_cast<float *>(
            indexed ? static_cast<Byte *>(output) + index * output_stride : destination);
        __m128 x = _mm_mul_ps(_mm_set1_ps(vertex[0]), row0),
               y = _mm_mul_ps(_mm_set1_ps(vertex[1]), row1),
               z = _mm_mul_ps(_mm_set1_ps(vertex[2]), row2);
        x = _mm_add_ps(x, position);
        y = _mm_add_ps(y, z);
        x = _mm_add_ps(x, y);
        if (project) {
            __m128 reciprocal = _mm_rcp_ss(x), saved = x;
            x = _mm_mul_ss(x, reciprocal);
            x = _mm_mul_ss(x, reciprocal);
            reciprocal = _mm_add_ss(reciprocal, reciprocal);
            reciprocal = _mm_sub_ss(reciprocal, x);
            __m128 screen = _mm_mul_ps(
                _mm_load_ps(reinterpret_cast<const float *>(prepared_vectors + 4)), saved);
            saved = _mm_sub_ss(saved, _mm_load_ss(&near_plane));
            saved = _mm_mul_ss(saved, _mm_load_ss(&depth_scale));
            _mm_store_ss(result + 2, saved);
            reciprocal = _mm_shuffle_ps(reciprocal, reciprocal, 0);
            screen = _mm_mul_ps(screen, reciprocal);
            _mm_store_ss(result + 3, reciprocal);
            screen = _mm_add_ps(screen,
                                _mm_load_ps(reinterpret_cast<const float *>(prepared_vectors + 8)));
            _mm_store_ss(result, _mm_shuffle_ps(screen, screen, 170));
            _mm_store_ss(result + 1, _mm_shuffle_ps(screen, screen, 85));
        } else {
            _mm_store_ss(result + 2, x);
            _mm_store_ss(result + 1, _mm_shuffle_ps(x, x, 85));
            _mm_store_ss(result, _mm_shuffle_ps(x, x, 170));
        }
        source += input_stride;
        destination += output_stride;
    } while (--count);
}
} // namespace
// Saved SSE association, reciprocal refinement and store order.
void streaming_affine(int n, const void *i, void *o, const float *p) {
    streaming_points(n, i, o, p, false, false);
}

void streaming_affine_project(int n, const void *i, void *o, const float *p) {
    streaming_points(n, i, o, p, false, true);
}

void streaming_affine_indexed(int n, const void *i, void *o, const float *p) {
    streaming_points(n, i, o, p, true, false);
}

void streaming_affine_project_indexed(int n, const void *i, void *o, const float *p) {
    streaming_points(n, i, o, p, true, true);
}
} // namespace nfs::game::runtime::matrices::handlers::pending
