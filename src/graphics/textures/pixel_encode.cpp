#include "../../math/precision/frame_x87_bridge.hpp"
#include "nfs/graphics/textures/pixel_codec.hpp"
#include "pixel_format_rows_x87.hpp"
static_assert(__LDBL_MANT_DIG__ == 64 && __LDBL_MAX_EXP__ == 16384,
              "Pixel conversion requires retained x87 intermediates");
#include <cfenv>
#include <cfloat>
#include <cstring>
namespace nfs::graphics::surfaces::pixels {
// Save and restore the rounding mode around each packed writer.
 

static Dword quantize(float value, float scale, float dither) {
    volatile float rounded = static_cast<float>(static_cast<long double>(value) * scale + dither);
    return Dword(game::runtime::hardware::current_integer_f32(rounded));
}
// Format-specific entries are bound in pixel_tables.cpp.
template <Dword FixedFormat>
static void write_row(Codec *object, Dword runtime_format, Dword y, Dword z, const Colour *input) {
    const Dword format = FixedFormat ? FixedFormat : runtime_format;
    volatile Dword *f = object->words;
    if (f[1042])
        input = convert(object, input);
    auto *output = reinterpret_cast<std::uint8_t *>(f[6] + f[1044] * y + f[1045] * z);
    const auto *palette = reinterpret_cast<const Colour *>(object->words + 12);
    if (format == 41) {
        for (Dword i = 0; i < f[1046]; ++i) {
            Dword selected = 0;
            float minimum = FLT_MAX;
            for (Dword entry = 0; entry < 256; ++entry) {
                long double dr = static_cast<long double>(input[i].red) - palette[entry].red,
                            dg = static_cast<long double>(input[i].green) - palette[entry].green,
                            db = static_cast<long double>(input[i].blue) - palette[entry].blue,
                            da = static_cast<long double>(input[i].alpha) - palette[entry].alpha;
                long double distance = da * da + db * db + dg * dg + dr * dr;
                if (distance < minimum) {
                    selected = entry;
                    minimum = static_cast<float>(distance);
                }
            }
            output[i] = static_cast<std::uint8_t>(selected);
        }
        return;
    }
    int previous_round = std::fegetround();
    std::fesetround(FE_TOWARDZERO);
    const auto *dither = reinterpret_cast<const float *>(f[11]) + ((z & 3) + (y & 3) * 8);
    for (Dword i = 0; i < f[1046]; ++i) {
        const Colour &c = input[i];
        float bias = dither[i & 3];
        auto q = [&](float value, float scale) { return quantize(value, scale, bias); };
        auto *out = output + i * (FixedFormat ? format_pixel_bytes(FixedFormat) : f[1050]);
        Dword packed = 0;
        int bytes = (FixedFormat ? format_pixel_bytes(FixedFormat) : f[1050]);
        switch (format) {
        case 20:
            out[0] = q(c.blue, 255);
            out[1] = q(c.green, 255);
            out[2] = q(c.red, 255);
            continue;
        case 21:
            packed = (((q(c.alpha, 255) << 8 | q(c.red, 255)) << 8 | q(c.green, 255)) << 8) |
                     q(c.blue, 255);
            break;
        case 22:
            packed = (q(c.red, 255) << 8 | q(c.green, 255)) << 8 | q(c.blue, 255);
            break;
        case 23:
            packed = (q(c.red, 31) << 6 | q(c.green, 63)) << 5 | q(c.blue, 31);
            break;
        case 24:
            packed = (q(c.red, 31) << 5 | q(c.green, 31)) << 5 | q(c.blue, 31);
            break;
        case 25:
            packed =
                ((q(c.alpha, 1) << 5 | q(c.red, 31)) << 5 | q(c.green, 31)) << 5 | q(c.blue, 31);
            break;
        case 26:
            packed =
                ((q(c.alpha, 15) << 4 | q(c.red, 15)) << 4 | q(c.green, 15)) << 4 | q(c.blue, 15);
            break;
        case 27:
            packed = (q(c.red, 7) << 3 | static_cast<std::uint8_t>(q(c.green, 7))) << 2 |
                     static_cast<std::uint8_t>(q(c.blue, 3));
            break;
        case 28:
            packed = q(c.alpha, 255);
            break;
        case 29:
            packed =
                ((q(c.alpha, 255) << 3 | q(c.red, 7)) << 3 | q(c.green, 7)) << 2 | q(c.blue, 3);
            break;
        case 30:
            packed = (q(c.red, 15) << 4 | q(c.green, 15)) << 4 | q(c.blue, 15);
            break;
        case 31:
            packed = ((q(c.alpha, 3) << 10 | q(c.blue, 1023)) << 10 | q(c.green, 1023)) << 10 |
                     q(c.red, 1023);
            break;
        case 34:
            packed = q(c.green, 65535) << 16 | q(c.red, 65535);
            break;
        case 40: {
            Dword selected = 0;
            for (Dword entry = 0; entry < 256; ++entry) {
                long double dr = static_cast<long double>(input[0].red) - palette[entry].red,
                            dg = static_cast<long double>(input[0].green) - palette[entry].green,
                            db = static_cast<long double>(input[0].blue) - palette[entry].blue;
                if (db * db + dg * dg + dr * dr < FLT_MAX)
                    selected = entry;
            }
            packed = static_cast<std::uint8_t>(q(c.alpha, 255)) << 8 | selected;
            break;
        }
        case 50:
        case 51:
        case 52:
        case 70:
        case 909200416:
        case 909200449: {
            long double luminance = (static_cast<long double>(c.red) * 0.2125f +
                                     static_cast<long double>(c.green) * 0.7154f +
                                     static_cast<long double>(c.blue) * 0.0721f);
            float scale = (format == 50 || format == 51) ? 255.0f : format == 52 ? 15.0f : 65535.0f;
            volatile float value = static_cast<float>(luminance * scale + bias);
            packed = Dword(game::runtime::hardware::current_integer_f32(value));
            if (format == 51)
                packed |= (q(c.alpha, 255) & 255) << 8;
            else if (format == 52)
                packed |= q(c.alpha, 15) << 4;
            else if (format == 909200449)
                packed |= q(c.alpha, 65535) << 16;
            break;
        }
        case 60:
            packed = (q(c.green, 128) & 255) << 8 | (q(c.red, 128) & 255);
            break;
        case 61:
            packed = ((q(c.green, 16) & 31) | q(c.alpha, 63) << 5) << 5 | (q(c.red, 16) & 31);
            break;
        case 62:
            packed = ((q(c.alpha, 255) & 255) << 8 | (q(c.green, 128) & 255)) << 8 |
                     (q(c.red, 128) & 255);
            break;
        case 63:
            packed = ((q(c.blue, 128) & 255) | q(c.alpha, 128) << 8) << 8 | (q(c.green, 128) & 255);
            packed = packed << 8 | (q(c.red, 128) & 255);
            break;
        case 64:
            packed = q(c.green, 32768) << 16 | (q(c.red, 32768) & 65535);
            break;
        case 65:
            packed =
                ((q(c.green, 1024) & 2047) | q(c.blue, 1024) << 11) << 10 | (q(c.red, 512) & 1023);
            break;
        case 67: {
            volatile float alpha =
                static_cast<float>(static_cast<long double>(c.alpha) + c.alpha + bias);
            Dword a = Dword(game::runtime::hardware::current_integer_f32(alpha));
            packed = ((q(c.blue, 512) & 1023) | a << 10) << 10 | (q(c.green, 512) & 1023);
            packed = packed << 10 | (q(c.red, 512) & 1023);
            break;
        }
        case 909201952: {
            std::uint16_t values[3] = {static_cast<std::uint16_t>(q(c.blue, 65535)),
                                       static_cast<std::uint16_t>(q(c.green, 65535)),
                                       static_cast<std::uint16_t>(q(c.red, 65535))};
            std::memcpy(out, values, 6);
            continue;
        }
        case 909201985: {
            Dword r = q(c.red, 65535), g = q(c.green, 65535), b = q(c.blue, 65535),
                 a = q(c.alpha, 65535);
            Dword values[2] = {
                g << 16 | b,
                ((((a << 16 | r) >> 16) | static_cast<Dword>(static_cast<int>(g) >> 31)) << 16) |
                    ((r << 16 | g) >> 16) | static_cast<Dword>(static_cast<int>(b) >> 31)};
            std::memcpy(out, values, 8);
            continue;
        }
        }
        std::memcpy(out, &packed, bytes);
    }
    std::fesetround(previous_round);
}

void write(Codec *object, Dword format, Dword y, Dword z, const Colour *input) {
    write_row<0>(object, format, y, z, input);
}
template <Dword Format> void write_format_row(Codec *object, Dword y, Dword z, const Colour *input) {
    write_row<Format>(object, 0, y, z, input);
}
template void write_format_row<20>(Codec *, Dword, Dword, const Colour *);
template void write_format_row<21>(Codec *, Dword, Dword, const Colour *);
template void write_format_row<22>(Codec *, Dword, Dword, const Colour *);
template void write_format_row<23>(Codec *, Dword, Dword, const Colour *);
template void write_format_row<24>(Codec *, Dword, Dword, const Colour *);
template void write_format_row<25>(Codec *, Dword, Dword, const Colour *);
template void write_format_row<26>(Codec *, Dword, Dword, const Colour *);
template void write_format_row<27>(Codec *, Dword, Dword, const Colour *);
template void write_format_row<28>(Codec *, Dword, Dword, const Colour *);
template void write_format_row<29>(Codec *, Dword, Dword, const Colour *);
template void write_format_row<30>(Codec *, Dword, Dword, const Colour *);
template void write_format_row<31>(Codec *, Dword, Dword, const Colour *);
template void write_format_row<34>(Codec *, Dword, Dword, const Colour *);
template void write_format_row<40>(Codec *, Dword, Dword, const Colour *);
template void write_format_row<41>(Codec *, Dword, Dword, const Colour *);
template void write_format_row<50>(Codec *, Dword, Dword, const Colour *);
template void write_format_row<51>(Codec *, Dword, Dword, const Colour *);
template void write_format_row<52>(Codec *, Dword, Dword, const Colour *);
template void write_format_row<60>(Codec *, Dword, Dword, const Colour *);
template void write_format_row<61>(Codec *, Dword, Dword, const Colour *);
template void write_format_row<62>(Codec *, Dword, Dword, const Colour *);
template void write_format_row<63>(Codec *, Dword, Dword, const Colour *);
template void write_format_row<64>(Codec *, Dword, Dword, const Colour *);
template void write_format_row<65>(Codec *, Dword, Dword, const Colour *);
template void write_format_row<67>(Codec *, Dword, Dword, const Colour *);
template void write_format_row<70>(Codec *, Dword, Dword, const Colour *);
template void write_format_row<909200416>(Codec *, Dword, Dword, const Colour *);
template void write_format_row<909200449>(Codec *, Dword, Dword, const Colour *);
template void write_format_row<909201952>(Codec *, Dword, Dword, const Colour *);
template void write_format_row<909201985>(Codec *, Dword, Dword, const Colour *);
} // namespace nfs::graphics::surfaces::pixels
