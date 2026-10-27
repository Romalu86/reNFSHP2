#include "nfs/graphics/transforms/transform_handlers.hpp"
namespace nfs::game::runtime::matrices::handlers {
using namespace matrices::pending;
// Signed integer comparisons of float bit patterns are intentional.
void clip_vertex(void *vertex, Byte *code) {
    Byte bits = 0;
    const int x = at<int>(vertex, 0), y = at<int>(vertex, 4), w = at<int>(vertex, 12);
    if (x < static_cast<int>(clip_limits[7]))
        bits = 16;
    else if (x > static_cast<int>(clip_limits[3]))
        bits = 32;
    if (y < static_cast<int>(clip_limits[6]))
        bits |= 4;
    else if (y > static_cast<int>(clip_limits[0]))
        bits |= 8;
    if (w < 0)
        bits ^= 61;
    else if (w > static_cast<int>(clip_limits[1]))
        bits |= 1;
    else if (at<int>(vertex, 8) >= static_cast<int>(clip_limits[2]))
        bits |= 2;
    *code = bits;
}

void clip(int count, void *output) {
    clip_intersection = 255;
    clip_union = 0;
    clip_cursor = reinterpret_cast<void *>(transform_projection_matrix);
    saved_output = output;
    do {
        clip_vertex(saved_output, static_cast<Byte *>(clip_cursor));
        const int code = at<std::int8_t>(clip_cursor, 0);
        clip_intersection &= Dword(code);
        clip_union |= Dword(code);
        saved_output = static_cast<Byte *>(saved_output) + output_stride;
        clip_cursor = static_cast<Byte *>(clip_cursor) + 1;
    } while (--count);
}

void clip_indexed(int count, void *output) {
    saved_vertices = transform_vertex_buffer;
    clip_intersection = 255;
    clip_union = 0;
    clip_cursor = reinterpret_cast<void *>(transform_projection_matrix);
    do {
        Dword index = *static_cast<Dword *>(saved_vertices);
        saved_vertices = static_cast<Dword *>(saved_vertices) + 1;
        saved_output = static_cast<Byte *>(output) + index * output_stride;
        clip_cursor = reinterpret_cast<Byte *>(transform_projection_matrix) + index;
        clip_vertex(saved_output, static_cast<Byte *>(clip_cursor));
        Dword code = Dword(at<std::int8_t>(clip_cursor, 0));
        clip_intersection &= code;
        clip_union |= code;
    } while (--count);
}

void repeat(int count, void *output) {
    copy_cursor = reinterpret_cast<void *>(clip_flags);
    repeat_cursor = reinterpret_cast<void *>(transform_clip_matrix);
    saved_output = output;
    do {
        Dword copies = at<Dword>(repeat_cursor, 0);
        while (copies--) {
            kernel::memory_copy(copy_cursor, saved_output, 16);
            copy_cursor = static_cast<Byte *>(copy_cursor) + output_stride;
        }
        saved_output = static_cast<Byte *>(saved_output) + output_stride;
        repeat_cursor = static_cast<Dword *>(repeat_cursor) + 1;
    } while (--count);
}

void clip_and_repeat(int count, void *output) {
    copy_cursor = reinterpret_cast<void *>(clip_flags);
    repeat_cursor = reinterpret_cast<void *>(transform_clip_matrix);
    repeat_flags_cursor = reinterpret_cast<void *>(clip_aux);
    clip_intersection = 255;
    clip_union = 0;
    clip_cursor = reinterpret_cast<void *>(transform_projection_matrix);
    saved_output = output;
    do {
        clip_vertex(saved_output, static_cast<Byte *>(clip_cursor));
        Dword code = Dword(at<std::int8_t>(clip_cursor, 0));
        clip_intersection &= code;
        clip_union |= code;
        Dword copies = at<Dword>(repeat_cursor, 0);
        while (copies--) {
            kernel::memory_copy(copy_cursor, saved_output, 16);
            copy_cursor = static_cast<Byte *>(copy_cursor) + output_stride;
            at<Byte>(repeat_flags_cursor, 0) = at<Byte>(clip_cursor, 0);
            repeat_flags_cursor = static_cast<Byte *>(repeat_flags_cursor) + 1;
        }
        saved_output = static_cast<Byte *>(saved_output) + output_stride;
        clip_cursor = static_cast<Byte *>(clip_cursor) + 1;
        repeat_cursor = static_cast<Dword *>(repeat_cursor) + 1;
    } while (--count);
}
} // namespace nfs::game::runtime::matrices::handlers
