#include "nfs/graphics/textures/shape_conversion.hpp"
#include "nfs/graphics/resources/eagl_assets.hpp"
#include <cstdio>
#include <cstring>

namespace nfs::graphics::textures::shape {

int normalized_depth(int depth) {
    Dword header = type_from_depth(depth == 0 ? 32 : depth);
    return static_cast<int>(tar::image_depth(&header));
}

int mip_storage_bytes(int width, int height, int depth, int levels) {
    const int normalized = normalized_depth(depth);
    const int bits = normalized == 15 ? 16 : normalized;
    int total = 0;
    if (width <= 0 || width > 65536 || height <= 0 || height > 65536 ||
        static_cast<int>(Dword(width) * Dword(height)) >= 1000000001 ||
        type_from_depth(normalized) == 0)
        return 0;
    for (int level = 0; level <= levels; ++level) {
        int w = width >> (level & 31), h = height >> (level & 31);
        if (w < 1)
            w = 1;
        if (h < 1)
            h = 1;
        total = static_cast<int>(Dword(total) + Dword((w * bits + 7) / 8) * Dword(h));
        if (w == 1 && h == 1)
            break;
    }
    return total;
}

int palette_entries(int depth) {
    return depth == 4 ? 16 : depth == 8 ? 256 : 0;
}

Dword palette_type_from_depth(int depth) {
    switch (depth) {
    case 15:
        return 45;
    case 16:
        return 41;
    case 18:
        return 34;
    case 24:
        return 36;
    case 32:
        return 42;
    default:
        return 0;
    }
}

int allocation_bytes(int width, int height, int depth, int palette_depth, int levels,
                     int extra_bytes, int private_bytes) {
    const int normalized = normalized_depth(depth);
    const int pixels = mip_storage_bytes(width, height, depth, levels);
    Dword total = Dword(pixels) + 16;
    if (normalized < 9 && palette_depth != 0) {
        if (total != 0)
            total = (Dword(pixels) + 31) & ~Dword(15);
        const int palette_bits = palette_entries(normalized) * palette_depth;
        total = Dword(palette_bits / 8) + 16 + ((total + 15) & ~Dword(15));
    }
    if (extra_bytes != 0)
        total += 8 + Dword(extra_bytes);
    if (private_bytes != 0)
        total += 16 + Dword(private_bytes);
    return static_cast<int>(total);
}

void initialize_shape(Dword *image, int width, int height, int depth, int palette_depth, int levels,
                      Dword extra_bytes, int private_bytes) {
    if (depth == 0)
        depth = 32;
    const Byte type = static_cast<Byte>(type_from_depth(depth));
    std::memset(image, 0, 16);
    reinterpret_cast<Byte *>(image)[0] = type;
    reinterpret_cast<std::uint16_t *>(image)[2] = static_cast<std::uint16_t>(width);
    reinterpret_cast<std::uint16_t *>(image)[3] = static_cast<std::uint16_t>(height);
    image[3] = (image[3] & 268435455) ^ (Dword(levels) << 28);
    Byte *next =
        reinterpret_cast<Byte *>(image) + 16 + mip_storage_bytes(width, height, depth, levels);
    Dword *previous = image;
    if (depth < 9 && palette_depth != 0) {
        const int count = palette_entries(depth);
        const Byte palette_type = static_cast<Byte>(palette_type_from_depth(palette_depth));
        const int bits = palette_depth == 15 ? 16 : palette_depth;
        const Dword offset = (Dword(next - reinterpret_cast<Byte *>(image)) + 15) & ~Dword(15);
        Dword *palette_header = reinterpret_cast<Dword *>(reinterpret_cast<Byte *>(image) + offset);
        image[0] = (offset << 8) ^ (image[0] & 255);
        std::memset(palette_header, 0, 16);
        reinterpret_cast<Byte *>(palette_header)[0] = palette_type;
        reinterpret_cast<std::uint16_t *>(palette_header)[2] = static_cast<std::uint16_t>(count);
        reinterpret_cast<std::uint16_t *>(palette_header)[3] = 1;
        Byte *colours = reinterpret_cast<Byte *>(palette_header + 4);
        if (palette_depth < 32) {
            std::memset(colours, 255, (bits * count) / 8);
        } else {
            for (int i = 0; i < count; ++i) {
                colours[4 * i] = colours[4 * i + 1] = colours[4 * i + 2] = static_cast<Byte>(i);
                colours[4 * i + 3] = 255;
            }
        }
        next = reinterpret_cast<Byte *>(palette_header) + 16 + (bits * count) / 8;
        previous = palette_header;
    }
    if (extra_bytes != 0) {
        previous[0] = (Dword(next - reinterpret_cast<Byte *>(previous)) << 8) ^ (previous[0] & 255);
        std::memset(next, 0, extra_bytes + 8);
        next[0] = 111;
        reinterpret_cast<Dword *>(next)[1] = extra_bytes;
        previous = reinterpret_cast<Dword *>(next);
        next += extra_bytes + 8;
    }
    if (private_bytes != 0) {
        previous[0] = (Dword(next - reinterpret_cast<Byte *>(previous)) << 8) ^ (previous[0] & 255);
        std::memset(next, 0, Dword(private_bytes) + 16);
        next[0] = 105;
        reinterpret_cast<std::uint16_t *>(next)[3] = 16;
    }
}

Dword *allocate_shape(int width, int height, int depth, int palette_depth, int levels, Dword pool,
                     int extra_bytes, int private_bytes) {
    const int size =
        allocation_bytes(width, height, depth, palette_depth, levels, extra_bytes, private_bytes);
    if (size == 0)
        return nullptr;
    char label[16];
    std::sprintf(label, "SHP%dx%dx%d", width, height, depth);
    auto *image = static_cast<Dword *>(allocate_aligned(label, size, 0, 16, pool));
    if (image != nullptr)
        initialize_shape(image, width, height, depth, palette_depth, levels, extra_bytes,
                         private_bytes);
    return image;
}
} // namespace nfs::graphics::textures::shape
