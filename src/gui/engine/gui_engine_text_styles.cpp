#include "../lifecycle/gui_font_styles.hpp"
#include "nfs/gui/resources/gui_format.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include "../lifecycle/gui_render_numeric_x87.hpp"
#include "../lifecycle/gui_font_metrics.hpp"
#include <cstring>

#pragma STDC FENV_ACCESS ON
namespace nfs::gui::engine {
namespace {
long double retained_ascent(int index, float scale) {
    auto *font = get_font(index);
    if (!font)
        return 0.0L;
    const int height = font->ascent ? font->ascent : int(fonts::height(font));
    return gui::retained::integer(height) * scale;
}
long double scaled_height(Engine *engine, int index, float scale) {
    auto *font = engine->context ? get_font(index) : nullptr;
    return font ? gui::retained::integer(int(fonts::height(font))) * scale : 0.0L;
}
}

void font_ascent_native80(int index, float scale, void *output) {
    const auto result = retained_ascent(index, scale);
    std::memcpy(output, &result, 10);
}

void initialize_font_metrics(Engine *engine, FontStyle *style) {
    const float scale_y = style->scale_y;
    const int font_index = style->font;
    const auto height = scaled_height(engine, font_index, scale_y);
    style->height = float(height);
    if (height != 0.0L) {
        const auto ascent = retained_ascent(style->font, style->scale_y);
        style->spacing = float(ascent / style->height);
    }
    if (style->spacing == 0.0f)
        style->spacing = 0.7799999713897705078125f;
}

void normalize_font_scale(FontStyle *style, float ratio) {
    const auto scale = 1.0L / ratio;
    style->scale_x = float(gui::retained::number(style->scale_x) * scale);
    style->scale_y = float(scale * style->scale_y);
}
} // namespace nfs::gui::engine

