#include "../lifecycle/gui_font_styles.hpp"
#include "../lifecycle/gui_configuration_values.hpp"
#include "nfs/gui/resources/gui_format.hpp"
#include "nfs/resources/ini/ini.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include <cstdio>
namespace nfs::gui::engine::pending {
using game::runtime::at;
using game::runtime::invoke;

static Dword configuration_read_only(void *configuration) {
    auto *document = at<ini::Document *>(configuration, 48);
    return document ? ini::read_only(document) : 1;
}

Dword write_text(void *configuration, const char *key, const char *value, const char *fallback) {
    return write_ascii(configuration, key, value, "STR", 0, fallback);
}

Dword write_float(void *configuration, const char *key, float value, int digits, float fallback) {
    char format[256], buffer[256];
    std::sprintf(format, "%s%d%s", "%0.", digits, "f");
    std::sprintf(buffer, format, double(value));
    String text;
    string_construct_ascii(&text, buffer);
    Dword result;
    try {
        result =
            invoke<Dword>(configuration, 7, key, &text, "FLOAT", Dword(0), Dword(value == fallback));
    } catch (...) {
        string_destruct(&text);
        throw;
    }
    string_destruct(&text);
    return result;
}

Dword __fastcall save_font_style(Engine *engine, void *, const String *name) {
    if (!string_data(name))
        return 6;
    FontStyle *style = nullptr;
    for (Dword index = 0; index < engine->font_styles.count; ++index)
        if (auto *record = static_cast<FontStyleRecord *>(
                events::container_value(&engine->font_styles, index)))
            if (string_equal(&record->name, name)) {
                style = &record->style;
                break;
            }
    if (!style || !string_data(name))
        return 6;
    alignas(4) Byte configuration[56];
    construct_configuration_for_file(configuration, "system\\fonts.gdc");
    Dword result = 0;
    try {
        if (configuration_read_only(configuration))
            result = 14;
        else {
            select_section(configuration, string_ascii(name));
            select_section(configuration, string_ascii(name));
            String face;
            font_name(&face, style->font);
            try {
                write_text(configuration, "face", string_ascii(&face), nullptr);
            } catch (...) {
                string_destruct(&face);
                throw;
            }
            string_destruct(&face);
            write_float(configuration, "scaleX", style->scale_x, 4, 0);
            write_float(configuration, "scaleY", style->scale_y, 4, 0);
            write_flags(configuration, "style", style->flags, font_flags, 0);
        }
    } catch (...) {
        destroy_configuration(configuration);
        throw;
    }
    destroy_configuration(configuration);
    return result;
}

Dword __fastcall save_palette(Engine *engine, void *, const String *name) {
    if (!string_data(name))
        return 6;
    Palette *palette = nullptr;
    const Dword count = engine->palettes.count;
    for (Dword index = 0; index < count; ++index)
        if (auto *candidate =
                static_cast<Palette *>(events::container_value(&engine->palettes, index)))
            if (string_equal(&candidate->name, name)) {
                palette = candidate;
                break;
            }
    if (!palette)
        return 6;
    alignas(4) Byte configuration[56];
    construct_configuration_for_file(configuration, "system\\Palettes.gdc");
    Dword result = 0;
    try {
        if (configuration_read_only(configuration))
            result = 14;
        else {
            select_section(configuration, string_ascii(&palette->name));
            Palette fallback;
            construct_palette(&fallback);
            try {
                const auto write = [&](const char *key, unsigned slot) {
                    write_colour(configuration, key, palette->colours + slot,
                                 fallback.colours + slot);
                };
                write("CLRS_NORMAL", 0);
                write("CLRS_FOCUS", 1);
                write("CLRS_DISABLED", 2);
                write("CLRS_SELECTED", 3);
                write("CLRS_EDGE", 5);
                write("CLRS_BACKGROUND", 4);
                write("CLRS_CARET", 9);
                write("CLRS_EDGEHILITE", 6);
                write("CLRS_EDGESHADOW", 7);
                write("CLRS_DROPSHADOW", 8);
                write("CLRS_DIVIDOR", 10);
                write("CLRS_HILITE", 11);
            } catch (...) {
                destroy_palette(&fallback);
                throw;
            }
            destroy_palette(&fallback);
        }
    } catch (...) {
        destroy_configuration(configuration);
        throw;
    }
    destroy_configuration(configuration);
    return result;
}
} // namespace nfs::gui::engine::pending
