#include "nfs/gui/engine/gui_engine.hpp"
#include "nfs/gui/resources/gui_format.hpp"
#include "nfs/graphics/display/viewport_transform.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include "../lifecycle/gui_render_numeric_x87.hpp"
#include "../lifecycle/gui_font_styles.hpp"
namespace gui_numeric = ::nfs::gui::retained;
namespace nfs::gui::engine::pending {
using game::runtime::at;
using game::runtime::bytes;
using game::runtime::invoke;

int compare_font_style(const void *record, const void *key) {
    const Dword handle = at<Dword>(record, 24), wanted = reinterpret_cast<Dword>(key);
    return handle < wanted ? -1 : wanted < handle;
}

static int compare_palette(const void *record, const void *key) {
    const Dword handle = at<Dword>(record, 72), wanted = reinterpret_cast<Dword>(key);
    return handle < wanted ? -1 : wanted < handle;
}

static int compare_texture(const void *record, const void *key) {
    const Dword handle = at<Dword>(record, 28), wanted = reinterpret_cast<Dword>(key);
    return handle < wanted ? -1 : wanted < handle;
}

static int compare_image(const void *record, const void *key) {
    const Dword handle = at<Dword>(record, 4), wanted = reinterpret_cast<Dword>(key);
    return handle < wanted ? -1 : wanted < handle;
}
namespace {
void *lookup(Container *objects, Compare compare, Dword handle) {
    return container_find(objects, compare, reinterpret_cast<void *>(handle));
}
const String *empty_string() {
    return &lifecycle::construction::unresolved::shared_string;
}
Dword find_named(Container *objects, unsigned name_offset, unsigned handle_offset,
                const String *name) {
    const Dword count = objects->count;
    for (Dword index = 0; index < count; ++index)
        if (void *record = events::container_value(objects, index))
            if (string_equal(reinterpret_cast<const String *>(bytes(record) + name_offset), name))
                return at<Dword>(record, handle_offset);
    return 0;
}
void release_reference(Container *objects, Compare compare, Dword handle,
                       unsigned reference_offset) {
    if (!handle)
        return;
    const Dword index = container_find_index(objects, compare, reinterpret_cast<void *>(handle));
    if (int(index) >= 0 && index < objects->count)
        if (void *record = events::container_value(objects, index))
            if (!--at<Dword>(record, reference_offset))
                container_remove_at(objects, index);
}
} // namespace

void __fastcall visit_font_styles(Engine *engine, void *,
                                  void(__cdecl *visitor)(void *, Dword, Dword, Dword), Dword first,
                                  Dword second) {
    if (engine->context && visitor)
        for (Dword index = 0; index < engine->font_styles.count; ++index)
            if (auto *record = static_cast<FontStyleRecord *>(
                    events::container_value(&engine->font_styles, index)))
                visitor(&record->name, record->handle, first, second);
}
// Font visitor takes THREE arguments, unlike the style visitor.
void __fastcall visit_fonts(Engine *engine, void *, void(__cdecl *visitor)(void *, Dword, Dword),
                            Dword first, Dword second) {
    if (!engine->context || !visitor)
        return;
    for (int index = 0; index < at<int>(game::runtime::system(20), 8); ++index) {
        String filename;
        string_construct(&filename);
        try {
            string_assign(&filename, reinterpret_cast<const char *>(get_font_face(index)));
            if (filename.length) {
                const char *text = string_ascii(&filename);
                int length = 0;
                while (length < filename.length && text[length] != '.')
                    ++length;
                if (length == filename.length)
                    length = -1;
                string_substring(&filename, 0, length, &filename);
                visitor(&filename, first, second);
            }
        } catch (...) {
            string_destruct(&filename);
            throw;
        }
        string_destruct(&filename);
    }
}

Dword __fastcall find_font_style(Engine *engine, void *, const String *name) {
    return engine->context && string_data(name) ? find_named(&engine->font_styles, 4, 24, name)
                                                : 0;
}

String *__fastcall font_style_name(Engine *engine, void *, String *out, Dword handle) {
    for (Dword index = 0; index < engine->font_styles.count; ++index)
        if (auto *record = static_cast<FontStyleRecord *>(
                events::container_value(&engine->font_styles, index)))
            if (record->handle == handle)
                return string_construct_copy(out, &record->name);
    return string_construct_copy(out, empty_string());
}

Dword __fastcall find_palette(Engine *engine, void *, const String *name) {
    if (!string_data(name) || !string_data(name))
        return 0;
    return find_named(&engine->palettes, 52, 72, name);
}

String *__fastcall palette_name(Engine *engine, void *, String *out, Dword handle) {
    auto *palette = handle
                        ? static_cast<Palette *>(lookup(&engine->palettes, compare_palette, handle))
                        : nullptr;
    return string_construct_copy(out, palette ? &palette->name : empty_string());
}

Dword __fastcall select_palette(Engine *engine, void *, Dword handle) {
    const Dword previous = engine->palette->handle;
    if (previous != handle) {
        auto *palette =
            handle ? static_cast<Palette *>(lookup(&engine->palettes, compare_palette, handle))
                   : nullptr;
        engine->palette = palette ? palette : &engine->default_palette;
    }
    return previous;
}

Dword __fastcall copy_palette(Engine *engine, void *, Dword colours, Dword count, Dword text) {
    const auto *name = reinterpret_cast<const String *>(text);
    if (name->length && string_data(name)) {
        const Dword length = engine->palettes.count;
        for (Dword index = 0; index < length; ++index)
            if (auto *palette =
                    static_cast<Palette *>(events::container_value(&engine->palettes, index)))
                if (string_equal(&palette->name, name))
                    return 0;
    }
    auto *palette = static_cast<Palette *>(allocate(sizeof(Palette)));
    if (palette)
        construct_palette(palette);
    string_copy(&palette->name, name);
    const auto *source = reinterpret_cast<const Dword *>(colours);
    auto *destination = palette->colours;
    for (; count; --count)
        *destination++ = *source++;
    container_append(&engine->palettes, palette, dispose_object);
    return palette->handle;
}

Dword __fastcall select_font_style(Engine *engine, void *, Dword handle) {
    const Dword previous = engine->font->handle;
    if (previous != handle) {
        auto *record = handle ? static_cast<FontStyleRecord *>(
                                    lookup(&engine->font_styles, compare_font_style, handle))
                              : nullptr;
        engine->font = record ? &record->style : &engine->default_font;
    }
    return previous;
}

Dword __fastcall font_pixel_height(Engine *engine, void *) {

    return Dword(gui_numeric::truncate(gui_numeric::number(engine->font->height) *
                                      gui_numeric::number(engine->scale_y)));
}

Dword __fastcall glyph_metrics(Engine *engine, void *, Dword code, Dword *out) {

    const float scale_x =
        float(gui_numeric::number(engine->scale_x) * gui_numeric::number(engine->font->scale_x) *
              gui_numeric::number(horizontal_scale));
    const float scale_y =
        float(gui_numeric::number(engine->font->scale_y) * gui_numeric::number(engine->scale_y) *
              gui_numeric::number(vertical_scale));
    auto *font = get_font(engine->font->font);
    if (!font)
        return 0;
    if (code == 10) {
        out[1] = out[2] = out[3] = 0;
        out[0] = 10;
        out[4] = font->ascent;
        out[5] = font->descent;
        return 1;
    }
    if (code == 4294967200u)
        return 0;
    const Dword character =
        code == (code & 255) || (code & 4278190080u) ? code & 255 : code & 65535;
    const auto *glyphs = reinterpret_cast<const Byte *>(font) + font->glyph_offset;
    const int stride = font->flags & 262144 ? 16 : 12;
    const int offset = int(character - 32);
    const Byte *glyph = nullptr;
    if (offset < font->count) {
        const auto *candidate = reinterpret_cast<const Byte *>(reinterpret_cast<Dword>(glyphs) +
                                                               Dword(offset) * Dword(stride));
        if (*reinterpret_cast<const std::uint16_t *>(candidate) == character)
            glyph = candidate;
    }
    if (!glyph)
        glyph = fonts::find_code(character, glyphs, font->count, stride);
    if (!glyph)
        return 0;
    const int advance = font->flags & 262144
                            ? *reinterpret_cast<const std::int16_t *>(glyph + 14)
                            : int(*reinterpret_cast<const std::int8_t *>(glyph + 8));
    out[0] = code;
    out[1] =
        Dword(gui_numeric::truncate(gui_numeric::integer(glyph[2]) * gui_numeric::number(scale_x)));
    out[2] =
        Dword(gui_numeric::truncate(gui_numeric::integer(glyph[3]) * gui_numeric::number(scale_y)));
    out[3] =
        Dword(gui_numeric::truncate(gui_numeric::integer(advance) * gui_numeric::number(scale_x)));
    out[5] = font->descent;
    out[4] = font->ascent;
    return 1;
}

Dword __fastcall acquire_texture(Engine *engine, void *, Dword handle) {
    if (handle)
        if (void *texture = lookup(&engine->textures, compare_texture, handle)) {
            ++at<Dword>(texture, 24);
            return handle;
        }
    return 0;
}

void __fastcall release_texture(Engine *engine, void *, Dword handle) {
    release_reference(&engine->textures, compare_texture, handle, 24);
}

Dword __fastcall texture_frame_count(Engine *engine, void *, Dword handle, Dword *out) {
    if (handle)
        if (void *texture = lookup(&engine->textures, compare_texture, handle)) {
            if (out)
                *out = at<Dword>(texture, 16);
            return 1;
        }
    return 0;
}
void *texture_frame_object(void *, int, Dword);

Dword __fastcall texture_frame(Engine *engine, void *, Dword handle, int index) {
    if (handle)
        if (void *texture = lookup(&engine->textures, compare_texture, handle))
            if (index >= 0 && index < at<int>(texture, 16))
                if (void *frame = texture_frame_object(texture, index, 1))
                    return at<Dword>(frame, 4);
    return 0;
}

Byte __fastcall image_uses_texture(Engine *engine, void *, Dword texture, Dword image) {
    if (texture && image)
        if (void *record = lookup(&engine->images, compare_image, image)) {
            const auto *name = reinterpret_cast<const String *>(bytes(record) + 12);
            if (!name->length)
                return at<Dword>(record, 32) == texture;
            const void *data = string_data(name);
            if (data && !(name->wide ? Dword(*static_cast<const std::uint16_t *>(data))
                                     : Dword(int(*static_cast<const signed char *>(data)))))
                return at<Dword>(record, 32) == texture;
        }
    return 0;
}

Dword __fastcall acquire_image(Engine *engine, void *, Dword handle) {
    if (handle)
        if (void *image = lookup(&engine->images, compare_image, handle)) {
            ++at<Dword>(image, 8);
            return handle;
        }
    return 0;
}

void __fastcall release_image(Engine *engine, void *, Dword handle) {
    release_reference(&engine->images, compare_image, handle, 8);
}

Dword __fastcall query_image(Engine *engine, void *, Dword handle, Dword first, Dword second) {
    if (handle)
        if (void *image = lookup(&engine->images, compare_image, handle))
            return invoke<Dword>(image, 3, first, second);
    return 0;
}
Dword update_scissor(Engine *);
namespace {
struct Coordinate {
    const Destroy *methods;
    Dword x, y;
};
} // namespace

static void destroy_coordinate(Coordinate *point) {
    point->methods = gui::base_methods;
}

static Coordinate *__fastcall delete_coordinate(Coordinate *point, void *, unsigned flags) {
    destroy_coordinate(point);
    if (flags & 1)
        gui::release(point);
    return point;
}
extern const Destroy coordinate_methods[] = {
    reinterpret_cast<Destroy>(delete_coordinate)};

Dword __fastcall control_rendering(Engine *engine, void *, Dword operation, Dword first, Dword second) {
    if (engine->context)
        switch (operation) {
        case 1:
            update_scissor(engine);
            break;
        case 5:
            if (first)
                update_scissor(engine);
            graphics::viewports::begin(engine->viewport);
            return 1;
        case 6:
            graphics::viewports::end(engine->viewport);
            return 1;
        case 7:
            if (!second) {
                Coordinate first_point{coordinate_methods, 0, 0},
                    second_point{coordinate_methods, 0, 0};
                Dword result = 0;
                if (first)
                    if (void *image = lookup(&engine->images, compare_image, first)) {
                        result = invoke<Dword>(image, 5);
                        if (result && !invoke<Dword>(image, 6, &first_point, &second_point))
                            result = 0;
                    }
                destroy_coordinate(&second_point);
                destroy_coordinate(&first_point);
                return result;
            } else if (first)
                if (void *image = lookup(&engine->images, compare_image, first)) {
                    Dword result = invoke<Dword>(image, 5);
                    if (result && invoke<Dword>(image, 6, reinterpret_cast<void *>(second),
                                               reinterpret_cast<void *>(second + 12)))
                        return result;
                }
        }
    return 0;
}
} // namespace nfs::gui::engine::pending
