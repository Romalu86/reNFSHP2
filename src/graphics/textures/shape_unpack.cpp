#include "nfs/graphics/textures/shape_conversion.hpp"
#include "nfs/graphics/resources/eagl_assets.hpp"
#include "nfs/core/memory/memory_primitives.hpp"

namespace nfs::graphics::textures::shape {

Dword *unpack_shape(Dword *image) {
    if (!image)
        return nullptr;
    int offset = static_cast<int>(*image) >> 8;
    Dword *extra =
        offset ? reinterpret_cast<Dword *>(reinterpret_cast<Byte *>(image) + offset) : nullptr;
    int extra_bytes = attachment_bytes(extra);
    Dword header[4] = {image[0], image[1], image[2], image[3]};
    Dword *output;
    int output_bytes;
    if (static_cast<std::int8_t>(*image) < 0) {
        header[0] &= ~128u;
        output_bytes = size(header);
        output = static_cast<Dword *>(allocate("unShp", output_bytes + extra_bytes, 0));
        kernel::memory_copy(output, image, 16);
        unpack_buffer(reinterpret_cast<Byte *>(image + 4), output + 4);
        *reinterpret_cast<Byte *>(output) &= 127;
    } else {
        Dword type = *image & 255;
        if (type != 96 && type != 97 && type != 98)
            return copy_shape(image);
        int format = type == 96 ? 1 : type == 97 ? 3 : 5;
        int width = static_cast<std::int16_t>(image[1]);
        int height = static_cast<std::int16_t>(image[1] >> 16);
        if (width % 4)
            width += 4 - width % 4;
        if (height % 4)
            height += 4 - height % 4;
        header[0] = (header[0] & ~255u) | 125;
        header[1] = std::uint16_t(width) | Dword(std::uint16_t(height)) << 16;
        output_bytes = size(header);
        output = static_cast<Dword *>(allocate("undxt", output_bytes + extra_bytes, 0));
        kernel::memory_copy(output, image, 16);
        output[1] = header[1];
        *reinterpret_cast<Byte *>(output) = 125;
        for (unsigned level = 0; level <= output[3] >> 28; ++level) {
            decode_dxt_image(tar::image_level(image, 0, 0, level),
                             tar::image_level(output, 0, 0, level), width, height, format);
            if (width > 1)
                width >>= 1;
            if (height > 1)
                height >>= 1;
        }
    }
    if (extra_bytes) {
        *output = (*output & 255) ^ Dword(output_bytes) << 8;
        offset = static_cast<int>(*image) >> 8;
        copy_attachments(reinterpret_cast<Byte *>(output) + output_bytes,
                         offset ? reinterpret_cast<Dword *>(reinterpret_cast<Byte *>(image) + offset)
                                : nullptr);
    }
    return output;
}
} // namespace nfs::graphics::textures::shape