namespace nfs::gui::engine::pending {
using game::runtime::at;
using gui::retained::number;
using gui::retained::integer;
namespace {
FontStyleRecord *style_record(Engine *engine, Dword handle) {
    return static_cast<FontStyleRecord *>(
        container_find(&engine->font_styles, compare_font_style, reinterpret_cast<void *>(handle)));
}
} // namespace

static Dword *font_settings(int index) {
    auto *face = get_font_face(index);
    return face ? &face->style_2c : nullptr;
}

String *font_name(String *out, int index) {
    String filename;
    string_construct_ascii(&filename, reinterpret_cast<const char *>(get_font_face(index)));
    try {
        if (!filename.length)
            string_construct_ascii(out, "");
        else {
            String stem;
            string_construct(&stem);
            try {
                const char *text = string_ascii(&filename);
                int length = 0;
                while (length < filename.length && text[length] != '.')
                    ++length;
                if (length == filename.length)
                    length = -1;
                string_substring(&filename, 0, length, &stem);
                string_construct_copy(out, &stem);
            } catch (...) {
                string_destruct(&stem);
                throw;
            }
            string_destruct(&stem);
        }
    } catch (...) {
        string_destruct(&filename);
        throw;
    }
    string_destruct(&filename);
    return out;
}

Dword __fastcall query_font_style(Engine *engine, void *, Dword handle, Dword point_height_out,
                                 Dword bold_out, Dword italic_out, Dword underline_out,
                                 Dword shadow_out, Dword strike_out, Dword face_out, Dword ascent_out,
                                 Dword height_out) {
    auto *record = handle ? style_record(engine, handle) : nullptr;
    const FontStyle *style = record ? &record->style : &engine->default_font;
    if (point_height_out) {
        const float scale_y = style->scale_y;
        const int font_index = style->font;
        const float height = float(scaled_height(engine, font_index, 1));
        if (height == 0.0f) {
            *reinterpret_cast<Dword *>(point_height_out) = 0;
        } else {
            const auto ratio = retained_ascent(font_index, 1) / height;
            const auto vertical_ratio = number(vertical_scale) * ratio;
            const auto scaled_height_value = vertical_ratio * height;
            const auto scaled_size = scaled_height_value * scale_y;
            *reinterpret_cast<Dword *>(point_height_out) = gui::retained::truncate(scaled_size * 0.75f);
        }
    }
    const auto flag = [&](Dword output, Dword bit) {
        if (output)
            *reinterpret_cast<Dword *>(output) = (style->flags & bit) != 0;
    };
    flag(bold_out, 2);
    flag(italic_out, 1);
    flag(underline_out, 8);
    flag(shadow_out, 16);
    flag(strike_out, 4);
    if (face_out) {
        String name;
        font_name(&name, style->font);
        try {
            string_copy(reinterpret_cast<String *>(face_out), &name);
        } catch (...) {
            string_destruct(&name);
            throw;
        }
        string_destruct(&name);
    }
    if (ascent_out)
        *reinterpret_cast<Dword *>(ascent_out) =
            gui::retained::truncate(number(style->height) * style->spacing);
    if (height_out)
        *reinterpret_cast<Dword *>(height_out) =
            gui::retained::truncate(number(style->height));
    return 1;
}

Dword __fastcall update_font_style(Engine *engine, void *, Dword handle, Dword point_height, Dword bold,
                                  Dword italic, Dword underline, Dword shadow, Dword strike,
                                  Dword face_name) {
    if (!point_height || !handle)
        return 0;
    auto *record = style_record(engine, handle);
    FontStyle *style = record ? &record->style : nullptr;
    const auto flag = [&](Dword enabled, Dword bit) {
        style->flags = enabled ? style->flags | bit : style->flags & ~bit;
    };
    flag(bold, 2);
    flag(italic, 1);
    flag(shadow, 16);
    flag(strike, 4);
    flag(underline, 8);
    style->font = find_font(reinterpret_cast<const String *>(face_name));
    const Dword *settings = font_settings(style->font);
    bool scaled = false;
    if (settings && !settings[4]) {
        const int font_index = style->font;
        const float height = float(scaled_height(engine, font_index, 1));
        const auto ascent = retained_ascent(font_index, 1);
        const auto reciprocal = 1.0L / height;
        const float stored_reciprocal = float(reciprocal);
        const auto ratio = ascent * reciprocal;
        if (ratio != 0.0L) {
            int signed_point_height;
            std::memcpy(&signed_point_height, &point_height, sizeof(signed_point_height));
            const auto size = integer(signed_point_height) * stored_reciprocal * 1.33333337306976318359375f;
            style->scale_x = float(size / (number(horizontal_scale) * ratio));
            style->scale_y = float(size / (number(vertical_scale) * ratio));
            scaled = true;
        }
    }
    if (!scaled) {
        style->scale_x = 1;
        style->scale_y = 1;
    }
    auto height = 0.0L;
    if (engine->context)
        if (auto *font = get_font(style->font)) {
            const int pixel_height = int(fonts::height(font));
            height = integer(pixel_height) * style->scale_y;
        }
    const float stored_height = float(height);
    style->height = stored_height;
    if (height != 0.0L)
        style->spacing = float(retained_ascent(style->font, style->scale_y) / stored_height);
    if (style->spacing == 0.0f)
        style->spacing = 0.7799999713897705078125f;
    return 1;
}

Dword __fastcall create_font_style(Engine *engine, void *, Dword point_height, Dword bold, Dword italic,
                                  Dword underline, Dword shadow, Dword strike, Dword face_name,
                                  Dword style_name, Dword reuse_unnamed) {
    const auto *name = reinterpret_cast<const String *>(style_name);
    if (name->length && string_data(name))
        for (Dword index = 0; index < engine->font_styles.count; ++index)
            if (auto *record = static_cast<FontStyleRecord *>(
                    events::container_value(&engine->font_styles, index)))
                if (string_equal(&record->name, name))
                    return 0;
    FontStyle style{
        1, 1, 0, 0, 0.7799999713897705078125f, find_font(reinterpret_cast<const String *>(face_name)), 0};
    const Dword *settings = font_settings(style.font);
    if (!settings)
        return 0;
    if (settings[2] != 1)
        bold = 0;
    if (settings[3] != 1)
        italic = 0;
    if (settings[4])
        point_height = settings[1];
    settings = font_settings(style.font);
    if (point_height && settings && !settings[4]) {
        const auto height = scaled_height(engine, style.font, 1);
        const float reciprocal = float(1.0L / height);
        const auto ratio = retained_ascent(style.font, 1) * reciprocal;
        if (ratio != 0.0L) {
            int signed_point_height;
            std::memcpy(&signed_point_height, &point_height, sizeof(signed_point_height));
            const auto size = integer(signed_point_height) * reciprocal * 1.33333337306976318359375f;
            style.scale_x = float(size / (ratio * number(horizontal_scale)));
            style.scale_y = float(size / (ratio * number(vertical_scale)));
        }
    }
    style.flags = (bold ? 2 : 0) | (italic ? 1 : 0) | (shadow ? 16 : 0) | (strike ? 4 : 0) |
                  (underline ? 8 : 0);
    if (reuse_unnamed && !string_data(name))
        for (Dword index = 0; index < engine->font_styles.count; ++index)
            if (auto *record = static_cast<FontStyleRecord *>(
                    events::container_value(&engine->font_styles, index)))
                if (record->style.font == style.font && record->style.scale_x == style.scale_x &&
                    record->style.scale_y == style.scale_y && record->style.flags == style.flags)
                    return record->handle;
    auto *record = create_font_style_record(name);
    if (!record)
        return 0;
    const auto height = scaled_height(engine, style.font, style.scale_y);
    style.height = float(height);
    if (height != 0.0L) {
        const auto spacing = retained_ascent(style.font, style.scale_y) / style.height;
        style.spacing = float(spacing);
        if (spacing == 0.0L)
            style.spacing = 0.7799999713897705078125f;
    }
    style.handle = record->handle;
    record->style = style;
    container_append(&engine->font_styles, record, dispose_object);
    return record->handle;
}
} // namespace nfs::gui::engine::pending
