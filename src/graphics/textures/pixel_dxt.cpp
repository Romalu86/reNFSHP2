#include "nfs/graphics/textures/pixel_compression.hpp"
#include "nfs/gui/lifecycle/gui.hpp"
#include <cfenv>
#include <cmath>
namespace nfs::graphics::surfaces::pixels {
extern const Dword dxt_base[4], dxt_methods[5][4];
using Decode = int (*)(Colour *, const void *);
using Encode = int (*)(void *, const Colour *);

Codec *construct_dxt(Codec *object, const Image *image) {
    construct(object, image, 0, 1);
    volatile Dword *f = object->words;
    f[0] = reinterpret_cast<Dword>(dxt_base);
    f[1054] = image->words[6] == 1 ? 0 : image->words[6] == 2 ? 1 : 3;
    f[1055] = image->words[7] == 1 ? 0 : image->words[7] == 2 ? 1 : 3;
    f[1053] = !(f[1054] == 3 && f[1055] == 3);
    switch (f[1]) {
    case 827611204:
        f[1056] = 8;
        f[1058] = reinterpret_cast<Dword>(encode_dxt1);
        f[1057] = reinterpret_cast<Dword>(decode_dxt1);
        break;
    case 844388420:
        f[1056] = 16;
        f[1058] = reinterpret_cast<Dword>(encode_dxt2);
        f[1057] = reinterpret_cast<Dword>(decode_dxt2);
        break;
    case 861165636:
        f[1056] = 16;
        f[1058] = reinterpret_cast<Dword>(encode_dxt3);
        f[1057] = reinterpret_cast<Dword>(decode_dxt3);
        break;
    case 877942852:
        f[1056] = 16;
        f[1058] = reinterpret_cast<Dword>(encode_dxt4);
        f[1057] = reinterpret_cast<Dword>(decode_dxt4);
        break;
    case 894720068:
        f[1056] = 16;
        f[1058] = reinterpret_cast<Dword>(encode_dxt5);
        f[1057] = reinterpret_cast<Dword>(decode_dxt5);
        break;
    }
    // retains bounds and publishes these fields in this order.
    Dword begin_z = f[1040], begin_x = f[1036], end_y = f[1039], begin_y = f[1037];
    f[1068] = ~0u;
    f[1069] = ~0u;
    f[1063] = begin_z;
    Dword end_x = (f[1038] + 3) & ~3u;
    begin_x &= ~3u;
    begin_y &= ~3u;
    f[1061] = end_x;
    end_y = (end_y + 3) & ~3u;
    Dword end_z = f[1041];
    f[1062] = end_y;
    f[1065] = (end_x - begin_x) >> 2;
    f[1064] = end_z;
    Dword depth = end_z - f[1040], height = (end_y - begin_y) >> 2;
    f[1059] = begin_x;
    f[1070] = 0;
    f[1071] = 0;
    f[1072] = 0;
    f[1060] = begin_y;
    f[1066] = height;
    f[1067] = depth;
    return object;
}
// Incomplete compressed rows are released, without encoding.
void destroy_dxt(Codec *object) {
    Dword *f = object->words;
    f[0] = reinterpret_cast<Dword>(dxt_base);
    if (f[1071] && f[1072]) {
        auto *row = reinterpret_cast<Dword *>(f[1072]);
        for (Dword z = f[1063]; z < f[1064]; ++z)
            for (Dword y = f[1060]; y < f[1062]; y += 4, row += 2)
                gui::application_release(reinterpret_cast<void *>(row[1]));
    }
    gui::application_release(reinterpret_cast<void *>(f[1070]));
    gui::application_release(reinterpret_cast<void *>(f[1072]));
    destroy(object);
}
// Key quantization uses the caller's rounding mode.
void quantize_dxt_key(Codec *object) {
    Dword *f = object->words;
    auto *scale = reinterpret_cast<float *>(f + 1051);
    scale[0] = f[1] == 844388420 || f[1] == 861165636 ? 15.0f : 255.0f;
    scale[1] = 1.0 / scale[0];
    auto *key = reinterpret_cast<float *>(f + 7);
    const float scales[4] = {31, 63, 31, scale[0]},
                inverse[4] = {0.0322580635547637939453125f, 0.01587301678955554962158203125f, 0.0322580635547637939453125f, scale[1]};
    for (unsigned i = 0; i < 4; ++i) {
        float value = static_cast<double>(key[i]) * scales[i] + 0.5;
        key[i] = static_cast<double>(static_cast<int>(std::nearbyint(value))) * inverse[i];
    }
}
// Scan for the key after copying the row; retain the current scan range.
 
void read_dxt(Codec *object, Dword y, Dword z, Colour *output) {
    Dword *f = object->words;
    y += f[1037];
    z += f[1040];
    if (!f[1070]) {
        f[1070] = reinterpret_cast<Dword>(gui::application_allocate(f[1065] << 8));
        if (!f[1070])
            return;
    }
    if ((y & ~3u) != f[1068] || z != f[1069]) {
        f[1068] = y & ~3u;
        f[1069] = z;
        Dword packed = f[6] + (f[1068] >> 2) * f[1044] + (f[1059] >> 2) * f[1056] + z * f[1045];
        auto *decoded = reinterpret_cast<Colour *>(f[1070]);
        for (Dword x = f[1059]; x < f[1061]; x += 4, packed += f[1056], decoded += 16)
            reinterpret_cast<Decode>(f[1057])(decoded, reinterpret_cast<void *>(packed));
    }
    Dword first = f[1036] - f[1059], last = first + f[1046];
    for (Dword x = first; x < last; ++x, ++output) {
        Dword index = (((x & ~3u) | ((y - f[1060]) & 3)) << 2) | (x & 3);
        copy_forward(output, reinterpret_cast<Colour *>(f[1070]) + index, 16);
    }
    if (f[4]) {
        int rounding = std::fegetround();
        std::fesetround(FE_TOWARDZERO);
        const auto *key = reinterpret_cast<const float *>(f + 7);
        const auto *alpha = reinterpret_cast<const float *>(f + 1051);
        const float scales[4] = {31, 63, 31, alpha[0]},
                    inverse[4] = {0.0322580635547637939453125f, 0.01587301678955554962158203125f, 0.0322580635547637939453125f, alpha[1]};
        for (Dword x = 0; x < f[1046]; ++x) {
            const auto *value = reinterpret_cast<const float *>(output + x);
            bool equal = true;
            for (unsigned i = 0; i < 4; ++i) {
                float quantized = static_cast<double>(value[i]) * scales[i] + 0.5;
                double decoded = static_cast<double>(static_cast<int>(quantized)) * inverse[i];
                if (i != 3)
                    decoded = static_cast<float>(decoded);
                equal = equal && decoded == key[i];
            }
            if (equal)
                output[x] = {0, 0, 0, 0};
        }
        std::fesetround(rounding);
    }
}
// Deferred writes retain partial boundary blocks and reuse row storage.
void write_dxt(Codec *object, Dword y, Dword z, const Colour *input) {
    Dword *f = object->words;
    y += f[1037];
    z += f[1040];
    if (!f[1072]) {
        Dword bytes = f[1066] * f[1067] * 8;
        f[1072] = reinterpret_cast<Dword>(gui::application_allocate(bytes));
        if (!f[1072])
            return;
        std::memset(reinterpret_cast<void *>(f[1072]), 0, bytes);
    }
    auto *entry = reinterpret_cast<Dword *>(f[1072]) +
                  ((z - f[1063]) * f[1066] + ((y - f[1060]) >> 2)) * 2;
    if (!entry[1]) {
        entry[1] = reinterpret_cast<Dword>(gui::application_allocate(f[1065] << 8));
        if (!entry[1])
            return;
        entry[0] = 0;
        ++f[1071];
    }
    auto decode = reinterpret_cast<Decode>(f[1057]);
    auto address = [&](Dword x) {
        return reinterpret_cast<const void *>(f[6] + (y >> 2) * f[1044] + (x >> 2) * f[1056] +
                                              z * f[1045]);
    };
    if (!entry[0]) {
        Dword bottom = f[1039] > 3 ? f[1039] : 4;
        if ((y & 3) < f[1037] || bottom < (y & 3) + 4) {
            auto *values = reinterpret_cast<Colour *>(entry[1]);
            for (Dword x = f[1059]; x < f[1061]; x += 4, values += 16)
                decode(values, address(x));
        } else {
            Dword right = f[1038] > 3 ? f[1038] : 4;
            if (f[1059] < f[1036])
                decode(reinterpret_cast<Colour *>(entry[1]), address(f[1059]));
            if (right < f[1061])
                decode(reinterpret_cast<Colour *>(entry[1]), address(f[1061] - 4));
        }
    }
    if (f[1042])
        input = convert(object, input);
    Dword first = f[1036] - f[1059], last = first + f[1046];
    for (Dword x = first; x < last; ++x, ++input) {
        Dword index = (((x & ~3u) | ((y - f[1060]) & 3)) << 2) | (x & 3);
        copy_forward(reinterpret_cast<Colour *>(entry[1]) + index, input, 16);
    }
    Dword row = (y - f[1060]) & 3;
    if (!f[1053])
        entry[0] |= 1u << row;
    else
        for (Dword i = 0; i < 4; ++i)
            if ((f[1055] & i) == row)
                entry[0] |= 1u << i;
    if (entry[0] == 15) {
        auto *values = reinterpret_cast<Colour *>(entry[1]);
        for (Dword x = f[1059]; x < f[1061]; x += 4, values += 16) {
            if (f[1053])
                for (Dword by = 0; by < 4; ++by)
                    for (Dword bx = 0; bx < 4; ++bx) {
                        Dword target = by * 4 + bx, source = (bx & f[1054]) + (by & f[1055]) * 4;
                        if (target != source)
                            copy_forward(values + target, values + source, 16);
                    }
            reinterpret_cast<Encode>(f[1058])(const_cast<void *>(address(x)), values);
        }
        auto *limit = reinterpret_cast<Dword *>(f[1072] + f[1067] * f[1066] * 8);
        if (entry + 2 < limit && !entry[2] && !entry[3]) {
            entry[3] = entry[1];
            entry[1] = 0;
        } else {
            gui::application_release(reinterpret_cast<void *>(entry[1]));
            --f[1071];
        }
        entry[1] = 0;
    }
}
static void *__fastcall delete_dxt(Codec *object, void *, Dword flags) {
    destroy_dxt(object);
    if (flags & 1)
        gui::application_release(object);
    return object;
}
static void __fastcall read_dxt_row(Codec *o, void *, Dword y, Dword z, Colour *row) {
    read_dxt(o, y, z, row);
}
static void __fastcall write_dxt_row(Codec *o, void *, Dword y, Dword z, const Colour *row) {
    write_dxt(o, y, z, row);
}
static void __fastcall dxt_key(Codec *object, void *) {
    quantize_dxt_key(object);
}
const Dword dxt_base[4] = {reinterpret_cast<Dword>(delete_dxt), reinterpret_cast<Dword>(read_dxt_row),
                          reinterpret_cast<Dword>(write_dxt_row), reinterpret_cast<Dword>(dxt_key)};
#define DXT_METHODS(N)                                                                             \
    static void *__fastcall delete_dxt##N(Codec *object, void *, Dword flags) {                     \
        destroy_dxt(object);                                                                       \
        if (flags & 1)                                                                             \
            gui::application_release(object);                                                      \
        return object;                                                                             \
    }
