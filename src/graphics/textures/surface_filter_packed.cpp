#include "nfs/graphics/textures/surface_filters.hpp"
#include <intrin.h>
namespace nfs::graphics::surfaces {

static int mmx_state = -1;

static int cpu_has_mmx() {
    int registers[4];
    __cpuid(registers, 1);
    return (registers[3] & 8388608) != 0;
}
int mmx_available() {
    HKEY key;
    DWORD kind, bytes = 4;
    int disabled;
    if (RegOpenKeyA(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Direct3D", &key) == ERROR_SUCCESS) {
        LSTATUS result = RegQueryValueExA(key, "DisableMMX", nullptr, &kind,
                                          reinterpret_cast<BYTE *>(&disabled), &bytes);
        if (result == ERROR_SUCCESS && kind == REG_DWORD && disabled) {
            RegCloseKey(key);
            mmx_state = 0;
            return 0;
        }
        RegCloseKey(key);
    }
    if (mmx_state < 0) {
        mmx_state = 0;
        if (cpu_has_mmx())
            mmx_state = 1;
    }
    return mmx_state;
}
// Retain the packed arithmetic and carries of the scalar path.
int average_argb(Dword *out, const Dword *in, Dword width, Dword height, Dword pitch,
                 Dword output_pitch) {
    Dword end = reinterpret_cast<Dword>(in) + height * pitch;
    for (Dword row = reinterpret_cast<Dword>(in); row < end; row += pitch * 2,
              out = reinterpret_cast<Dword *>(reinterpret_cast<Dword>(out) + output_pitch)) {
        auto *a = reinterpret_cast<const Dword *>(row);
        auto *b = reinterpret_cast<const Dword *>(row + pitch);
        for (Dword x = 0; x < width; x += 2) {
            Dword p = a[x], q = a[x + 1], r = b[x], s = b[x + 1];
            out[x / 2] =
                ((((p >> 2 & 4290789312u) + (r >> 2 & 4290789312u) + (s >> 2 & 4290789312u) -
                   8388480 + (q >> 2 & 4290789312u)) ^
                  (((p & 16711935) + (r & 16711935) + (s & 16711935) + 131074 + (q & 16711935)) >>
                   2)) &
                 16711935) ^
                (8388736 + (q >> 2 & 1069563840) + (p >> 2 & 1069563840) + (r >> 2 & 1069563840) +
                 (s >> 2 & 1069563840));
        }
    }
    return 0;
}

int average_xrgb(Dword *out, const Dword *in, Dword width, Dword height, Dword pitch,
                 Dword output_pitch) {
    Dword end = reinterpret_cast<Dword>(in) + height * pitch;
    for (Dword row = reinterpret_cast<Dword>(in); row < end; row += pitch * 2,
              out = reinterpret_cast<Dword *>(reinterpret_cast<Dword>(out) + output_pitch)) {
        auto *a = reinterpret_cast<const Dword *>(row);
        auto *b = reinterpret_cast<const Dword *>(row + pitch);
        for (Dword x = 0; x < width; x += 2) {
            Dword p = a[x], q = a[x + 1], r = b[x], s = b[x + 1];
            out[x / 2] =
                ((((p & 16711935) + (r & 16711935) + (s & 16711935) + 131074 + (q & 16711935)) &
                  66847740) |
                 (((p & 65280) + (r & 65280) + (s & 65280) + 512 + (q & 65280)) & 261120)) >>
                2;
        }
    }
    return 0;
}
// C++ expresses the MMX lane arithmetic, including eight-byte stores.
int average_mmx(Dword *out, const Dword *in, Dword width, Dword height, Dword pitch, Dword output_pitch) {
    if (width & 3)
        return average_argb(out, in, width, height, pitch, output_pitch);
    Dword end = reinterpret_cast<Dword>(in) + height * pitch;
    for (Dword row = reinterpret_cast<Dword>(in); row < end; row += pitch * 2,
              out = reinterpret_cast<Dword *>(reinterpret_cast<Dword>(out) + output_pitch)) {
        const auto *a = reinterpret_cast<const std::uint8_t *>(row);
        const auto *b = reinterpret_cast<const std::uint8_t *>(row + pitch);
        for (Dword x = 0; x < width; x += 4) {
            Dword result[2] = {};
            for (Dword pair = 0; pair < 2; ++pair)
                for (Dword channel = 0; channel < 4; ++channel) {
                    Dword offset = (x + pair * 2) * 4 + channel;
                    result[pair] |=
                        ((a[offset] + a[offset + 4] + b[offset] + b[offset + 4] + 2) >> 2)
                        << (channel * 8);
                }
            std::memcpy(out + x / 2, result, 8);
        }
    }
    return 0;
}
using Average = int (*)(Dword *, const Dword *, Dword, Dword, Dword, Dword);
static int select_argb(Dword *, const Dword *, Dword, Dword, Dword, Dword);
static int select_xrgb(Dword *, const Dword *, Dword, Dword, Dword, Dword);
Average argb_average = select_argb, xrgb_average = select_xrgb;

static void select_average() {
    if (mmx_available()) {
        argb_average = average_mmx;
        xrgb_average = average_mmx;
    } else {
        argb_average = average_argb;
        xrgb_average = average_xrgb;
    }
}
static int select_argb(Dword *o, const Dword *i, Dword w, Dword h, Dword p, Dword q) {
    select_average();
    return argb_average(o, i, w, h, p, q);
}
static int select_xrgb(Dword *o, const Dword *i, Dword w, Dword h, Dword p, Dword q) {
    select_average();
    return xrgb_average(o, i, w, h, p, q);
}
// All masks/biases are taken from their packed loops.
static int packed_words(Transfer *object, Dword mask, Dword bias, Dword other_mask, Dword other_bias,
                        Dword bytes, bool preserve_high) {
    Dword *s = object->source;
    Dword *d = object->destination;
    for (Dword y = 0; y < s[1047]; y += 2) {
        const auto *top = reinterpret_cast<const std::uint8_t *>(s[6] + y * s[1044]);
        const auto *bottom = top + s[1044];
        auto *out = reinterpret_cast<std::uint8_t *>(d[6] + (y / 2) * d[1044]);
        for (Dword x = 0; x < s[1046]; x += 2) {
            Dword p = 0, q = 0, r = 0, t = 0;
            std::memcpy(&p, top + x * bytes, bytes);
            std::memcpy(&q, top + (x + 1) * bytes, bytes);
            std::memcpy(&r, bottom + x * bytes, bytes);
            std::memcpy(&t, bottom + (x + 1) * bytes, bytes);
            Dword first = ((p & mask) + (q & mask) + (r & mask) + (t & mask) + bias) >> 2,
                 second = ((p & other_mask) + (q & other_mask) + (r & other_mask) +
                           (t & other_mask) + other_bias) >>
                          2;
            Dword result = preserve_high ? ((first ^ second) & other_mask) ^ first
                                        : (first & mask) | (second & other_mask);
            std::memcpy(out + (x / 2) * bytes, &result, bytes);
        }
    }
    return 0;
}
int packed_half(Transfer *object) {
    Dword *s = object->source;
    Dword *d = object->destination;
    switch (s[1]) {
    case 21:
        return argb_average(reinterpret_cast<Dword *>(d[6]), reinterpret_cast<const Dword *>(s[6]),
                            s[1046], s[1047], s[1044], d[1044]);
    case 22:
        return xrgb_average(reinterpret_cast<Dword *>(d[6]), reinterpret_cast<const Dword *>(s[6]),
                            s[1046], s[1047], s[1044], d[1044]);
    case 23:
        return packed_words(object, 63519, 4098, 2016, 64, 2, true);
    case 24:
        return packed_words(object, 31775, 2050, 992, 64, 2, false);
    case 25:
        return packed_words(object, 33760, 65600, 31775, 2050, 2, true);
    case 26:
        return packed_words(object, 61680, 8224, 3855, 514, 2, true);
    case 27:
        return packed_words(object, 227, 66, 28, 8, 1, true);
    case 28:
    case 50:
        return packed_words(object, 255, 2, 0, 0, 1, false);
    case 29:
        return packed_words(object, 65308, 520, 227, 66, 2, true);
    case 30:
        return packed_words(object, 3855, 514, 240, 32, 2, false);
    case 51:
        return packed_words(object, 65280, 512, 255, 2, 2, true);
    case 52:
        return packed_words(object, 240, 32, 15, 2, 1, true);
    default:
        return static_cast<int>(2147500037u);
    }
}
} // namespace nfs::graphics::surfaces
