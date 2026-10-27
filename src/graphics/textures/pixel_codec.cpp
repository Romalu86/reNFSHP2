#include "nfs/graphics/textures/pixel_codec.hpp"
#include "../../math/precision/frame_x87_bridge.hpp"
static_assert(__LDBL_MANT_DIG__ == 64 && __LDBL_MAX_EXP__ == 16384,
              "Pixel conversion requires retained x87 intermediates");
#include "nfs/gui/lifecycle/gui.hpp"
#include <cstring>
namespace nfs::graphics::surfaces::pixels {

static Codec *__fastcall delete_base(Codec *codec, void *, Dword flags) {
    destroy(codec);
    if (flags & 1)
        gui::application_release(codec);
    return codec;
}
// The complete native body is RET0Ch.
static void __fastcall read_base_row(Codec *, void *, Dword, Dword, Colour *) {
}
// The complete native body is RET0Ch.
static void __fastcall write_base_row(Codec *, void *, Dword, Dword, const Colour *) {
}
// Written by both pixel-conversion paths.
const Dword base_methods[4] = {
    reinterpret_cast<Dword>(delete_base), reinterpret_cast<Dword>(read_base_row),
    reinterpret_cast<Dword>(write_base_row), reinterpret_cast<Dword>(quantize_key)};

void decode_colour(Colour *out, Dword value) {
    constexpr float scale = 1.0f / 255.0f;
    out->red = (value >> 16 & 255) * scale;
    out->green = (value >> 8 & 255) * scale;
    out->blue = (value & 255) * scale;
    out->alpha = (value >> 24) * scale;
}
// Empty float-vector construction does not clear the 4096-byte palette.
Codec *construct(Codec *object, const Image *image, Dword bits, Dword kind) {
    Dword *f = object->words;
    const Dword *in = image->words;
    f[0] = reinterpret_cast<Dword>(base_methods);
    f[6] = in[0];
    f[1] = in[1];
    f[1044] = in[2];
    f[1045] = in[3];
    std::memcpy(f + 1036, in + 10, 24);
    f[11] = reinterpret_cast<Dword>(in[16] ? ordered_dither : flat_dither);
    f[1050] = bits >> 3;
    f[1042] = f[1043] = 0;
    f[3] = bits != 0;
    f[2] = kind;
    f[4] = in[17];
    if (f[4])
        decode_colour(reinterpret_cast<Colour *>(f + 7), f[4]);
    f[5] = (f[1] == 40 || f[1] == 41);
    if (f[5]) {
        auto *palette = reinterpret_cast<Colour *>(f + 12);
        if (!in[18]) {
            for (int i = 0; i < 256; ++i)
                palette[i] = {1, 1, 1, 1};
        } else {
            const auto *entries = reinterpret_cast<const std::uint8_t *>(in[18]);
            constexpr float scale = 1.0f / 255.0f;
            for (int i = 0; i < 256; ++i)
                palette[i] = {entries[i * 4] * scale, entries[i * 4 + 1] * scale,
                              entries[i * 4 + 2] * scale, entries[i * 4 + 3] * scale};
        }
    }
    f[1047] = f[1039] - f[1037];
    f[1046] = f[1038] - f[1036];
    f[1048] = f[1041] - f[1040];
    f[1049] = f[1050] * f[1046];
    if (f[3]) {
        f[6] += f[1050] * f[1036] + f[1044] * f[1037] + f[1045] * f[1040];
        f[1038] = f[1046];
        f[1039] = f[1047];
        f[1036] = f[1037] = f[1040] = 0;
        f[1041] = f[1048];
    }
    return object;
}

void destroy(Codec *object) {
    object->words[0] = reinterpret_cast<Dword>(base_methods);
    gui::application_release(reinterpret_cast<void *>(object->words[1043]));
}
// The row cache is allocated only for a change in vector interpretation.
int source(Codec *object, const Codec *input) {
    Dword *f = object->words;
    if (f[2] != input->words[2]) {
        f[1043] = reinterpret_cast<Dword>(gui::application_allocate(f[1046] * 16));
        if (!f[1043])
            return static_cast<int>(2147942414u);
        f[1042] = input->words[2];
    }
    return 0;
}
template <Dword Destination, Dword Source>
static __forceinline void convert_row_vectors(volatile Dword *fields, const Colour *input) {
    auto output = [fields]() { return reinterpret_cast<Colour *>(fields[1043]); };
    for (Dword i = 0; i < fields[1046]; ++i) {
        const Colour &value = input[i];
        if constexpr (Destination == 1) {
            output()[i].red = (static_cast<long double>(value.red) + 1) * 0.5;
            output()[i].green = (static_cast<long double>(value.green) + 1) * 0.5;
            output()[i].blue = (static_cast<long double>(value.blue) + 1) * 0.5;
        } else if constexpr (Source == 1) {
            output()[i].red = static_cast<long double>(value.red) * 2 - 1;
            output()[i].green = static_cast<long double>(value.green) * 2 - 1;
            output()[i].blue = static_cast<long double>(value.blue) * 2 - 1;
        } else {
            output()[i].red = value.red;
            output()[i].green = value.green;
            output()[i].blue = value.blue;
        }
        if constexpr ((Destination == 1 && Source == 3) || (Destination == 2 && Source == 3))
            output()[i].alpha = (static_cast<long double>(value.alpha) + 1) * 0.5;
        else if constexpr (Destination == 3 && Source == 1)
            output()[i].alpha = static_cast<long double>(value.alpha) * 2 - 1;
        else if constexpr (Destination == 3 && Source == 2)
            output()[i].alpha = (static_cast<long double>(value.alpha) + value.alpha) - 1;
        else
            output()[i].alpha = value.alpha;
    }
}
// Unsupported kind pairs return before row access.
Colour *convert(Codec *object, const Colour *input) {
    volatile Dword *fields = object->words;
    switch (fields[2]) {
    case 1:
        switch (fields[1042]) {
        case 2:
            convert_row_vectors<1, 2>(fields, input);
            break;
        case 3:
            convert_row_vectors<1, 3>(fields, input);
            break;
        }
        break;
    case 2:
        switch (fields[1042]) {
        case 1:
            convert_row_vectors<2, 1>(fields, input);
            break;
        case 3:
            convert_row_vectors<2, 3>(fields, input);
            break;
        }
        break;
    case 3:
        switch (fields[1042]) {
        case 1:
            convert_row_vectors<3, 1>(fields, input);
            break;
        case 2:
            convert_row_vectors<3, 2>(fields, input);
            break;
        }
        break;
    }
    return reinterpret_cast<Colour *>(fields[1043]);
}
// TEST AH,44h / JP skips unordered values as well as unequal ones.
void colour_key(Codec *object, Colour *row) {
    const auto *key = reinterpret_cast<const float *>(object->words + 7);
    for (Dword i = 0; i < object->words[1046]; ++i) {
        auto *value = reinterpret_cast<float *>(row + i);
        bool equal = true;
        for (int channel = 0; channel < 4; ++channel)
            equal = equal && value[channel] == key[channel];
        if (equal)
            row[i] = {0, 0, 0, 0};
    }
}
// Quantize the key with the selected format's genuine writer and reader.
void __fastcall quantize_key(Codec *object, void *) {
    Dword *f = object->words;
    Dword width = f[1046], row_bytes = f[1049], data = f[6], dither = f[11], key = f[4];
    std::uint8_t scratch[8];
    f[1046] = 1;
    f[1049] = f[1050];
    f[6] = reinterpret_cast<Dword>(scratch);
    f[11] = reinterpret_cast<Dword>(flat_dither);
    f[4] = 0;
    if (f[2] != 1) {
        Dword source_kind = f[1042], cache = f[1043];
        f[1042] = 1;
        f[1043] = reinterpret_cast<Dword>(f + 7);
        convert(object, reinterpret_cast<const Colour *>(f + 7));
        f[1042] = source_kind;
        f[1043] = cache;
    }
    using Row = void(__thiscall *)(Codec *, Dword, Dword, Colour *);
    reinterpret_cast<const Row *>(f[0])[2](object, 0, 0, reinterpret_cast<Colour *>(f + 7));
    reinterpret_cast<const Row *>(f[0])[1](object, 0, 0, reinterpret_cast<Colour *>(f + 7));
    f[1046] = width;
    f[1049] = row_bytes;
    f[6] = data;
    f[11] = dither;
    f[4] = key;
}
} // namespace nfs::graphics::surfaces::pixels