DXT_METHODS(1)
DXT_METHODS(2) DXT_METHODS(3) DXT_METHODS(4) DXT_METHODS(5)
#undef DXT_METHODS
    const Dword dxt_methods[5][4] = {
        {reinterpret_cast<Dword>(delete_dxt1), reinterpret_cast<Dword>(read_dxt_row),
         reinterpret_cast<Dword>(write_dxt_row), reinterpret_cast<Dword>(dxt_key)},
        {reinterpret_cast<Dword>(delete_dxt2), reinterpret_cast<Dword>(read_dxt_row),
         reinterpret_cast<Dword>(write_dxt_row), reinterpret_cast<Dword>(dxt_key)},
        {reinterpret_cast<Dword>(delete_dxt3), reinterpret_cast<Dword>(read_dxt_row),
         reinterpret_cast<Dword>(write_dxt_row), reinterpret_cast<Dword>(dxt_key)},
        {reinterpret_cast<Dword>(delete_dxt4), reinterpret_cast<Dword>(read_dxt_row),
         reinterpret_cast<Dword>(write_dxt_row), reinterpret_cast<Dword>(dxt_key)},
        {reinterpret_cast<Dword>(delete_dxt5), reinterpret_cast<Dword>(read_dxt_row),
         reinterpret_cast<Dword>(write_dxt_row), reinterpret_cast<Dword>(dxt_key)}};

Codec *construct_dxt1(Codec *object, const Image *image) {
    construct_dxt(object, image);
    object->words[0] = reinterpret_cast<Dword>(dxt_methods[0]);
    return object;
}
Codec *construct_dxt2(Codec *object, const Image *image) {
    construct_dxt(object, image);
    object->words[0] = reinterpret_cast<Dword>(dxt_methods[1]);
    return object;
}
Codec *construct_dxt3(Codec *object, const Image *image) {
    construct_dxt(object, image);
    object->words[0] = reinterpret_cast<Dword>(dxt_methods[2]);
    return object;
}
Codec *construct_dxt4(Codec *object, const Image *image) {
    construct_dxt(object, image);
    object->words[0] = reinterpret_cast<Dword>(dxt_methods[3]);
    return object;
}
Codec *construct_dxt5(Codec *object, const Image *image) {
    construct_dxt(object, image);
    object->words[0] = reinterpret_cast<Dword>(dxt_methods[4]);
    return object;
}
} // namespace nfs::graphics::surfaces::pixels
