#include "nfs/graphics/textures/shape_conversion.hpp"
#include "nfs/graphics/resources/eagl_assets.hpp"
#include "nfs/core/memory/memory_primitives.hpp"

namespace nfs::graphics::textures::shape {

void compress_dxt_image(const void *source, void *destination, int width, int height, int format,
                        Dword quality) {
    int scale = quality * 104857;
    int precision = 8 - static_cast<int>(static_cast<int>(quality) * double(-0.07999999821186065673828125f));
    int threshold = 20000 - quality * 200;
    int pixel_offset = 0;
    auto *output = static_cast<Byte *>(destination);
    for (int y = 0; y < height; y += 4) {
        int tile_height = height - y < 5 ? height - y : 4;
        const auto *input = static_cast<const Dword *>(source) + pixel_offset;
        if (width > 0)
            pixel_offset += ((width - 1) / 4 + 1) * 4;
        for (int x = 0; x < width; x += 4) {
            int tile_width = width - x > 4 ? 4 : width - x;
            encode_dxt_tile(input, output, width, tile_width, tile_height, format, scale, precision,
                            threshold);
            output += format != 1 ? 16 : 8;
            input += 4;
        }
        if (width & 3)
            pixel_offset += -4 + (width & 3);
        pixel_offset += width * 3;
    }
}

// Preserve the recorded type assignment for every format,
// including the format-five header value and the pre-alignment copied dimensions.
Dword *pack_shape(Dword *image, int format, Dword quality) {
    Dword *converted = image;
    Byte output_type = Byte((format != 1) + 96);
    if (Byte(*image) != 125)
        converted = convert_depth(image, 32);
    if (image[3] & 8192) {
        Dword *linear = unswizzle_shape(converted);
        if (converted != image)
            release_shape(converted);
        converted = linear;
    }
    int offset = static_cast<int>(*image) >> 8;
    Dword *extra =
        offset ? reinterpret_cast<Dword *>(reinterpret_cast<Byte *>(image) + offset) : nullptr;
    int extra_bytes = attachment_bytes(extra);
    Dword header[4] = {(*image & ~255u) | output_type, image[1], image[2], image[3]};
    int width = static_cast<std::int16_t>(header[1]);
    int height = static_cast<std::int16_t>(header[1] >> 16);
    if (width % 4)
        width += 4 - width % 4;
    if (height % 4)
        height += 4 - height % 4;
    header[1] = std::uint16_t(width) | Dword(std::uint16_t(height)) << 16;
    int output_bytes = size(header);
    auto *output =
        static_cast<Dword *>(allocate_aligned("dxtpacked", output_bytes + extra_bytes, 0, 16, 0));
    kernel::memory_copy(output, converted, 16);
    *reinterpret_cast<Byte *>(output) = output_type;
    width = static_cast<std::int16_t>(output[1]);
    height = static_cast<std::int16_t>(output[1] >> 16);
    for (unsigned level = 0; level <= image[3] >> 28; ++level) {
        compress_dxt_image(tar::image_level(converted, 0, 0, level),
                           tar::image_level(output, 0, 0, level), width, height, format, quality);
        width >>= 1;
        height >>= 1;
    }
    if (extra_bytes) {
        offset = static_cast<int>(*image) >> 8;
        copy_attachments(reinterpret_cast<Byte *>(output) + output_bytes,
                         offset ? reinterpret_cast<Dword *>(reinterpret_cast<Byte *>(image) + offset)
                                : nullptr);
        *output = (*output & 255) ^ Dword(output_bytes) << 8;
    }
    if (converted != image)
        release(converted);
    return output;
}
} // namespace nfs::graphics::textures::shape
