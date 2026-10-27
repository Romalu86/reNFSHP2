#include "nfs/graphics/textures/pixel_codec.hpp"
#include "nfs/gui/lifecycle/gui.hpp"
#include <cfenv>
#include <cmath>
namespace nfs::graphics::surfaces::pixels {
extern const Dword yuv_base[4], uyvy_methods[4], yuy2_methods[4];
// Chroma comes from the first pixel, without pair averaging.
int flush_yuv(Codec *object) {
    Dword *f = object->words;
    if (f[1059] && f[1060]) {
        auto *out = reinterpret_cast<std::uint16_t *>(f[6] + f[1056] * f[1045] +
                                                      f[1053] * f[1044] + f[1052] * 2);
        auto *in = reinterpret_cast<const Colour *>(f[1051]);
        auto rounded = [](float value) { return static_cast<int>(std::nearbyint(value)); };
        auto clip = [](int value) { return value < 0 ? 0 : value > 255 ? 255 : value; };
        for (Dword x = f[1052]; x < f[1054]; x += 2, in += 2, out += 2) {
            float second = static_cast<double>(in[1].red) * 65.4810028076171875f +
                           static_cast<double>(in[1].blue) * 24.965999603271484375f +
                           static_cast<double>(in[1].green) * 128.5529937744140625f;
            float u = (static_cast<double>(in[0].blue) * 112 -
                       static_cast<double>(in[0].green) * 74.2030029296875f) -
                      static_cast<double>(in[0].red) * 37.797000885009765625f;
            float v = static_cast<double>(in[0].red) * 112 -
                      (static_cast<double>(in[0].green) * 93.78600311279296875f +
                       static_cast<double>(in[0].blue) * 18.214000701904296875f);
            float first = static_cast<double>(in[0].red) * 65.4810028076171875f +
                          static_cast<double>(in[0].green) * 128.5529937744140625f +
                          static_cast<double>(in[0].blue) * 24.965999603271484375f + 0.5;
            int y0 = clip(rounded(first) + 16), y1 = clip(rounded(second + 0.5f) + 16),
                cb = clip(rounded(u + 0.5f) + 128), cr = clip(rounded(v + 0.5f) + 128);
            out[0] = static_cast<std::uint16_t>((Dword(y0) << (f[1061] & 31)) |
                                                (Dword(cb) << (f[1062] & 31)));
            out[1] = static_cast<std::uint16_t>((Dword(y1) << (f[1061] & 31)) |
                                                (Dword(cr) << (f[1062] & 31)));
        }
        f[1059] = 0;
    }
    return 0;
}
// A cache miss flushes dirty pixels before decoding a new row.
int cache_yuv(Codec *object, Dword y, Dword z, Dword decode) {
    Dword *f = object->words;
    if (!f[1060])
        return static_cast<int>(2147942414u);
    if (y < f[1053] || y >= f[1055] || z < f[1056] || z >= f[1057]) {
        int result = flush_yuv(object);
        if (result < 0)
            return result;
        f[1055] = y + 1;
        f[1053] = y;
        f[1056] = z;
        f[1057] = z + 1;
        if (decode) {
            auto *in = reinterpret_cast<const std::uint16_t *>(f[6] + f[1044] * y + f[1045] * z +
                                                               f[1052] * 2);
            auto *out = reinterpret_cast<Colour *>(f[1051]);
            auto clamp = [](float value) {
                if (!(value >= 0))
                    return 0.0f;
                return value > 1 ? 1.0f : value;
            };
            for (Dword x = f[1052]; x < f[1054]; x += 2, in += 2, out += 2) {
                float y0 = float(in[0] >> (f[1061] & 31) & 255) - 16,
                      y1 = float(in[1] >> (f[1061] & 31) & 255) - 16;
                float cb = float(in[0] >> (f[1062] & 31) & 255) - 128,
                      cr = float(in[1] >> (f[1062] & 31) & 255) - 128;
                double r = static_cast<double>(cr) * 0.006258930079638957977294921875f,
                       l0 = static_cast<double>(y0) * 0.0045662098564207553863525390625f;
                float l = static_cast<float>(l0), g = static_cast<double>(cb) * 0.001536319963634014129638671875f,
                      h = static_cast<double>(cr) * 0.003188109956681728363037109375f,
                      b = static_cast<double>(cb) * 0.0079107098281383514404296875f;
                out[0] = {
                    clamp(l0 + r),
                    clamp((static_cast<double>(l) - g) - static_cast<double>(cr) * 0.003188109956681728363037109375f),
                    clamp(static_cast<double>(cb) * 0.0079107098281383514404296875f + l), 1};
                double l1 = static_cast<double>(y1) * 0.0045662098564207553863525390625f;
                l = static_cast<float>(l1);
                out[1] = {clamp(l1 + r), clamp((static_cast<double>(l) - g) - h),
                          clamp(static_cast<double>(l) + b), 1};
            }
        }
    }
    return 0;
}
// including forward word-copy order.
void read_yuv(Codec *object, Dword y, Dword z, Colour *output) {
    Dword *f = object->words;
    if (cache_yuv(object, y + f[1037], z + f[1040], 1) >= 0) {
        copy_forward(output, reinterpret_cast<const Colour *>(f[1051]) + f[1036] - f[1052],
                     f[1046] * 16);
        if (f[4])
            colour_key(object, output);
    }
}
void write_yuv(Codec *object, Dword y, Dword z, const Colour *input) {
    Dword *f = object->words;
    if (f[1042])
        input = convert(object, input);
    if (cache_yuv(object, y + f[1037], z + f[1040], f[1058] != f[1046]) >= 0) {
        copy_forward(reinterpret_cast<Colour *>(f[1051]) + f[1036] - f[1052], input,
                     f[1046] * 16);
        f[1059] = 1;
    }
}
// Pending output is committed during destruction.
void destroy_yuv(Codec *object) {
    object->words[0] = reinterpret_cast<Dword>(yuv_base);
    flush_yuv(object);
    if (object->words[1051])
        gui::application_release(reinterpret_cast<void *>(object->words[1051]));
    destroy(object);
}
static void *__fastcall delete_yuv(Codec *object, void *, Dword flags) {
    destroy_yuv(object);
    if (flags & 1)
        gui::application_release(object);
    return object;
}
static void *__fastcall delete_uyvy(Codec *object, void *, Dword flags) {
    destroy_yuv(object);
    if (flags & 1)
        gui::application_release(object);
    return object;
}
static void *__fastcall delete_yuy2(Codec *object, void *, Dword flags) {
    destroy_yuv(object);
    if (flags & 1)
        gui::application_release(object);
    return object;
}
static void __fastcall read_yuv_row(Codec *object, void *, Dword y, Dword z, Colour *row) {
    read_yuv(object, y, z, row);
}
static void __fastcall write_yuv_row(Codec *object, void *, Dword y, Dword z, const Colour *row) {
    write_yuv(object, y, z, row);
}
// is a genuine RET (the YUV key remains unquantized).
static void __fastcall yuv_key(Codec *, void *) {
}
const Dword yuv_base[4] = {reinterpret_cast<Dword>(delete_yuv), reinterpret_cast<Dword>(read_yuv_row),
                          reinterpret_cast<Dword>(write_yuv_row), reinterpret_cast<Dword>(yuv_key)};
const Dword uyvy_methods[4] = {
    reinterpret_cast<Dword>(delete_uyvy), reinterpret_cast<Dword>(read_yuv_row),
    reinterpret_cast<Dword>(write_yuv_row), reinterpret_cast<Dword>(yuv_key)};
const Dword yuy2_methods[4] = {
    reinterpret_cast<Dword>(delete_yuy2), reinterpret_cast<Dword>(read_yuv_row),
    reinterpret_cast<Dword>(write_yuv_row), reinterpret_cast<Dword>(yuv_key)};

Codec *construct_yuv(Codec *object, const Image *image) {
    construct(object, image, 0, 1);
    // FuncInfo  unwinds the completed base on allocation failure.
    struct BaseConstruction {
        Codec *object;
        ~BaseConstruction() noexcept(false) {
            if (object)
                destroy(object);
        }
    } pending{object};
    Dword *f = object->words;
    f[0] = reinterpret_cast<Dword>(yuv_base);
    f[1052] = f[1036] & ~1u;
    f[1054] = (f[1038] + 1) & ~1u;
    f[1053] = f[1055] = f[1056] = f[1057] = 0;
    f[1058] = f[1054] - f[1052];
    f[1059] = 0;
    f[1060] = 1;
    f[1051] = reinterpret_cast<Dword>(gui::application_allocate(f[1058] * 16));
    if (!f[1051])
        f[1060] = 0;
    f[1061] = image->words[1] == 1498831189 ? 8 : 0;
    f[1062] = image->words[1] == 1498831189 ? 0 : 8;
    pending.object = nullptr;
    return object;
}
Codec *construct_uyvy(Codec *object, const Image *image) {
    construct_yuv(object, image);
    object->words[0] = reinterpret_cast<Dword>(uyvy_methods);
    return object;
}
Codec *construct_yuy2(Codec *object, const Image *image) {
    construct_yuv(object, image);
    object->words[0] = reinterpret_cast<Dword>(yuy2_methods);
    return object;
}
} // namespace nfs::graphics::surfaces::pixels
