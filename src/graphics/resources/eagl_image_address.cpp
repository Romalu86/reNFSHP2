#include "nfs/graphics/resources/eagl_assets.hpp"
#include <cstring>
namespace nfs::graphics::tar {
#include "eagl_image_tables.inc"
static int exponent(int value) {
    float converted = static_cast<float>(value);
    Dword bits;
    std::memcpy(&bits, &converted, 4);
    return (static_cast<int>(bits) >> 23) - 127;
}
// Morton lookup, including the asymmetric high-byte conditions.
Dword swizzled_offset(Dword x, Dword y, int width, int height, int bits) {
    int a = exponent(width), b = exponent(height);
    Dword offset, first, second;
    if (a < b) {
        first = swizzle_table[x & 255];
        if ((x >> 8) & 255)
            first |= Dword(swizzle_table[static_cast<int>(x) >> 8]) << 16;
        Dword masked = (width - 1u) & y;
        second = swizzle_table[masked & 255];
        if (((y >> 8) & 255) && masked)
            second |= Dword(swizzle_table[static_cast<int>(y) >> 8]) << 16;
        offset = (static_cast<Dword>(static_cast<int>(y) >> (a & 31)) << ((a * 2) & 31)) |
                 second * 2 | first;
    } else {
        second = swizzle_table[y & 255];
        if ((y >> 8) & 255)
            second |= Dword(swizzle_table[static_cast<int>(y) >> 8]) << 16;
        Dword masked = (height - 1u) & x;
        first = swizzle_table[masked & 255];
        if (((x >> 8) & 255) && masked)
            first |= Dword(swizzle_table[static_cast<int>(x) >> 8]) << 16;
        offset = (static_cast<Dword>(static_cast<int>(x) >> (b & 31)) << ((b * 2) & 31)) |
                 second * 2 | first;
    }
    switch (bits) {
    case 4:
        return offset << 2;
    case 8:
        return offset << 3;
    case 15:
    case 16:
        return offset << 4;
    case 32:
        return offset << 5;
    default:
        return offset;
    }
}

Dword *image_level(Dword *image, int x, int y, int level) {
    Dword pixels = (image[3] & 4096) ? reinterpret_cast<Dword>(image) + image[4]
                                      : reinterpret_cast<Dword>(image + 4);
    int width = static_cast<std::int16_t>(image[1]),
        height = static_cast<std::int16_t>(image[1] >> 16), bits = image_depth(image);
    if (bits == 15)
        bits = 16;
    if ((*image & 255) == 96) {
        width = (width + 3) >> 2;
        height = (height + 3) >> 2;
        bits = 64;
    }
    Dword accumulated = 0;
    for (int i = 0; i < level; ++i) {
        accumulated += static_cast<Dword>(bits) * height * width;
        if (width > 1)
            width >>= 1;
        if (height > 1)
            height >>= 1;
    }
    if ((*image & 255) == 96) {
        x = (x + ((x >> 31) & 3)) >> 2;
        y = (y + ((y >> 31) & 3)) >> 2;
    }
    Dword offset = (image[3] & 8192)
                      ? swizzled_offset(x, y, width, height, bits)
                      : ((static_cast<Dword>(bits) * width + 7) & ~7u) * y + bits * x;
    return reinterpret_cast<Dword *>(pixels + (static_cast<int>(offset + accumulated) >> 3));
}
} // namespace nfs::graphics::tar
