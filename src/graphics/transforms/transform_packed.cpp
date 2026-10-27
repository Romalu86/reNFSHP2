#include "nfs/graphics/transforms/transform_handlers.hpp"
#include <mm3dnow.h>
namespace nfs::game::runtime::matrices::handlers::pending {
namespace {
__forceinline __m64 pair(const void *p) {
    return *static_cast<const __m64 *>(p);
}
void packed_points(int count, const void *input, void *output, const float *translation_cache,
                   bool indexed, bool project) {
    const auto *indices = static_cast<const Dword *>(matrices::pending::transform_vertex_buffer);
    auto *source = static_cast<const Byte *>(input);
    auto *destination = static_cast<Byte *>(output);
    do {
        const Dword index = indexed ? *indices++ : 0;
        const auto *vertex = reinterpret_cast<const Dword *>(
            indexed ? static_cast<const Byte *>(input) + index * input_stride : source);
        auto *result = reinterpret_cast<Dword *>(
            indexed ? static_cast<Byte *>(output) + index * output_stride : destination);
        __m64 xy = pair(vertex), z = _m_from_int(static_cast<int>(vertex[2]));
        __m64 depth = _m_pfmul(pair(prepared_matrix + 8), xy),
              depth_tail = _m_pfmul(pair(prepared_matrix + 10), z),
              x = _m_pfmul(pair(prepared_matrix), xy), xt = _m_pfmul(pair(prepared_matrix + 2), z),
              y = _m_pfmul(pair(prepared_matrix + 4), xy),
              yt = _m_pfmul(pair(prepared_matrix + 6), z);
        depth = _m_pfadd(depth, depth_tail);
        x = _m_pfadd(x, xt);
        y = _m_pfadd(y, yt);
        depth = _m_pfadd(_m_pfacc(depth, depth), pair(translation_cache + 2));
        __m64 screen = _m_pfadd(_m_pfacc(x, y), pair(translation_cache));
        if (project) {
            __m64 reciprocal = depth, estimate = _m_pfrcp(depth);
            reciprocal = _m_pfrcpit1(reciprocal, estimate);
            depth = _m_pfsub(
                depth, _m_from_int(static_cast<int>(*reinterpret_cast<const Dword *>(&near_plane))));
            screen = _m_pfmul(screen, pair(prepared_vectors + 4));
            reciprocal = _m_pfrcpit2(reciprocal, estimate);
            depth = _m_pfmul(depth, _m_from_int(static_cast<int>(
                                        *reinterpret_cast<const Dword *>(&depth_scale))));
            result[3] = Dword(_m_to_int(reciprocal));
            screen = _m_pfmul(screen, reciprocal);
            result[2] = Dword(_m_to_int(depth));
            screen = _m_pfadd(screen, pair(prepared_vectors + 8));
            *reinterpret_cast<__m64 *>(result) = screen;
        } else {
            result[2] = Dword(_m_to_int(depth));
            *reinterpret_cast<__m64 *>(result) = screen;
        }
        source += input_stride;
        destination += output_stride;
    } while (--count);
    _m_femms();
}
} // namespace
// Real 3DNow operations, including the two reciprocal refinement stages.
void packed_affine(int n, const void *i, void *o, const float *p) {
    packed_points(n, i, o, p, false, false);
}

void packed_affine_project(int n, const void *i, void *o, const float *p) {
    packed_points(n, i, o, p, false, true);
}

void packed_affine_indexed(int n, const void *i, void *o, const float *p) {
    packed_points(n, i, o, p, true, false);
}

void packed_affine_project_indexed(int n, const void *i, void *o, const float *p) {
    packed_points(n, i, o, p, true, true);
}
} // namespace nfs::game::runtime::matrices::handlers::pending
