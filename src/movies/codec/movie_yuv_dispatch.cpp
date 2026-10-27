#include "movie_yuv.hpp"
#include "nfs/core/memory/memory_primitives.hpp"
#include <mmintrin.h>
namespace nfs::movies::pending {

Dword convert_yuv(const Byte *luma, const Byte *u, const Byte *v, Dword source_stride, Dword width,
                 int height, Byte *output, Dword output_stride, Dword format) {
    if (!kernel::cpu_mmx)
        return 0;
    if (format > 7)
        return 1;
    if (source_stride & 1)
        return 2;
    if (format == 1) {
        if (output_stride & 7)
            return 3;
        if (reinterpret_cast<Dword>(output) & 7) {
            if (reinterpret_cast<Dword>(output) & 3)
                return 4;
            output = yuv::advance(output, 4);
            --width;
        }
    } else {
        if (output_stride & 3)
            return 3;
        if (reinterpret_cast<Dword>(output) & 3) {
            if (reinterpret_cast<Dword>(output) & 1)
                return 4;
            output = yuv::advance(output, 2);
            --width;
        }
    }
    width &= 4294967294u;
    if (format == 7 && int(width) > 1024)
        return 5;
    if (int(width) < 2 || height < 1)
        return 0;
    Dword chroma_stride = Dword(int(source_stride) >> 1);
    Dword row_bytes = (format == 0 || format == 2 || format == 3) ? width * 2 : width * 4;
    if (format == 1)
        yuv::set_component_order(0);
    else if (format >= 2)
        yuv::configure_packed(format & 1);
    if (format >= 6) {
        Byte *previous = yuv::filter_rows + 4104;
        Byte *current = yuv::filter_rows;
        yuv::row_colour(luma, u, v, previous, yuv::advance(previous, row_bytes));
        for (int row = 0; row < height; ++row) {
            yuv::row_colour(luma, u, v, current, yuv::advance(current, row_bytes));
            yuv::row_horizontal(previous, output, yuv::advance(output, row_bytes));
            yuv::dither ^= yuv::dither_flip;
            output = yuv::advance(output, output_stride);
            yuv::row_vertical(current, previous, output, yuv::advance(output, row_bytes));
            yuv::dither ^= yuv::dither_flip;
            output = yuv::advance(output, output_stride);
            luma = yuv::advance(luma, source_stride);
            if (row & 1) {
                u = yuv::advance(u, chroma_stride);
                v = yuv::advance(v, chroma_stride);
            }
            Byte *swap = current;
            current = previous;
            previous = swap;
        }
    } else {
        for (int row = 0; row < height; ++row) {
            Byte *end = yuv::advance(output, row_bytes);
            if (format == 0)
                yuv::row_yuy2(luma, u, v, output, end);
            else if (format == 1)
                yuv::row_colour(luma, u, v, output, end);
            else if (format < 4)
                yuv::row_packed(luma, u, v, output, end);
            else
                yuv::row_doubled(luma, u, v, output, end);
            luma = yuv::advance(luma, source_stride);
            if (row & 1) {
                u = yuv::advance(u, chroma_stride);
                v = yuv::advance(v, chroma_stride);
            }
            output = yuv::advance(output, output_stride);
            if (format >= 2)
                yuv::dither ^= yuv::dither_flip;
        }
    }
    // Native YUY2 uses integer stores and returns without changing x87 tags.
    // The other row formats leave MMX active, so their dispatcher owns EMMS.
    if (format != 0)
        _mm_empty();
    return 0;
}
} // namespace nfs::movies::pending
