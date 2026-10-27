#include "nfs/graphics/textures/pixel_codec.hpp"
#include "pixel_format_rows_x87.hpp"
#include "nfs/gui/lifecycle/gui.hpp"
namespace nfs::graphics::surfaces::pixels {
#include "pixel_dither.inc"
// vtable .
static void *__fastcall delete_rgb8(Codec *object, void *, unsigned flags) {
    destroy(object);
    if (flags & 1)
        gui::application_release(object);
    return object;
}

static void __fastcall read_rgb8(Codec *object, void *, Dword y, Dword z, Colour *out) {
    read_format_row<20>(object, y, z, out);
}

static void __fastcall write_rgb8(Codec *object, void *, Dword y, Dword z, const Colour *in) {
    write_format_row<20>(object, y, z, in);
}
static const Dword table_rgb8[4] = {
    reinterpret_cast<Dword>(delete_rgb8), reinterpret_cast<Dword>(read_rgb8),
    reinterpret_cast<Dword>(write_rgb8), reinterpret_cast<Dword>(quantize_key)};

static Codec *construct_rgb8(Codec *object, const Image *image) {
    construct(object, image, 24, 1);
    object->words[0] = reinterpret_cast<Dword>(table_rgb8);
    return object;
}
// vtable .
static void *__fastcall delete_argb8(Codec *object, void *, unsigned flags) {
    destroy(object);
    if (flags & 1)
        gui::application_release(object);
    return object;
}

static void __fastcall read_argb8(Codec *object, void *, Dword y, Dword z, Colour *out) {
    read_format_row<21>(object, y, z, out);
}

static void __fastcall write_argb8(Codec *object, void *, Dword y, Dword z, const Colour *in) {
    write_format_row<21>(object, y, z, in);
}
static const Dword table_argb8[4] = {
    reinterpret_cast<Dword>(delete_argb8), reinterpret_cast<Dword>(read_argb8),
    reinterpret_cast<Dword>(write_argb8), reinterpret_cast<Dword>(quantize_key)};

static Codec *construct_argb8(Codec *object, const Image *image) {
    construct(object, image, 32, 1);
    object->words[0] = reinterpret_cast<Dword>(table_argb8);
    return object;
}
// vtable .
static void *__fastcall delete_xrgb8(Codec *object, void *, unsigned flags) {
    destroy(object);
    if (flags & 1)
        gui::application_release(object);
    return object;
}

static void __fastcall read_xrgb8(Codec *object, void *, Dword y, Dword z, Colour *out) {
    read_format_row<22>(object, y, z, out);
}

static void __fastcall write_xrgb8(Codec *object, void *, Dword y, Dword z, const Colour *in) {
    write_format_row<22>(object, y, z, in);
}
static const Dword table_xrgb8[4] = {
    reinterpret_cast<Dword>(delete_xrgb8), reinterpret_cast<Dword>(read_xrgb8),
    reinterpret_cast<Dword>(write_xrgb8), reinterpret_cast<Dword>(quantize_key)};

static Codec *construct_xrgb8(Codec *object, const Image *image) {
    construct(object, image, 32, 1);
    object->words[0] = reinterpret_cast<Dword>(table_xrgb8);
    return object;
}
// vtable .
static void *__fastcall delete_rgb565(Codec *object, void *, unsigned flags) {
    destroy(object);
    if (flags & 1)
        gui::application_release(object);
    return object;
}

static void __fastcall read_rgb565(Codec *object, void *, Dword y, Dword z, Colour *out) {
    read_format_row<23>(object, y, z, out);
}

static void __fastcall write_rgb565(Codec *object, void *, Dword y, Dword z, const Colour *in) {
    write_format_row<23>(object, y, z, in);
}
static const Dword table_rgb565[4] = {
    reinterpret_cast<Dword>(delete_rgb565), reinterpret_cast<Dword>(read_rgb565),
    reinterpret_cast<Dword>(write_rgb565), reinterpret_cast<Dword>(quantize_key)};

static Codec *construct_rgb565(Codec *object, const Image *image) {
    construct(object, image, 16, 1);
    object->words[0] = reinterpret_cast<Dword>(table_rgb565);
    return object;
}
// vtable .
static void *__fastcall delete_xrgb1555(Codec *object, void *, unsigned flags) {
    destroy(object);
    if (flags & 1)
        gui::application_release(object);
    return object;
}

static void __fastcall read_xrgb1555(Codec *object, void *, Dword y, Dword z, Colour *out) {
    read_format_row<24>(object, y, z, out);
}

static void __fastcall write_xrgb1555(Codec *object, void *, Dword y, Dword z, const Colour *in) {
    write_format_row<24>(object, y, z, in);
}
static const Dword table_xrgb1555[4] = {
    reinterpret_cast<Dword>(delete_xrgb1555), reinterpret_cast<Dword>(read_xrgb1555),
    reinterpret_cast<Dword>(write_xrgb1555), reinterpret_cast<Dword>(quantize_key)};

static Codec *construct_xrgb1555(Codec *object, const Image *image) {
    construct(object, image, 16, 1);
    object->words[0] = reinterpret_cast<Dword>(table_xrgb1555);
    return object;
}
// vtable .
static void *__fastcall delete_argb1555(Codec *object, void *, unsigned flags) {
    destroy(object);
    if (flags & 1)
        gui::application_release(object);
    return object;
}

static void __fastcall read_argb1555(Codec *object, void *, Dword y, Dword z, Colour *out) {
    read_format_row<25>(object, y, z, out);
}

static void __fastcall write_argb1555(Codec *object, void *, Dword y, Dword z, const Colour *in) {
    write_format_row<25>(object, y, z, in);
}
static const Dword table_argb1555[4] = {
    reinterpret_cast<Dword>(delete_argb1555), reinterpret_cast<Dword>(read_argb1555),
    reinterpret_cast<Dword>(write_argb1555), reinterpret_cast<Dword>(quantize_key)};

static Codec *construct_argb1555(Codec *object, const Image *image) {
    construct(object, image, 16, 1);
    object->words[0] = reinterpret_cast<Dword>(table_argb1555);
    return object;
}
// vtable .
static void *__fastcall delete_argb4(Codec *object, void *, unsigned flags) {
    destroy(object);
    if (flags & 1)
        gui::application_release(object);
    return object;
}

static void __fastcall read_argb4(Codec *object, void *, Dword y, Dword z, Colour *out) {
    read_format_row<26>(object, y, z, out);
}

static void __fastcall write_argb4(Codec *object, void *, Dword y, Dword z, const Colour *in) {
    write_format_row<26>(object, y, z, in);
}
static const Dword table_argb4[4] = {
    reinterpret_cast<Dword>(delete_argb4), reinterpret_cast<Dword>(read_argb4),
    reinterpret_cast<Dword>(write_argb4), reinterpret_cast<Dword>(quantize_key)};

static Codec *construct_argb4(Codec *object, const Image *image) {
    construct(object, image, 16, 1);
    object->words[0] = reinterpret_cast<Dword>(table_argb4);
    return object;
}
// vtable .
static void *__fastcall delete_rgb332(Codec *object, void *, unsigned flags) {
    destroy(object);
    if (flags & 1)
        gui::application_release(object);
    return object;
}

static void __fastcall read_rgb332(Codec *object, void *, Dword y, Dword z, Colour *out) {
    read_format_row<27>(object, y, z, out);
}

static void __fastcall write_rgb332(Codec *object, void *, Dword y, Dword z, const Colour *in) {
    write_format_row<27>(object, y, z, in);
}
static const Dword table_rgb332[4] = {
    reinterpret_cast<Dword>(delete_rgb332), reinterpret_cast<Dword>(read_rgb332),
    reinterpret_cast<Dword>(write_rgb332), reinterpret_cast<Dword>(quantize_key)};

static Codec *construct_rgb332(Codec *object, const Image *image) {
    construct(object, image, 8, 1);
    object->words[0] = reinterpret_cast<Dword>(table_rgb332);
    return object;
}
// vtable .
static void *__fastcall delete_alpha8(Codec *object, void *, unsigned flags) {
    destroy(object);
    if (flags & 1)
        gui::application_release(object);
    return object;
}

static void __fastcall read_alpha8(Codec *object, void *, Dword y, Dword z, Colour *out) {
    read_format_row<28>(object, y, z, out);
}

static void __fastcall write_alpha8(Codec *object, void *, Dword y, Dword z, const Colour *in) {
    write_format_row<28>(object, y, z, in);
}
static const Dword table_alpha8[4] = {
    reinterpret_cast<Dword>(delete_alpha8), reinterpret_cast<Dword>(read_alpha8),
    reinterpret_cast<Dword>(write_alpha8), reinterpret_cast<Dword>(quantize_key)};

static Codec *construct_alpha8(Codec *object, const Image *image) {
    construct(object, image, 8, 1);
    object->words[0] = reinterpret_cast<Dword>(table_alpha8);
    return object;
}
// vtable .
static void *__fastcall delete_argb8332(Codec *object, void *, unsigned flags) {
    destroy(object);
    if (flags & 1)
        gui::application_release(object);
    return object;
}

static void __fastcall read_argb8332(Codec *object, void *, Dword y, Dword z, Colour *out) {
    read_format_row<29>(object, y, z, out);
}

static void __fastcall write_argb8332(Codec *object, void *, Dword y, Dword z, const Colour *in) {
    write_format_row<29>(object, y, z, in);
}
static const Dword table_argb8332[4] = {
    reinterpret_cast<Dword>(delete_argb8332), reinterpret_cast<Dword>(read_argb8332),
    reinterpret_cast<Dword>(write_argb8332), reinterpret_cast<Dword>(quantize_key)};

static Codec *construct_argb8332(Codec *object, const Image *image) {
    construct(object, image, 16, 1);
    object->words[0] = reinterpret_cast<Dword>(table_argb8332);
    return object;
}
// vtable .
static void *__fastcall delete_xrgb4(Codec *object, void *, unsigned flags) {
    destroy(object);
    if (flags & 1)
        gui::application_release(object);
    return object;
}

static void __fastcall read_xrgb4(Codec *object, void *, Dword y, Dword z, Colour *out) {
    read_format_row<30>(object, y, z, out);
}

static void __fastcall write_xrgb4(Codec *object, void *, Dword y, Dword z, const Colour *in) {
    write_format_row<30>(object, y, z, in);
}
static const Dword table_xrgb4[4] = {
    reinterpret_cast<Dword>(delete_xrgb4), reinterpret_cast<Dword>(read_xrgb4),
    reinterpret_cast<Dword>(write_xrgb4), reinterpret_cast<Dword>(quantize_key)};

static Codec *construct_xrgb4(Codec *object, const Image *image) {
    construct(object, image, 16, 1);
    object->words[0] = reinterpret_cast<Dword>(table_xrgb4);
    return object;
}
// vtable .
static void *__fastcall delete_abgr2101010(Codec *object, void *, unsigned flags) {
    destroy(object);
    if (flags & 1)
        gui::application_release(object);
    return object;
}

static void __fastcall read_abgr2101010(Codec *object, void *, Dword y, Dword z, Colour *out) {
    read_format_row<31>(object, y, z, out);
}

static void __fastcall write_abgr2101010(Codec *object, void *, Dword y, Dword z, const Colour *in) {
    write_format_row<31>(object, y, z, in);
}
static const Dword table_abgr2101010[4] = {
    reinterpret_cast<Dword>(delete_abgr2101010), reinterpret_cast<Dword>(read_abgr2101010),
    reinterpret_cast<Dword>(write_abgr2101010), reinterpret_cast<Dword>(quantize_key)};

static Codec *construct_abgr2101010(Codec *object, const Image *image) {
    construct(object, image, 32, 1);
    object->words[0] = reinterpret_cast<Dword>(table_abgr2101010);
    return object;
}
// vtable .
static void *__fastcall delete_rg16(Codec *object, void *, unsigned flags) {
    destroy(object);
    if (flags & 1)
        gui::application_release(object);
    return object;
}

static void __fastcall read_rg16(Codec *object, void *, Dword y, Dword z, Colour *out) {
    read_format_row<34>(object, y, z, out);
}

static void __fastcall write_rg16(Codec *object, void *, Dword y, Dword z, const Colour *in) {
    write_format_row<34>(object, y, z, in);
}
static const Dword table_rg16[4] = {
    reinterpret_cast<Dword>(delete_rg16), reinterpret_cast<Dword>(read_rg16),
    reinterpret_cast<Dword>(write_rg16), reinterpret_cast<Dword>(quantize_key)};

static Codec *construct_rg16(Codec *object, const Image *image) {
    construct(object, image, 32, 1);
    object->words[0] = reinterpret_cast<Dword>(table_rg16);
    return object;
}
// vtable .
static void *__fastcall delete_alpha_palette8(Codec *object, void *, unsigned flags) {
    destroy(object);
    if (flags & 1)
        gui::application_release(object);
    return object;
}

static void __fastcall read_alpha_palette8(Codec *object, void *, Dword y, Dword z, Colour *out) {
    read_format_row<40>(object, y, z, out);
}

static void __fastcall write_alpha_palette8(Codec *object, void *, Dword y, Dword z,
                                            const Colour *in) {
    write_format_row<40>(object, y, z, in);
}
static const Dword table_alpha_palette8[4] = {
    reinterpret_cast<Dword>(delete_alpha_palette8), reinterpret_cast<Dword>(read_alpha_palette8),
    reinterpret_cast<Dword>(write_alpha_palette8), reinterpret_cast<Dword>(quantize_key)};

static Codec *construct_alpha_palette8(Codec *object, const Image *image) {
    construct(object, image, 16, 1);
    object->words[0] = reinterpret_cast<Dword>(table_alpha_palette8);
    return object;
}
// vtable .
static void *__fastcall delete_palette8(Codec *object, void *, unsigned flags) {
    destroy(object);
    if (flags & 1)
        gui::application_release(object);
    return object;
}

static void __fastcall read_palette8(Codec *object, void *, Dword y, Dword z, Colour *out) {
    read_format_row<41>(object, y, z, out);
}

static void __fastcall write_palette8(Codec *object, void *, Dword y, Dword z, const Colour *in) {
    write_format_row<41>(object, y, z, in);
}
static const Dword table_palette8[4] = {
    reinterpret_cast<Dword>(delete_palette8), reinterpret_cast<Dword>(read_palette8),
    reinterpret_cast<Dword>(write_palette8), reinterpret_cast<Dword>(quantize_key)};

static Codec *construct_palette8(Codec *object, const Image *image) {
    construct(object, image, 8, 1);
    object->words[0] = reinterpret_cast<Dword>(table_palette8);
    return object;
}
// vtable .
static void *__fastcall delete_luminance8(Codec *object, void *, unsigned flags) {
    destroy(object);
    if (flags & 1)
        gui::application_release(object);
    return object;
}

static void __fastcall read_luminance8(Codec *object, void *, Dword y, Dword z, Colour *out) {
    read_format_row<50>(object, y, z, out);
}

static void __fastcall write_luminance8(Codec *object, void *, Dword y, Dword z, const Colour *in) {
    write_format_row<50>(object, y, z, in);
}
static const Dword table_luminance8[4] = {
    reinterpret_cast<Dword>(delete_luminance8), reinterpret_cast<Dword>(read_luminance8),
    reinterpret_cast<Dword>(write_luminance8), reinterpret_cast<Dword>(quantize_key)};

static Codec *construct_luminance8(Codec *object, const Image *image) {
    construct(object, image, 8, 1);
    object->words[0] = reinterpret_cast<Dword>(table_luminance8);
    return object;
}
// vtable .
static void *__fastcall delete_alpha_luminance8(Codec *object, void *, unsigned flags) {
    destroy(object);
    if (flags & 1)
        gui::application_release(object);
    return object;
}

static void __fastcall read_alpha_luminance8(Codec *object, void *, Dword y, Dword z, Colour *out) {
    read_format_row<51>(object, y, z, out);
}

static void __fastcall write_alpha_luminance8(Codec *object, void *, Dword y, Dword z,
                                              const Colour *in) {
    write_format_row<51>(object, y, z, in);
}
static const Dword table_alpha_luminance8[4] = {
    reinterpret_cast<Dword>(delete_alpha_luminance8), reinterpret_cast<Dword>(read_alpha_luminance8),
    reinterpret_cast<Dword>(write_alpha_luminance8), reinterpret_cast<Dword>(quantize_key)};

static Codec *construct_alpha_luminance8(Codec *object, const Image *image) {
    construct(object, image, 16, 1);
    object->words[0] = reinterpret_cast<Dword>(table_alpha_luminance8);
    return object;
}
// vtable .
static void *__fastcall delete_alpha_luminance4(Codec *object, void *, unsigned flags) {
    destroy(object);
    if (flags & 1)
        gui::application_release(object);
    return object;
}

static void __fastcall read_alpha_luminance4(Codec *object, void *, Dword y, Dword z, Colour *out) {
    read_format_row<52>(object, y, z, out);
}

static void __fastcall write_alpha_luminance4(Codec *object, void *, Dword y, Dword z,
                                              const Colour *in) {
    write_format_row<52>(object, y, z, in);
}
static const Dword table_alpha_luminance4[4] = {
    reinterpret_cast<Dword>(delete_alpha_luminance4), reinterpret_cast<Dword>(read_alpha_luminance4),
    reinterpret_cast<Dword>(write_alpha_luminance4), reinterpret_cast<Dword>(quantize_key)};

static Codec *construct_alpha_luminance4(Codec *object, const Image *image) {
    construct(object, image, 8, 1);
    object->words[0] = reinterpret_cast<Dword>(table_alpha_luminance4);
    return object;
}
// vtable .
static void *__fastcall delete_signed_rg8(Codec *object, void *, unsigned flags) {
    destroy(object);
    if (flags & 1)
        gui::application_release(object);
    return object;
}

static void __fastcall read_signed_rg8(Codec *object, void *, Dword y, Dword z, Colour *out) {
    read_format_row<60>(object, y, z, out);
}

static void __fastcall write_signed_rg8(Codec *object, void *, Dword y, Dword z, const Colour *in) {
    write_format_row<60>(object, y, z, in);
}
static const Dword table_signed_rg8[4] = {
    reinterpret_cast<Dword>(delete_signed_rg8), reinterpret_cast<Dword>(read_signed_rg8),
    reinterpret_cast<Dword>(write_signed_rg8), reinterpret_cast<Dword>(quantize_key)};

static Codec *construct_signed_rg8(Codec *object, const Image *image) {
    construct(object, image, 16, 2);
    object->words[0] = reinterpret_cast<Dword>(table_signed_rg8);
    return object;
}
// vtable .
static void *__fastcall delete_signed_rg5(Codec *object, void *, unsigned flags) {
    destroy(object);
    if (flags & 1)
        gui::application_release(object);
    return object;
}

static void __fastcall read_signed_rg5(Codec *object, void *, Dword y, Dword z, Colour *out) {
    read_format_row<61>(object, y, z, out);
}

static void __fastcall write_signed_rg5(Codec *object, void *, Dword y, Dword z, const Colour *in) {
    write_format_row<61>(object, y, z, in);
}
static const Dword table_signed_rg5[4] = {
    reinterpret_cast<Dword>(delete_signed_rg5), reinterpret_cast<Dword>(read_signed_rg5),
    reinterpret_cast<Dword>(write_signed_rg5), reinterpret_cast<Dword>(quantize_key)};

static Codec *construct_signed_rg5(Codec *object, const Image *image) {
    construct(object, image, 16, 2);
    object->words[0] = reinterpret_cast<Dword>(table_signed_rg5);
    return object;
}
// vtable .
static void *__fastcall delete_luminance_signed_rg8(Codec *object, void *, unsigned flags) {
    destroy(object);
    if (flags & 1)
        gui::application_release(object);
    return object;
}

static void __fastcall read_luminance_signed_rg8(Codec *object, void *, Dword y, Dword z,
                                                 Colour *out) {
    read_format_row<62>(object, y, z, out);
}

static void __fastcall write_luminance_signed_rg8(Codec *object, void *, Dword y, Dword z,
                                                  const Colour *in) {
    write_format_row<62>(object, y, z, in);
}
static const Dword table_luminance_signed_rg8[4] = {
    reinterpret_cast<Dword>(delete_luminance_signed_rg8),
    reinterpret_cast<Dword>(read_luminance_signed_rg8),
    reinterpret_cast<Dword>(write_luminance_signed_rg8), reinterpret_cast<Dword>(quantize_key)};

static Codec *construct_luminance_signed_rg8(Codec *object, const Image *image) {
    construct(object, image, 32, 2);
    object->words[0] = reinterpret_cast<Dword>(table_luminance_signed_rg8);
    return object;
}
// vtable .
static void *__fastcall delete_signed_rgba8(Codec *object, void *, unsigned flags) {
    destroy(object);
    if (flags & 1)
        gui::application_release(object);
    return object;
}

static void __fastcall read_signed_rgba8(Codec *object, void *, Dword y, Dword z, Colour *out) {
    read_format_row<63>(object, y, z, out);
}

static void __fastcall write_signed_rgba8(Codec *object, void *, Dword y, Dword z, const Colour *in) {
    write_format_row<63>(object, y, z, in);
}
static const Dword table_signed_rgba8[4] = {
    reinterpret_cast<Dword>(delete_signed_rgba8), reinterpret_cast<Dword>(read_signed_rgba8),
    reinterpret_cast<Dword>(write_signed_rgba8), reinterpret_cast<Dword>(quantize_key)};

static Codec *construct_signed_rgba8(Codec *object, const Image *image) {
    construct(object, image, 32, 3);
    object->words[0] = reinterpret_cast<Dword>(table_signed_rgba8);
    return object;
}
// vtable .
static void *__fastcall delete_signed_rg16(Codec *object, void *, unsigned flags) {
    destroy(object);
    if (flags & 1)
        gui::application_release(object);
    return object;
}

static void __fastcall read_signed_rg16(Codec *object, void *, Dword y, Dword z, Colour *out) {
    read_format_row<64>(object, y, z, out);
}

static void __fastcall write_signed_rg16(Codec *object, void *, Dword y, Dword z, const Colour *in) {
    write_format_row<64>(object, y, z, in);
}
static const Dword table_signed_rg16[4] = {
    reinterpret_cast<Dword>(delete_signed_rg16), reinterpret_cast<Dword>(read_signed_rg16),
    reinterpret_cast<Dword>(write_signed_rg16), reinterpret_cast<Dword>(quantize_key)};

static Codec *construct_signed_rg16(Codec *object, const Image *image) {
    construct(object, image, 32, 2);
    object->words[0] = reinterpret_cast<Dword>(table_signed_rg16);
    return object;
}
// vtable .
static void *__fastcall delete_signed_rgb655(Codec *object, void *, unsigned flags) {
    destroy(object);
    if (flags & 1)
        gui::application_release(object);
    return object;
}

static void __fastcall read_signed_rgb655(Codec *object, void *, Dword y, Dword z, Colour *out) {
    read_format_row<65>(object, y, z, out);
}

static void __fastcall write_signed_rgb655(Codec *object, void *, Dword y, Dword z,
                                           const Colour *in) {
    write_format_row<65>(object, y, z, in);
}
static const Dword table_signed_rgb655[4] = {
    reinterpret_cast<Dword>(delete_signed_rgb655), reinterpret_cast<Dword>(read_signed_rgb655),
    reinterpret_cast<Dword>(write_signed_rgb655), reinterpret_cast<Dword>(quantize_key)};

static Codec *construct_signed_rgb655(Codec *object, const Image *image) {
    construct(object, image, 32, 2);
    object->words[0] = reinterpret_cast<Dword>(table_signed_rgb655);
    return object;
}
// vtable .
static void *__fastcall delete_alpha_signed_rgb10(Codec *object, void *, unsigned flags) {
    destroy(object);
    if (flags & 1)
        gui::application_release(object);
    return object;
}

static void __fastcall read_alpha_signed_rgb10(Codec *object, void *, Dword y, Dword z, Colour *out) {
    read_format_row<67>(object, y, z, out);
}

static void __fastcall write_alpha_signed_rgb10(Codec *object, void *, Dword y, Dword z,
                                                const Colour *in) {
    write_format_row<67>(object, y, z, in);
}
static const Dword table_alpha_signed_rgb10[4] = {reinterpret_cast<Dword>(delete_alpha_signed_rgb10),
                                                 reinterpret_cast<Dword>(read_alpha_signed_rgb10),
                                                 reinterpret_cast<Dword>(write_alpha_signed_rgb10),
                                                 reinterpret_cast<Dword>(quantize_key)};

static Codec *construct_alpha_signed_rgb10(Codec *object, const Image *image) {
    construct(object, image, 32, 2);
    object->words[0] = reinterpret_cast<Dword>(table_alpha_signed_rgb10);
    return object;
}
// vtable .
static void *__fastcall delete_luminance16(Codec *object, void *, unsigned flags) {
    destroy(object);
    if (flags & 1)
        gui::application_release(object);
    return object;
}

static void __fastcall read_luminance16(Codec *object, void *, Dword y, Dword z, Colour *out) {
    read_format_row<70>(object, y, z, out);
}

static void __fastcall write_luminance16(Codec *object, void *, Dword y, Dword z, const Colour *in) {
    write_format_row<70>(object, y, z, in);
}
static const Dword table_luminance16[4] = {
    reinterpret_cast<Dword>(delete_luminance16), reinterpret_cast<Dword>(read_luminance16),
    reinterpret_cast<Dword>(write_luminance16), reinterpret_cast<Dword>(quantize_key)};

static Codec *construct_luminance16(Codec *object, const Image *image) {
    construct(object, image, 16, 1);
    object->words[0] = reinterpret_cast<Dword>(table_luminance16);
    return object;
}
// vtable .
static void *__fastcall delete_luminance16_private(Codec *object, void *, unsigned flags) {
    destroy(object);
    if (flags & 1)
        gui::application_release(object);
    return object;
}

static void __fastcall read_luminance16_private(Codec *object, void *, Dword y, Dword z,
                                                Colour *out) {
    read_format_row<909200416>(object, y, z, out);
}

static void __fastcall write_luminance16_private(Codec *object, void *, Dword y, Dword z,
                                                 const Colour *in) {
    write_format_row<909200416>(object, y, z, in);
}
static const Dword table_luminance16_private[4] = {
    reinterpret_cast<Dword>(delete_luminance16_private),
    reinterpret_cast<Dword>(read_luminance16_private),
    reinterpret_cast<Dword>(write_luminance16_private), reinterpret_cast<Dword>(quantize_key)};

static Codec *construct_luminance16_private(Codec *object, const Image *image) {
    construct(object, image, 16, 1);
    object->words[0] = reinterpret_cast<Dword>(table_luminance16_private);
    return object;
}
// vtable .
static void *__fastcall delete_alpha_luminance16(Codec *object, void *, unsigned flags) {
    destroy(object);
    if (flags & 1)
        gui::application_release(object);
    return object;
}

static void __fastcall read_alpha_luminance16(Codec *object, void *, Dword y, Dword z, Colour *out) {
    read_format_row<909200449>(object, y, z, out);
}

static void __fastcall write_alpha_luminance16(Codec *object, void *, Dword y, Dword z,
                                               const Colour *in) {
    write_format_row<909200449>(object, y, z, in);
}
static const Dword table_alpha_luminance16[4] = {reinterpret_cast<Dword>(delete_alpha_luminance16),
                                                reinterpret_cast<Dword>(read_alpha_luminance16),
                                                reinterpret_cast<Dword>(write_alpha_luminance16),
                                                reinterpret_cast<Dword>(quantize_key)};

static Codec *construct_alpha_luminance16(Codec *object, const Image *image) {
    construct(object, image, 32, 1);
    object->words[0] = reinterpret_cast<Dword>(table_alpha_luminance16);
    return object;
}
// vtable .
static void *__fastcall delete_rgb16(Codec *object, void *, unsigned flags) {
    destroy(object);
    if (flags & 1)
        gui::application_release(object);
    return object;
}

static void __fastcall read_rgb16(Codec *object, void *, Dword y, Dword z, Colour *out) {
    read_format_row<909201952>(object, y, z, out);
}

static void __fastcall write_rgb16(Codec *object, void *, Dword y, Dword z, const Colour *in) {
    write_format_row<909201952>(object, y, z, in);
}
static const Dword table_rgb16[4] = {
    reinterpret_cast<Dword>(delete_rgb16), reinterpret_cast<Dword>(read_rgb16),
    reinterpret_cast<Dword>(write_rgb16), reinterpret_cast<Dword>(quantize_key)};

static Codec *construct_rgb16(Codec *object, const Image *image) {
    construct(object, image, 48, 1);
    object->words[0] = reinterpret_cast<Dword>(table_rgb16);
    return object;
}
// vtable .
static void *__fastcall delete_argb16(Codec *object, void *, unsigned flags) {
    destroy(object);
    if (flags & 1)
        gui::application_release(object);
    return object;
}

static void __fastcall read_argb16(Codec *object, void *, Dword y, Dword z, Colour *out) {
    read_format_row<909201985>(object, y, z, out);
}

static void __fastcall write_argb16(Codec *object, void *, Dword y, Dword z, const Colour *in) {
    write_format_row<909201985>(object, y, z, in);
}
static const Dword table_argb16[4] = {
    reinterpret_cast<Dword>(delete_argb16), reinterpret_cast<Dword>(read_argb16),
    reinterpret_cast<Dword>(write_argb16), reinterpret_cast<Dword>(quantize_key)};

static Codec *construct_argb16(Codec *object, const Image *image) {
    construct(object, image, 64, 1);
    object->words[0] = reinterpret_cast<Dword>(table_argb16);
    return object;
}
const Dword linear_formats[30] = {20u, 21u, 22u,        23u,        24u,        25u,       26u, 27u,
                                 28u, 29u, 30u,        31u,        34u,        40u,       41u, 50u,
                                 51u, 52u, 60u,        61u,        62u,        63u,       64u, 65u,
                                 67u, 70u, 909200416u, 909200449u, 909201952u, 909201985u};
const Dword *const linear_tables[30] = {table_rgb8,
                                       table_argb8,
                                       table_xrgb8,
                                       table_rgb565,
                                       table_xrgb1555,
                                       table_argb1555,
                                       table_argb4,
                                       table_rgb332,
                                       table_alpha8,
                                       table_argb8332,
                                       table_xrgb4,
                                       table_abgr2101010,
                                       table_rg16,
                                       table_alpha_palette8,
                                       table_palette8,
                                       table_luminance8,
                                       table_alpha_luminance8,
                                       table_alpha_luminance4,
                                       table_signed_rg8,
                                       table_signed_rg5,
                                       table_luminance_signed_rg8,
                                       table_signed_rgba8,
                                       table_signed_rg16,
                                       table_signed_rgb655,
                                       table_alpha_signed_rgb10,
                                       table_luminance16,
                                       table_luminance16_private,
                                       table_alpha_luminance16,
                                       table_rgb16,
                                       table_argb16};
Codec *construct_linear(Codec *object, const Image *image, Dword format) {
    switch (format) {
    case 20u:
        return construct_rgb8(object, image);
    case 21u:
        return construct_argb8(object, image);
    case 22u:
        return construct_xrgb8(object, image);
    case 23u:
        return construct_rgb565(object, image);
    case 24u:
        return construct_xrgb1555(object, image);
    case 25u:
        return construct_argb1555(object, image);
    case 26u:
        return construct_argb4(object, image);
    case 27u:
        return construct_rgb332(object, image);
    case 28u:
        return construct_alpha8(object, image);
    case 29u:
        return construct_argb8332(object, image);
    case 30u:
        return construct_xrgb4(object, image);
    case 31u:
        return construct_abgr2101010(object, image);
    case 34u:
        return construct_rg16(object, image);
    case 40u:
        return construct_alpha_palette8(object, image);
    case 41u:
        return construct_palette8(object, image);
    case 50u:
        return construct_luminance8(object, image);
    case 51u:
        return construct_alpha_luminance8(object, image);
    case 52u:
        return construct_alpha_luminance4(object, image);
    case 60u:
        return construct_signed_rg8(object, image);
    case 61u:
        return construct_signed_rg5(object, image);
    case 62u:
        return construct_luminance_signed_rg8(object, image);
    case 63u:
        return construct_signed_rgba8(object, image);
    case 64u:
        return construct_signed_rg16(object, image);
    case 65u:
        return construct_signed_rgb655(object, image);
    case 67u:
        return construct_alpha_signed_rgb10(object, image);
    case 70u:
        return construct_luminance16(object, image);
    case 909200416u:
        return construct_luminance16_private(object, image);
    case 909200449u:
        return construct_alpha_luminance16(object, image);
    case 909201952u:
        return construct_rgb16(object, image);
    case 909201985u:
        return construct_argb16(object, image);
    }
    return nullptr;
}
} // namespace nfs::graphics::surfaces::pixels
