#include "nfs/graphics/textures/shape_conversion.hpp"
#include "nfs/graphics/resources/eagl_assets.hpp"
#include <cstdio>

namespace nfs::graphics::textures::shape {

Dword *convert_depth(Dword *image, int depth) {
    return convert_type(image, type_from_depth(depth), 1, -1, -1);
}

// The dispatch tables select intermediate formats as well as
// row functions. Preserve the ten-pass limit, ownership and post-processing order.
Dword *convert_type(Dword *image, Dword target, Byte transparency, int swizzle_mode, int mip_mode) {
    if (!image)
        return nullptr;
    Dword *owned = nullptr;
    Dword *source_palette = palette(image);
    int palette_count = 0;
    if (source_palette)
        palette_count = static_cast<std::int16_t>(source_palette[1]);
    else if (Byte(*image) == 100) {
        source_palette = image;
        palette_count = 256;
    }
    Dword type = Byte(*image);
    if (static_cast<std::int8_t>(type) < 0 || type == 96 || type == 97 || type == 98) {
        image = owned = unpack_shape(image);
        if ((*image & 255) == target)
            return image;
    }
    bool was_swizzled = false;
    if (image[3] & 8192) {
        image = unswizzle_shape(image);
        if (owned)
            release(owned);
        owned = image;
        was_swizzled = true;
    }
    bool had_mips = (image[3] & 4026531840u) != 0;
    Dword compressed = target & 128;
    Dword *output = nullptr;
    for (int attempt = 0; attempt < 10; ++attempt) {
        type = *image & 255;
        Dword intermediate = compressed || type == 96 || type == 97 || type == 98 ? 125 : target;
        int index = format_index(type) * 12 + format_index(intermediate);
        RowConverter convert =
            transparency ? transparent_converters[index] : opaque_converters[index];
        Dword output_type = conversion_types[index];
        if (!output_type)
            kernel::error("SHAPE_converttype - UNABLE TO CONVERT TO TARGET SHAPE TYPE.\nInitial "
                          "shape type ($%02x). Target shape type ($%02x)\n",
                          type, target);
        int source_depth = tar::image_depth(image);
        int destination_depth = tar::image_depth(&output_type);
        int source_bits = (source_depth + 1) & ~1;
        int destination_bits = (destination_depth + 1) & ~1;
        int width = static_cast<std::int16_t>(image[1]);
        int height = static_cast<std::int16_t>(image[1] >> 16);
        pitch(image);
        if (source_depth < 9 && destination_depth > 8) {
            if (!palette_count)
                palette_count = 1 << (source_depth & 31);
            active_palette = reinterpret_cast<Byte *>(
                palette_colours(source_palette, reinterpret_cast<Dword *>(palette_scratch)));
            if (target == 109)
                rows::bgra_to_argb4444(palette_scratch, active_palette, palette_count);
            else if (destination_depth == 15) {
                if (transparency)
                    rows::palette_to_argb1555(palette_scratch, active_palette, palette_count);
                else
                    rows::palette_to_argb1555_opaque_binding(palette_scratch, active_palette,
                                                             palette_count);
            } else if (destination_depth == 16) {
                if (transparency)
                    rows::palette_to_rgb565(palette_scratch, active_palette, palette_count);
                else
                    rows::palette_to_rgb565_opaque_binding(palette_scratch, active_palette,
                                                           palette_count);
            }
        }
        size(image);
        int offset = static_cast<int>(*image) >> 8;
        Dword *extra =
            offset ? reinterpret_cast<Dword *>(reinterpret_cast<Byte *>(image) + offset) : nullptr;
        int extra_bytes = attachment_bytes(extra);
        Dword header[4] = {(*image & ~255u) | output_type, image[1], image[2], image[3]};
        if (!mip_mode && had_mips)
            header[3] &= 268435455;
        int output_bytes = size(header);
        char label[32];
        std::sprintf(label, "%dto%d", source_depth, destination_depth);
        output = static_cast<Dword *>(allocate(label, output_bytes + extra_bytes, 0));
        output[0] = output_type;
        output[1] = image[1];
        output[2] = image[2];
        output[3] = image[3];
        if (!mip_mode && had_mips)
            output[3] &= 268435455;
        for (unsigned level = 0; level <= output[3] >> 28; ++level) {
            auto *source_row = reinterpret_cast<Byte *>(tar::image_level(image, 0, 0, level));
            auto *destination_row = reinterpret_cast<Byte *>(tar::image_level(output, 0, 0, level));
            for (int row = 0; row < height; ++row) {
                convert(destination_row, source_row, width);
                source_row += (width * source_bits + 7) >> 3;
                destination_row += (width * destination_bits + 7) >> 3;
            }
            if (width > 1)
                width >>= 1;
            if (height > 1)
                height >>= 1;
        }
        offset = static_cast<int>(*image) >> 8;
        if (offset) {
            copy_attachments(reinterpret_cast<Byte *>(output) + output_bytes,
                             reinterpret_cast<Dword *>(reinterpret_cast<Byte *>(image) + offset));
            *output = (*output & 255) ^ Dword(output_bytes) << 8;
        }
        if (owned)
            release(owned);
        if ((*output & 255) == (target & 255))
            break;
        image = owned = output;
    }
    if (mip_mode == 1 && !had_mips) {
        Dword *mipped = build_mips(output);
        release(output);
        output = mipped;
    }
    if (compressed || target == 96 || target == 97 || target == 98) {
        Dword *packed = pack_shape(output, target == 97 ? 3 : target == 98 ? 5 : 1, 100);
        if (output)
            release_shape(output);
        output = packed;
    }
    type = *output & 255;
    if ((swizzle_mode == 1 || (swizzle_mode == -1 && was_swizzled)) && !(output[3] & 8192) &&
        type != 96 && type != 97 && type != 98 && type != 104) {
        Dword *tiled = swizzle_shape(output);
        if (output)
            release(output);
        output = tiled;
    }
    return output;
}
} // namespace nfs::graphics::textures::shape
