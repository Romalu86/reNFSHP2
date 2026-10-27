#include "nfs/graphics/textures/shape_conversion.hpp"
#include "nfs/core/memory/memory_primitives.hpp"
#include <cstring>

namespace nfs::graphics::textures::shape {
namespace {
Byte *pixels(Dword *image) {
    return image[3] & 4096 ? reinterpret_cast<Byte *>(image) + image[4]
                             : reinterpret_cast<Byte *>(image + 4);
}
Dword *attachments(Dword *image) {
    int offset = static_cast<int>(*image) >> 8;
    return offset ? reinterpret_cast<Dword *>(reinterpret_cast<Byte *>(image) + offset) : nullptr;
}
int mip_count(Dword *image) {
    int width = static_cast<std::int16_t>(image[1]);
    int height = static_cast<std::int16_t>(image[1] >> 16);
    float minimum = static_cast<float>(height <= width ? height : width);
    int bits;
    std::memcpy(&bits, &minimum, 4);
    return (bits >> 23) - 127;
}
std::uint16_t average16(const std::uint16_t *input, int width, int kind) {
    Dword a = input[0], b = input[1], c = input[width], d = input[width + 1];
    if (kind == 7) {
        Dword red = ((a >> 11) + (b >> 11) + (c >> 11) + (d >> 11)) >> 2;
        Dword green = ((a >> 5 & 63) + (b >> 5 & 63) + (c >> 5 & 63) + (d >> 5 & 63)) >> 2;
        Dword blue = ((a & 31) + (b & 31) + (c & 31) + (d & 31)) >> 2;
        return static_cast<std::uint16_t>(red << 11 | green << 5 | blue);
    }
    if (kind == 8) {
        Dword low = ((a & 255) + (b & 255) + (c & 255) + (d & 255)) >> 2;
        Dword high = ((a >> 8) + (b >> 8) + (c >> 8) + (d >> 8)) >> 2;
        return static_cast<std::uint16_t>(high << 8 ^ low);
    }
    Dword mask = kind == 3 ? 31775 : 3855;
    unsigned shift = kind == 3 ? 1 : 4;
    Dword upper_mask = kind == 3 ? 16880 : 3855;
    Dword upper = ((((a >> shift) & upper_mask) + ((b >> shift) & upper_mask) +
                   ((c >> shift) & upper_mask) + ((d >> shift) & upper_mask)) >>
                  2)
                 << shift;
    Dword lower = ((a & mask) + (b & mask) + (c & mask) + (d & mask)) >> 2;
    return static_cast<std::uint16_t>(((lower ^ upper) & mask) ^ upper);
}
} // namespace

// Stored mip levels are copied; otherwise use the format's
// exact sample/average rule, including its odd-width source cursor adjustment.
Dword *half_size(Dword *image) {
    if (!image)
        return nullptr;
    int kind = 0;
    switch (*image & 127) {
    case 121:
        kind = 1;
        break;
    case 123:
        kind = 2;
        break;
    case 126:
        kind = 3;
        break;
    case 102:
    case 127:
        kind = 4;
        break;
    case 125:
        kind = 5;
        break;
    case 109:
        kind = 6;
        break;
    case 120:
        kind = 7;
        break;
    case 101:
        kind = 8;
        break;
    default:
        if (!(image[3] & 4026531840u))
            return nullptr;
    }
    int extra_bytes = attachment_bytes(attachments(image));
    Dword header[4] = {image[0], image[1], image[2], image[3]};
    if (header[3] & 4026531840u)
        header[3] -= 268435456;
    int width = static_cast<std::int16_t>(image[1]) / 2;
    int height = static_cast<std::int16_t>(image[1] >> 16) / 2;
    header[1] = std::uint16_t(width) | Dword(std::uint16_t(height)) << 16;
    int output_bytes = size(header);
    bool was_swizzled = (image[3] & 8192) && !(image[3] & 4026531840u);
    if (was_swizzled)
        image = unswizzle_shape(image);
    auto *output =
        static_cast<Dword *>(allocate_aligned("halfsized", output_bytes + extra_bytes, 0, 16, 0));
    kernel::memory_copy(output, image, 16);
    output[1] = header[1];
    if (image[3] & 4026531840u) {
        Dword base_header[4] = {image[0], image[1], image[2], image[3] & 268435455};
        output[3] -= 268435456;
        kernel::memory_copy(output + 4, reinterpret_cast<Byte *>(image) + size(base_header),
                            output_bytes - 16);
    } else {
        int source_width = static_cast<std::int16_t>(image[1]);
        int even_width = source_width + (source_width & 1);
        const Byte *input = pixels(image);
        Byte *destination = pixels(output);
        for (int y = 0; y < height; ++y) {
            if (kind == 1) {
                int row_bytes = (width + (width & 1)) >> 1;
                for (int x = 0; x < row_bytes; ++x) {
                    *destination++ = Byte((input[0] & 15) ^ input[1] << 4);
                    input += 2;
                }
                input += -(width & 1) + (source_width & 1) + even_width / 2;
                continue;
            }
            int stride = kind == 2 ? 1 : kind == 4 ? 3 : kind == 5 ? 4 : 2;
            for (int x = 0; x < width; ++x) {
                if (kind == 2)
                    *destination = *input;
                else if (kind == 4)
                    kernel::memory_copy(destination, input, 3);
                else if (kind == 5) {
                    const auto *words = reinterpret_cast<const Dword *>(input);
                    Dword a = words[0], b = words[1], c = words[source_width],
                         d = words[source_width + 1];
                    Dword a2 = a >> 2, b2 = b >> 2, c2 = c >> 2, d2 = d >> 2;
                    Dword low =
                        (((a & 16711935) + (b & 16711935) + (c & 16711935) + (d & 16711935)) >> 2) &
                        16711935;
                    Dword carry = ((d2 & 4290789312u) + (a2 & 4290789312u) + (b2 & 4290789312u) +
                                  (c2 & 4290789312u)) &
                                 16711935;
                    *reinterpret_cast<Dword *>(destination) =
                        low ^ carry ^
                        ((a2 & 1069563840) + (b2 & 1069563840) + (c2 & 1069563840) +
                         (d2 & 1069563840));
                } else if (kind) {
                    *reinterpret_cast<std::uint16_t *>(destination) = average16(
                        reinterpret_cast<const std::uint16_t *>(input), source_width, kind);
                }
                input += stride * 2;
                destination += stride;
            }
            input += even_width * stride;
        }
    }
    if (extra_bytes) {
        copy_attachments(reinterpret_cast<Byte *>(output) + output_bytes, attachments(image));
        *output = (*output & 255) ^ Dword(output_bytes) << 8;
    }
    if (was_swizzled) {
        Dword *tiled = swizzle_shape(output);
        release(output);
        release(image);
        return tiled;
    }
    return output;
}

// The first copy uses the requested output length; callers must provide readable source padding
// when adding mip levels.
Dword *build_mips(Dword *image, int levels) {
    if (!image)
        return nullptr;
    int maximum = mip_count(image);
    if (maximum <= levels)
        levels = maximum;
    int base_bytes = size(image);
    int extra_bytes = attachment_bytes(attachments(image));
    Dword header[4] = {image[0], image[1], image[2], (image[3] & 268435455) ^ Dword(levels) << 28};
    int output_bytes = size(header);
    bool was_swizzled = (image[3] & 8192) != 0;
    Dword *linear = was_swizzled ? unswizzle_shape(image) : image;
    auto *output =
        static_cast<Dword *>(allocate_aligned("mipped", output_bytes + extra_bytes, 0, 16, 0));
    kernel::memory_copy(output, linear, output_bytes);
    output[3] = (output[3] & 268435455) ^ Dword(levels) << 28;
    Byte *cursor = reinterpret_cast<Byte *>(output) + base_bytes;
    Dword *previous = linear;
    Dword *owned = nullptr;
    while (levels-- > 0) {
        Dword *next = half_size(previous);
        if (owned)
            release(owned);
        if (!next) {
            release(output);
            return nullptr;
        }
        int length = size(next) - 16;
        kernel::memory_copy(cursor, pixels(next), length);
        cursor += length;
        previous = owned = next;
    }
    if (owned)
        release(owned);
    if (extra_bytes) {
        copy_attachments(reinterpret_cast<Byte *>(output) + output_bytes, attachments(linear));
        *output = (*output & 255) ^ Dword(output_bytes) << 8;
    }
    if (was_swizzled) {
        Dword *tiled = swizzle_shape(output);
        release(output);
        if (linear)
            release(linear);
        return tiled;
    }
    return output;
}


Dword *build_mips(Dword *image) {
    return image ? build_mips(image, mip_count(image)) : nullptr;
}
} // namespace nfs::graphics::textures::shape
