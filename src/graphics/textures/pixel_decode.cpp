#include "../../math/precision/frame_x87_bridge.hpp"
#include "nfs/graphics/textures/pixel_codec.hpp"
#include "pixel_format_rows_x87.hpp"
static_assert(__LDBL_MANT_DIG__ == 64 && __LDBL_MAX_EXP__ == 16384,
              "Pixel conversion requires retained x87 intermediates");
namespace nfs::graphics::surfaces::pixels {
namespace {
template <class Component>
Component read_component(const std::uint8_t *pixel, unsigned offset = 0) {
    return *reinterpret_cast<const volatile Component *>(pixel + offset);
}

void copy_palette_colour(const Colour *palette, std::uint8_t index, volatile Colour &colour) {
    const auto *source = reinterpret_cast<const volatile Dword *>(palette + index);
    auto *destination = reinterpret_cast<volatile Dword *>(&colour);
    destination[0] = source[0];
    destination[1] = source[1];
    destination[2] = source[2];
    destination[3] = source[3];
}

std::uint64_t read_colour_pair(const std::uint8_t *pixel) {
    const Dword low = read_component<Dword>(pixel);
    const Dword high = read_component<Dword>(pixel, 4);
    return low | (std::uint64_t(high) << 32);
}
} // namespace

// Channel stores precede the next component read, including overlapping rows.
// Each fixed-format table entry is bound separately in pixel_tables.cpp.
template <Dword FixedFormat>
static void read_row(Codec *object, Dword runtime_format, Dword y, Dword z, Colour *output) {
    const Dword format = FixedFormat ? FixedFormat : runtime_format;
    volatile Dword *f = object->words;
    const auto *input = reinterpret_cast<const std::uint8_t *>(f[6] + f[1044] * y + f[1045] * z);
    constexpr float b8 = 1.0f / 255.0f, b5 = 1.0f / 31.0f, b6 = 1.0f / 63.0f, b4 = 1.0f / 15.0f,
                    b3 = 1.0f / 7.0f, b2 = 1.0f / 3.0f, b10 = 1.0f / 1023.0f, b16 = 1.0f / 65535.0f;
    Dword row_end = 0;
    if constexpr (FixedFormat)
        row_end =
            reinterpret_cast<Dword>(input) + ((FixedFormat == 20 || FixedFormat == 909201952)
                                                 ? f[1049]
                                                 : f[1046] * format_pixel_bytes(FixedFormat));
    Dword i = 0;
    for (;
         FixedFormat ? reinterpret_cast<Dword>(input) + i * format_pixel_bytes(FixedFormat) < row_end
                     : i < f[1046];
         ++i) {
        const auto *p = input + i * (FixedFormat ? format_pixel_bytes(FixedFormat) : f[1050]);
        Dword word;
        std::uint16_t half;
        volatile Colour &c = output[i];
        switch (format) {
        case 20:
            c.red = read_component<std::uint8_t>(p, 2) * b8;
            c.green = read_component<std::uint8_t>(p, 1) * b8;
            c.blue = read_component<std::uint8_t>(p) * b8;
            c.alpha = 1;
            break;
        case 21:
            c.red = read_component<std::uint8_t>(p, 2) * b8;
            c.green = read_component<std::uint8_t>(p, 1) * b8;
            c.blue = (read_component<Dword>(p) & 255) * b8;
            c.alpha = read_component<std::uint8_t>(p, 3) * b8;
            break;
        case 22:
            c.red = read_component<std::uint8_t>(p, 2) * b8;
            c.green = read_component<std::uint8_t>(p, 1) * b8;
            c.blue = (read_component<Dword>(p) & 255) * b8;
            c.alpha = 1;
            break;
        case 23:
            c.red = (read_component<std::uint16_t>(p) >> 11) * b5;
            c.green = ((read_component<std::uint16_t>(p) >> 5) & 63) * b6;
            c.blue = (read_component<std::uint8_t>(p) & 31) * b5;
            c.alpha = 1;
            break;
        case 24:
            c.red = ((read_component<std::uint16_t>(p) >> 10) & 31) * b5;
            c.green = ((read_component<std::uint16_t>(p) >> 5) & 31) * b5;
            c.blue = (read_component<std::uint8_t>(p) & 31) * b5;
            c.alpha = 1;
            break;
        case 25:
            c.red = ((read_component<std::uint16_t>(p) >> 10) & 31) * b5;
            c.green = ((read_component<std::uint16_t>(p) >> 5) & 31) * b5;
            c.blue = (read_component<std::uint8_t>(p) & 31) * b5;
            c.alpha = static_cast<float>(read_component<std::uint16_t>(p) >> 15);
            break;
        case 26:
            c.red = (read_component<std::uint8_t>(p, 1) & 15) * b4;
            c.green = (read_component<std::uint8_t>(p) >> 4) * b4;
            c.blue = (read_component<std::uint8_t>(p) & 15) * b4;
            c.alpha = (read_component<std::uint16_t>(p) >> 12) * b4;
            break;
        case 27:
            c.red = (read_component<std::uint8_t>(p) >> 5) * b3;
            c.green = ((read_component<std::uint8_t>(p) >> 2) & 7) * b3;
            c.blue = (read_component<std::uint8_t>(p) & 3) * b2;
            c.alpha = 1;
            break;
        case 28:
            c.red = 0;
            c.green = 0;
            c.blue = 0;
            c.alpha = read_component<std::uint8_t>(p) * b8;
            break;
        case 29:
            c.red = (read_component<std::uint8_t>(p) >> 5) * b3;
            c.green = ((read_component<std::uint8_t>(p) >> 2) & 7) * b3;
            c.blue = (read_component<std::uint8_t>(p) & 3) * b2;
            c.alpha = read_component<std::uint8_t>(p, 1) * b8;
            break;
        case 30:
            c.red = (read_component<std::uint8_t>(p, 1) & 15) * b4;
            c.green = (read_component<std::uint8_t>(p) >> 4) * b4;
            c.blue = (read_component<std::uint8_t>(p) & 15) * b4;
            c.alpha = 1;
            break;
        case 31:
            c.red = (read_component<Dword>(p) & 1023) * b10;
            c.green = ((read_component<Dword>(p) >> 10) & 1023) * b10;
            c.blue = ((read_component<Dword>(p) >> 20) & 1023) * b10;
            c.alpha = (read_component<Dword>(p) >> 30) * b2;
            break;
        case 34:
            c.red = (read_component<Dword>(p) & 65535) * b16;
            c.green = read_component<std::uint16_t>(p, 2) * b16;
            c.blue = 1;
            c.alpha = 1;
            break;
        case 40:
            copy_palette_colour(reinterpret_cast<const Colour *>(object->words + 12),
                                read_component<std::uint8_t>(p), c);
            c.alpha = read_component<std::uint8_t>(p, 1) * b8;
            break;
        case 41:
            copy_palette_colour(reinterpret_cast<const Colour *>(object->words + 12),
                                read_component<std::uint8_t>(p), c);
            break;
        case 50:
        case 51: {
            const long double luminance =
                static_cast<long double>(read_component<std::uint8_t>(p)) * b8;
            c.blue = static_cast<float>(luminance);
            c.green = static_cast<float>(luminance);
            c.red = static_cast<float>(luminance);
            c.alpha = format == 51 ? read_component<std::uint8_t>(p, 1) * b8 : 1;
            break;
        }
        case 52: {
            const long double luminance =
                static_cast<long double>(read_component<std::uint8_t>(p) & 15) * b4;
            c.blue = static_cast<float>(luminance);
            c.green = static_cast<float>(luminance);
            c.red = static_cast<float>(luminance);
            c.alpha = (read_component<std::uint8_t>(p) >> 4) * b4;
            break;
        }
        case 60:
            c.red = read_component<std::int8_t>(p) * 0.0078125f;
            c.green = read_component<std::int8_t>(p, 1) * 0.0078125f;
            c.blue = 0;
            c.alpha = 1;
            break;
        case 61: {
            const auto red_bits = read_component<std::uint8_t>(p);
            half = read_component<std::uint16_t>(p);
            c.red = (static_cast<std::int8_t>(red_bits << 3) >> 3) * 0.0625f;
            c.green = (static_cast<std::int8_t>((half >> 5) << 3) >> 3) * 0.0625f;
            c.blue = 0;
            c.alpha = (read_component<std::uint16_t>(p) >> 10) * b6;
            break;
        }
        case 62:
            c.red = read_component<std::int8_t>(p) * 0.0078125f;
            c.green = read_component<std::int8_t>(p, 1) * 0.0078125f;
            c.blue = 0;
            c.alpha = read_component<std::uint8_t>(p, 2) * b8;
            break;
        case 63:
            c.red = read_component<std::int8_t>(p) * 0.0078125f;
            c.green = read_component<std::int8_t>(p, 1) * 0.0078125f;
            c.blue = read_component<std::int8_t>(p, 2) * 0.0078125f;
            c.alpha = read_component<std::int8_t>(p, 3) * 0.0078125f;
            break;
        case 64:
            c.red = read_component<std::int16_t>(p) * 0.000030517578125f;
            c.green = read_component<std::int16_t>(p, 2) * 0.000030517578125f;
            c.blue = 0;
            c.alpha = 1;
            break;
        case 65:
            word = read_component<Dword>(p);
            half = read_component<std::uint16_t>(p);
            c.red = (static_cast<std::int16_t>(half << 6) >> 6) * 0.001953125f;
            c.green = (static_cast<std::int16_t>((word >> 10) << 5) >> 5) * 0.0009765625f;
            c.blue = static_cast<std::int16_t>(static_cast<int>(word) >> 21) * 0.0009765625f;
            c.alpha = 1;
            break;
        case 67:
            word = read_component<Dword>(p);
            half = read_component<std::uint16_t>(p);
            c.red = (static_cast<std::int16_t>(half << 6) >> 6) * 0.001953125f;
            c.green = (static_cast<std::int16_t>((word >> 10) << 6) >> 6) * 0.001953125f;
            c.blue = (static_cast<std::int16_t>((word >> 20) << 6) >> 6) * 0.001953125f;
            c.alpha = (read_component<Dword>(p) >> 30) * b2;
            break;
        case 70:
        case 909200416: {
            const long double luminance =
                static_cast<long double>(read_component<std::uint16_t>(p)) * b16;
            c.blue = static_cast<float>(luminance);
            c.green = static_cast<float>(luminance);
            c.red = static_cast<float>(luminance);
            c.alpha = 1;
            break;
        }
        case 909200449: {
            const long double luminance =
                static_cast<long double>(read_component<Dword>(p) & 65535) * b16;
            c.blue = static_cast<float>(luminance);
            c.green = static_cast<float>(luminance);
            c.red = static_cast<float>(luminance);
            c.alpha = read_component<std::uint16_t>(p, 2) * b16;
            break;
        }
        case 909201952:
            c.red = read_component<std::uint16_t>(p, 4) * b16;
            c.green = read_component<std::uint16_t>(p, 2) * b16;
            c.blue = read_component<std::uint16_t>(p) * b16;
            c.alpha = 1;
            break;
        case 909201985:
            c.red = static_cast<Dword>((read_colour_pair(p) >> 32) & 65535) * b16;
            c.green = static_cast<Dword>((read_colour_pair(p) >> 16) & 65535) * b16;
            c.blue = (read_component<Dword>(p) & 65535) * b16;
            c.alpha = read_component<std::uint16_t>(p, 6) * b16;
            break;
        }
    }
    if (f[4]) {
        if constexpr (FixedFormat)
            output =
                reinterpret_cast<Colour *>(reinterpret_cast<Dword>(output) + i * 16 - f[1046] * 16);
        colour_key(object, output);
    }
}

void read(Codec *object, Dword format, Dword y, Dword z, Colour *output) {
    read_row<0>(object, format, y, z, output);
}
template <Dword Format> void read_format_row(Codec *object, Dword y, Dword z, Colour *output) {
    read_row<Format>(object, 0, y, z, output);
}
template void read_format_row<20>(Codec *, Dword, Dword, Colour *);
template void read_format_row<21>(Codec *, Dword, Dword, Colour *);
template void read_format_row<22>(Codec *, Dword, Dword, Colour *);
template void read_format_row<23>(Codec *, Dword, Dword, Colour *);
template void read_format_row<24>(Codec *, Dword, Dword, Colour *);
template void read_format_row<25>(Codec *, Dword, Dword, Colour *);
template void read_format_row<26>(Codec *, Dword, Dword, Colour *);
template void read_format_row<27>(Codec *, Dword, Dword, Colour *);
template void read_format_row<28>(Codec *, Dword, Dword, Colour *);
template void read_format_row<29>(Codec *, Dword, Dword, Colour *);
template void read_format_row<30>(Codec *, Dword, Dword, Colour *);
template void read_format_row<31>(Codec *, Dword, Dword, Colour *);
template void read_format_row<34>(Codec *, Dword, Dword, Colour *);
template void read_format_row<40>(Codec *, Dword, Dword, Colour *);
template void read_format_row<41>(Codec *, Dword, Dword, Colour *);
template void read_format_row<50>(Codec *, Dword, Dword, Colour *);
template void read_format_row<51>(Codec *, Dword, Dword, Colour *);
template void read_format_row<52>(Codec *, Dword, Dword, Colour *);
template void read_format_row<60>(Codec *, Dword, Dword, Colour *);
template void read_format_row<61>(Codec *, Dword, Dword, Colour *);
template void read_format_row<62>(Codec *, Dword, Dword, Colour *);
template void read_format_row<63>(Codec *, Dword, Dword, Colour *);
template void read_format_row<64>(Codec *, Dword, Dword, Colour *);
template void read_format_row<65>(Codec *, Dword, Dword, Colour *);
template void read_format_row<67>(Codec *, Dword, Dword, Colour *);
template void read_format_row<70>(Codec *, Dword, Dword, Colour *);
template void read_format_row<909200416>(Codec *, Dword, Dword, Colour *);
template void read_format_row<909200449>(Codec *, Dword, Dword, Colour *);
template void read_format_row<909201952>(Codec *, Dword, Dword, Colour *);
template void read_format_row<909201985>(Codec *, Dword, Dword, Colour *);
} // namespace nfs::graphics::surfaces::pixels
