#include "nfs/graphics/textures/texture_conversion.hpp"
#include "nfs/graphics/resources/eagl_assets.hpp"
#include <cstring>
namespace nfs::graphics::textures::shape {

Dword pitch(const Dword *image) {
    int width = static_cast<std::int16_t>(image[1]);
    if ((*image & 255) == 123)
        return width;
    int bits = tar::image_depth(image);
    if (bits == 15)
        bits = 16;
    return static_cast<int>(static_cast<Dword>(bits) * width + 7) >> 3;
}

bool is_palette(Dword type) {
    switch (type) {
    case 32:
    case 33:
    case 34:
    case 36:
    case 41:
    case 42:
    case 44:
    case 45:
    case 46:
    case 48:
    case 49:
    case 50:
    case 51:
    case 56:
        return true;
    default:
        return false;
    }
}
Dword *palette(Dword *image) {
    while (image) {
        if (is_palette(*image & 255))
            return image;
        int step = static_cast<int>(*image) >> 8;
        if (!step)
            break;
        image = reinterpret_cast<Dword *>(reinterpret_cast<Dword>(image) + step);
    }
    return nullptr;
}
static int divide_eight(Dword value) {
    return static_cast<int>(value) / 8;
}
// Preserve signed shifts and divisions, DXT alignment and palette packing.
 
Dword bytes(Dword type, int width, int height, Dword levels, std::uint8_t special) {
    Dword marker = type & 255;
    int bits = tar::image_depth(&marker);
    if (bits == 15)
        bits = 16;
    Dword result = 0;
    if ((type == 32 || type == 33) && special) {
        Dword w = width, remainder = w & 31;
        if (remainder >= 16 && remainder <= 23)
            w = (w & ~31u) + 24;
        else if (remainder > 7 && remainder < 16)
            w += 8;
        result = divide_eight(static_cast<Dword>(bits) * w);
    } else {
        for (Dword level = 0;; ++level) {
            int w = width >> (level & 31), h = height >> (level & 31);
            if (w < 1)
                w = 1;
            if (h < 1)
                h = 1;
            if (type == 96 || type == 97 || type == 98) {
                w = (static_cast<Dword>(w) + 3) & ~3u;
                h = (static_cast<Dword>(h) + 3) & ~3u;
            }
            result += static_cast<Dword>(divide_eight(static_cast<Dword>(w) * bits + 7)) * h;
            if (level == levels)
                break;
        }
    }
    return result ? result + 16 : 0;
}
// RefPack header/block traversal returns a byte count and optional encoded
// length.
int compressed_size(const std::uint8_t *begin, Dword *encoded) {
    if (!begin)
        return 0;
    const auto *cursor = begin + (((Dword(begin[0]) * 256 + begin[1]) & 256) ? 5 : 2);
    int decoded = (Dword(cursor[0]) * 256 + cursor[1]) * 256 + cursor[2];
    cursor += 3;
    if (encoded) {
        for (;;) {
            Dword tag = *cursor;
            const auto *after = cursor + 1;
            if (tag < 128) {
                cursor += (tag & 3) + 2;
                continue;
            }
            if (!(tag & 64)) {
                cursor += (*after >> 6) + 3;
                continue;
            }
            if (!(tag & 32)) {
                cursor += (tag & 3) + 4;
                continue;
            }
            Dword length = (tag & 31) * 4 + 4;
            if (length > 112) {
                *encoded = static_cast<Dword>(after - begin) + (tag & 3);
                break;
            }
            cursor = after + length;
        }
    }
    return decoded;
}

Dword size(const Dword *image) {
    Dword type = *image & 255;
    switch (type) {
    case 105:
        return (reinterpret_cast<const std::uint8_t *>(image)[6] & 16)
                   ? (Dword(static_cast<std::uint16_t>(image[1]))
                          ? Dword(static_cast<std::uint16_t>(image[1]))
                          : 64) +
                         16
                   : 16;
    case 111:
        return image[1] + 8;
    case 124:
        return image[1] * 8 + 8;
    case 112:
        return static_cast<Dword>(std::strlen(reinterpret_cast<const char *>(image + 1))) + 5;
    default:
        break;
    }
    if (static_cast<std::int8_t>(*image) < 0) {
        Dword encoded = 0;
        const auto *pixels = reinterpret_cast<const std::uint8_t *>(
            (image[3] & 4096) ? reinterpret_cast<Dword>(image) + image[4]
                                : reinterpret_cast<Dword>(image + 4));
        return compressed_size(pixels, &encoded) ? encoded + 16 : 0;
    }
    return bytes(type, static_cast<std::int16_t>(image[1]),
                 static_cast<std::int16_t>(image[1] >> 16), image[3] >> 28,
                 static_cast<std::uint8_t>((image[3] >> 13) & 4294967041u));
}
} // namespace nfs::graphics::textures::shape
