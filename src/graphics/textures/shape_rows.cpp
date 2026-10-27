#include "nfs/graphics/textures/shape_conversion.hpp"
#include "nfs/core/memory/memory_primitives.hpp"
#include <cstring>

namespace nfs::graphics::textures::shape::rows {
namespace {

template <class Convert>
void words16(void *destination, const void *source, int count, Convert convert) {
    auto *output = static_cast<std::uint16_t *>(destination);
    const auto *input = static_cast<const std::uint16_t *>(source);
    for (int pixel = 0; pixel < count; ++pixel)
        output[pixel] = convert(input[pixel]);
}

template <class Convert>
void words16_to_32(void *destination, const void *source, int count, Convert convert) {
    auto *output = static_cast<Dword *>(destination);
    const auto *input = static_cast<const std::uint16_t *>(source);
    for (int pixel = 0; pixel < count; ++pixel)
        output[pixel] = convert(input[pixel]);
}

template <class Convert>
void words32(void *destination, const void *source, int count, Convert convert) {
    auto *output = static_cast<Dword *>(destination);
    const auto *input = static_cast<const Dword *>(source);
    for (int pixel = 0; pixel < count; ++pixel)
        output[pixel] = convert(input[pixel]);
}

} // namespace


void copy_indices4(void *out, const void *in, int count) {
    kernel::memory_copy(out, in, (count + 1) >> 1);
}
void copy_indices8(void *out, const void *in, int count) {
    kernel::memory_copy(out, in, count);
}
void copy_word16(void *out, const void *in, int count) {
    kernel::memory_copy(out, in, Dword(count) * 2);
}
void copy_bgr(void *out, const void *in, int count) {
    kernel::memory_copy(out, in, Dword(count) * 3);
}
void copy_bgra(void *out, const void *in, int count) {
    kernel::memory_copy(out, in, Dword(count) * 4);
}


void pack_indices4(void *destination, const void *source, int count) {
    auto *output = static_cast<Byte *>(destination);
    const auto *input = static_cast<const Byte *>(source);
    for (int pixel = 0; pixel < count; ++pixel) {
        Byte &packed = output[pixel >> 1];
        packed = pixel & 1 ? Byte(input[pixel] << 4 | (packed & 15))
                           : Byte((input[pixel] & 15) ^ (packed & 240));
    }
}


void swap_nibbles(void *destination, const void *source, int count) {
    auto *output = static_cast<Byte *>(destination);
    const auto *input = static_cast<const Byte *>(source);
    if (count > 0)
        for (int pixel = 0; pixel < (count + 1) / 2; ++pixel) {
            Byte value = input[pixel];
            output[pixel] = Byte(value >> 4 | value << 4);
        }
}


void expand_indices4(void *destination, const void *source, int count) {
    auto *output = static_cast<Byte *>(destination);
    const auto *input = static_cast<const Byte *>(source);
    for (int pixel = 0; pixel < count; ++pixel)
        output[pixel] = pixel & 1 ? input[pixel >> 1] >> 4 : input[pixel >> 1] & 15;
}

// The 32768-entry index lookup has a separate producer.
void lookup_565(void *destination, const void *source, int count) {
    auto *output = static_cast<Byte *>(destination);
    const auto *input = static_cast<const std::uint16_t *>(source);
    for (int pixel = 0; pixel < count; ++pixel) {
        Dword value = input[pixel];
        Dword index = (((value >> 6 & 31) | (value >> 11) << 5) << 5) | (value & 31);
        output[pixel] = colour_index_lookup[index];
    }
}


void rgb565_to_argb1555(void *out, const void *in, int count) {
    words16(out, in, count,
            [](Dword value) { return (value >> 1 & 65504) | (value & 31) | 32768; });
}
void rgb565_to_xrgb1555(void *out, const void *in, int count) {
    words16(out, in, count, [](Dword value) { return (value >> 1 & 32736) | (value & 31); });
}

// the two bodies call the same alpha-aware helper.
void palette_to_argb1555(void *destination, const void *source, int count) {
    auto *output = static_cast<std::uint16_t *>(destination);
    const auto *input = static_cast<const Dword *>(source);
    for (int pixel = 0; pixel < count; ++pixel)
        output[pixel] = static_cast<std::uint16_t>(pack_alpha1555(input[pixel]));
}
void palette_to_argb1555_opaque_binding(void *out, const void *in, int count) {
    palette_to_argb1555(out, in, count);
}


void indices4_to_word16(void *destination, const void *source, int count) {
    auto *output = static_cast<std::uint16_t *>(destination);
    const auto *input = static_cast<const Byte *>(source);
    const auto *entries = reinterpret_cast<const std::uint16_t *>(palette_scratch);
    for (int pixel = 0; pixel < count; ++pixel)
        output[pixel] = entries[pixel & 1 ? input[pixel >> 1] >> 4 : input[pixel >> 1] & 15];
}
void indices8_to_word16(void *destination, const void *source, int count) {
    auto *output = static_cast<std::uint16_t *>(destination);
    const auto *input = static_cast<const Byte *>(source);
    const auto *entries = reinterpret_cast<const std::uint16_t *>(palette_scratch);
    for (int pixel = 0; pixel < count; ++pixel)
        output[pixel] = entries[input[pixel]];
}


void argb1555_to_rgb565(void *out, const void *in, int count) {
    words16(out, in, count, [](Dword value) {
        Dword green = value >> 5 & 31;
        return ((((green >> 4 | green << 1) << 4) | (value & 64512)) << 1) | (value & 31);
    });
}


void palette_to_rgb565(void *destination, const void *source, int count) {
    auto *output = static_cast<std::uint16_t *>(destination);
    const auto *input = static_cast<const Dword *>(source);
    for (int pixel = 0; pixel < count; ++pixel)
        output[pixel] = static_cast<std::uint16_t>(pack_visible565(input[pixel]));
}
void palette_to_rgb565_opaque_binding(void *out, const void *in, int count) {
    palette_to_rgb565(out, in, count);
}


void bgra_to_argb4444(void *destination, const void *source, int count) {
    auto *output = static_cast<std::uint16_t *>(destination);
    const auto *input = static_cast<const Byte *>(source);
    for (int pixel = 0; pixel < count; ++pixel, input += 4)
        output[pixel] = std::uint16_t(
            (((input[3] >> 4) << 4 | input[2] >> 4) << 4 | input[1] >> 4) << 4 | input[0] >> 4);
}


void argb1555_to_argb4444(void *out, const void *in, int count) {
    words16(out, in, count, [](Dword value) {
        Dword rgb = ((((value >> 1 & 15360) | (value & 960)) >> 1) | (value & 30)) >> 1;
        return rgb | (value >> 15) * 61440;
    });
}
void rgb565_to_argb4444(void *out, const void *in, int count) {
    words16(out, in, count, [](Dword value) {
        return ((((value >> 1 & 63495) | (value & 1920)) >> 2 | (value & 30)) >> 1) | 61440;
    });
}


void bgra_high_word16(void *destination, const void *source, int count) {
    auto *output = static_cast<std::uint16_t *>(destination);
    const auto *input = static_cast<const std::uint16_t *>(source) + 1;
    for (int pixel = 0; pixel < count; ++pixel)
        output[pixel] = input[pixel * 2];
}


void indices8_to_bgr(void *destination, const void *source, int count) {
    auto *output = static_cast<Byte *>(destination);
    const auto *input = static_cast<const Byte *>(source);
    for (int pixel = 0; pixel < count; ++pixel, output += 3) {
        Dword index = input[pixel];
        output[2] = active_palette[index * 4 + 2];
        output[1] = active_palette[index * 4 + 1];
        output[0] = active_palette[index * 4];
    }
}


void rgb565_to_bgr(void *destination, const void *source, int count) {
    auto *output = static_cast<Byte *>(destination);
    const auto *input = static_cast<const std::uint16_t *>(source);
    for (int pixel = 0; pixel < count; ++pixel, output += 3) {
        Dword value = input[pixel], green = value >> 5 & 63;
        output[0] = Byte((value & 31) >> 2 | (value & 31) << 3);
        output[1] = Byte(green >> 4 | green << 2);
        output[2] = Byte((value >> 11) >> 2 | (value >> 8 & 248));
    }
}


void bgra_to_bgr(void *destination, const void *source, int count) {
    auto *output = static_cast<Byte *>(destination);
    const auto *input = static_cast<const Byte *>(source);
    for (int pixel = 0; pixel < count; ++pixel, output += 3, input += 4) {
        Byte green = input[1], red = input[2];
        output[0] = input[0];
        output[1] = green;
        output[2] = red;
    }
}


void bgra_to_argb6666(void *destination, const void *source, int count) {
    auto *output = static_cast<Byte *>(destination);
    const auto *input = static_cast<const Byte *>(source);
    for (int pixel = 0; pixel < count; ++pixel, output += 3, input += 4) {
        Dword value =
            ((((Dword(input[3]) & ~3u) << 6 | (input[2] & ~3u)) << 6 | (input[1] & ~3u)) << 4);
        output[0] = Byte(value) | input[0] >> 2;
        output[1] = Byte(value >> 8);
        output[2] = Byte(value >> 16);
    }
}
void bgra_to_argb2101010(void *destination, const void *source, int count) {
    auto *output = static_cast<Dword *>(destination);
    const auto *input = static_cast<const Byte *>(source);
    for (int pixel = 0; pixel < count; ++pixel, input += 4)
        output[pixel] =
            (((((Dword(input[3]) & ~63u) << 2 | input[2]) << 10 | input[1]) << 10) | input[0]) << 2;
}


void indices4_to_bgra(void *destination, const void *source, int count) {
    auto *output = static_cast<Dword *>(destination);
    const auto *input = static_cast<const Byte *>(source);
    for (int pixel = 0; pixel < count / 2; ++pixel) {
        auto *entries = reinterpret_cast<const Dword *>(active_palette);
        output[pixel * 2] = entries[input[pixel] & 15];
        entries = reinterpret_cast<const Dword *>(active_palette);
        output[pixel * 2 + 1] = entries[input[pixel] >> 4];
    }
    if (count & 1)
        output[count - 1] = reinterpret_cast<const Dword *>(active_palette)[input[count / 2] & 15];
}
void indices8_to_bgra(void *destination, const void *source, int count) {
    auto *output = static_cast<Dword *>(destination);
    const auto *input = static_cast<const Byte *>(source);
    for (int pixel = 0; pixel < count; ++pixel)
        output[pixel] = reinterpret_cast<const Dword *>(active_palette)[input[pixel]];
}


void argb1555_to_bgra(void *out, const void *in, int count) {
    words16_to_32(out, in, count, [](Dword value) {
        Dword red = value >> 10 & 31, green = value >> 5 & 31, blue = value & 31;
        Dword red8 = red << 3 | red >> 2;
        Dword green8 = green << 3 | green >> 2;
        Dword blue8 = blue << 3 | blue >> 2;
        return (value & 32768 ? 4278190080u : 0) | red8 << 16 | green8 << 8 | blue8;
    });
}


void rgb565_to_bgra(void *out, const void *in, int count) {
    words16_to_32(out, in, count, [](Dword value) {
        Dword red = value >> 11, green = value >> 5 & 63, blue = value & 31;
        Dword red8 = red << 3 | red >> 2;
        Dword green8 = green << 2 | green >> 4;
        Dword blue8 = blue << 3 | blue >> 2;
        return 4278190080u | red8 << 16 | green8 << 8 | blue8;
    });
}


void argb4444_to_bgra(void *out, const void *in, int count) {
    words16_to_32(out, in, count, [](Dword value) {
        return ((value >> 12) * 17 << 24) | ((value >> 8 & 15) * 17 << 16) |
               ((value >> 4 & 15) * 17 << 8) | (value & 15) * 17;
    });
}


void alpha1555_to_bgra(void *out, const void *in, int count) {
    words32(out, in, count, [](Dword value) {
        Dword red = value >> 10 & 31, green = value >> 5 & 31, blue = value & 31;
        return (value & 4278190080u) | (red << 3 | red >> 2) << 16 | (green << 3 | green >> 2) << 8 |
               (blue << 3 | blue >> 2);
    });
}
void alpha565_to_bgra(void *out, const void *in, int count) {
    words32(out, in, count, [](Dword value) {
        Dword red = value >> 11 & 31, green = value >> 5 & 63, blue = value & 31;
        return (value & 4278190080u) | (red << 3 | red >> 2) << 16 | (green << 2 | green >> 4) << 8 |
               (blue << 3 | blue >> 2);
    });
}


void alpha_luminance8_to_bgra(void *out, const void *in, int count) {
    words16_to_32(out, in, count, [](Dword value) {
        Dword luminance = value & 255;
        return (((value >> 8 << 8 | luminance) << 8 | luminance) << 8) | luminance;
    });
}
void bgr_to_bgra(void *destination, const void *source, int count) {
    auto *output = static_cast<Dword *>(destination);
    const auto *input = static_cast<const Byte *>(source);
    for (int pixel = 0; pixel < count; ++pixel, input += 3)
        output[pixel] = ((Dword(input[2]) | 4294967040u) << 8 | input[1]) << 8 | input[0];
}


void argb6666_to_bgra(void *destination, const void *source, int count) {
    auto *output = static_cast<Dword *>(destination);
    const auto *input = static_cast<const Byte *>(source);
    for (int pixel = 0; pixel < count; ++pixel, input += 3) {
        Dword value;
        std::memcpy(&value, input, 4);
        value &= 16777215;
        Dword alpha = value >> 18 & 63, red = value >> 12 & 63, green = value >> 6 & 63,
             blue = value & 63;
        output[pixel] = ((((alpha << 2 | alpha >> 4) << 8 | (red << 2 | red >> 4)) << 8 |
                          (green << 2 | green >> 4))
                         << 8) |
                        (blue << 2 | blue >> 4);
    }
}
void argb2101010_to_bgra(void *out, const void *in, int count) {
    words32(out, in, count, [](Dword value) {
        return (((((value >> 30) * 255 + 2) / 3 << 8 | (value >> 22 & 255)) << 8 |
                 (value >> 12 & 255))
                << 8) |
               (value >> 2 & 255);
    });
}

} // namespace nfs::graphics::textures::shape::rows
