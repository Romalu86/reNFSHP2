#include "nfs/graphics/textures/shape_conversion.hpp"

namespace nfs::graphics::textures::shape {


Byte palette_scratch[1024] = {};
Byte default_palette[1024] = {};
Byte *active_palette = default_palette;
Byte *colour_index_lookup = nullptr;

Allocate allocate = kernel::memory_allocate;
AllocateAligned allocate_aligned = kernel::memory_allocate_aligned;
Release release = kernel::memory_free;


Dword pack_visible565(Dword colour) {
    if (!(colour & 4278190080u))
        return 0;
    return (((colour >> 19 & 31) << 6 | (colour >> 10 & 63)) << 5) | (colour >> 3 & 31);
}


Dword pack_alpha1555(Dword colour) {
    if (!(colour & 4278190080u))
        return 0;
    return (((colour >> 19 & 31) << 5 | (colour >> 11 & 31)) << 5) |
           ((colour & 248 | 262144) >> 3);
}


void make_alpha_palette(void *destination) {
    auto *output = static_cast<Byte *>(destination);
    for (unsigned entry = 0; entry < 256; ++entry) {
        output[entry * 4] = output[entry * 4 + 1] = output[entry * 4 + 2] = 255;
        output[entry * 4 + 3] = static_cast<Byte>(entry);
    }
}

// Six-bit palette channels are signed bytes in these instructions.
void palette_rgb6(int count, const void *source, void *destination) {
    const auto *input = static_cast<const std::int8_t *>(source);
    auto *output = static_cast<Byte *>(destination);
    for (int entry = 0; entry < count; ++entry) {
        output[3] = entry == 255 ? 0 : 255;
        for (int channel = 0; channel < 3; ++channel)
            output[2 - channel] = static_cast<Byte>((int(input[channel]) * 255 + 32) / 63);
        input += 3;
        output += 4;
    }
}


void palette_rgb8(int count, const void *source, void *destination) {
    const auto *input = static_cast<const Byte *>(source);
    auto *output = static_cast<Byte *>(destination);
    for (int entry = 0; entry < count; ++entry) {
        output[3] = entry == 255 ? 0 : 255;
        output[2] = input[0];
        output[1] = input[1];
        output[0] = input[2];
        input += 3;
        output += 4;
    }
}


Dword *palette_colours(Dword *image, Dword *scratch) {
    Dword *chunk = palette(image);
    if (!chunk) {
        if (image && (*image & 255) == 100) {
            make_alpha_palette(scratch);
            return scratch;
        }
        return reinterpret_cast<Dword *>(default_palette);
    }
    const void *pixels =
        chunk[3] & 4096 ? reinterpret_cast<const void *>(reinterpret_cast<Dword>(chunk) + chunk[4])
                          : chunk + 4;
    int count = static_cast<std::int16_t>(chunk[1]);
    switch (*chunk & 255) {
    case 42:
        return const_cast<Dword *>(static_cast<const Dword *>(pixels));
    case 36:
        scratch[255] = 0;
        palette_rgb8(count, pixels, scratch);
        break;
    case 41:
        rows::rgb565_to_bgra(scratch, pixels, count);
        break;
    case 45:
        rows::argb1555_to_bgra(scratch, pixels, count);
        break;
    case 44:
        rows::alpha565_to_bgra(scratch, pixels, count);
        break;
    case 46:
        rows::alpha1555_to_bgra(scratch, pixels, count);
        break;
    case 34:
        palette_rgb6(count, pixels, scratch);
        break;
    }
    return scratch;
}

} // namespace nfs::graphics::textures::shape
