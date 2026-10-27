#include "../lifecycle/gui_font_styles.hpp"
#include "../lifecycle/gui_configuration_values.hpp"
#include "nfs/gui/resources/gui_format.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include "../../math/precision/extended_float.hpp"
#include "../lifecycle/gui_font_metrics.hpp"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
namespace nfs::gui::engine::pending {
using game::runtime::invoke;
namespace {
Dword font_style_serial = 1;
} // namespace
const FlagDefinition font_flags[] = {
    {"FEF_ITALIC", 0, 1},    {"FEF_BOLD", 0, 2},        {"FEF_STRIKEOUT", 0, 4},
    {"FEF_UNDERLINE", 0, 8}, {"FEF_DROPSHADOW", 0, 16}, {nullptr, 0, 0}};
namespace {
extern const Destroy font_style_methods[];
const String *empty_string() {
    return &lifecycle::construction::unresolved::shared_string;
}
} // namespace

Dword character_at(const String *text, int index) {
    if (index >= 0 && index < text->length)
        if (const void *data = string_data(text))
            return text->wide ? Dword(static_cast<const std::uint16_t *>(data)[index])
                              : Dword(int(static_cast<const signed char *>(data)[index]));
    return 0;
}
// Narrow return retains atof precision; wide conversion stores float.
long double string_number(const String *text) {
    if (!text->wide) {
        const char *value = static_cast<const char *>(string_data(text));
        return value ? std::atof(value) : 0;
    }
    String narrow;
    string_construct_copy(&narrow, text);
    float result = 0;
    try {
        if (string_make_narrow(&narrow))
            if (const char *value = string_ascii(&narrow))
                result = float(std::atof(value));
    } catch (...) {
        string_destruct(&narrow);
        throw;
    }
    string_destruct(&narrow);
    return result;
}
// Spaces are stripped within each token; names are case sensitive.
Dword parse_flags(const FlagDefinition *definitions, const String *text) {
    if (!text->length)
        return 0;
    if (string_data(text) && !string_compare_inline_ascii(text, "NULL"))
        return 0;
    char buffer[1024], token[256];
    std::strcpy(buffer, string_ascii(text));
    Dword result = 0;
    for (char *part = std::strtok(buffer, "|"); part; part = std::strtok(nullptr, "|")) {
        char *out = token;
        int remaining = 255;
        while (*part && remaining) {
            if (*part != ' ')
                *out++ = *part;
            ++part;
            --remaining;
        }
        *out = 0;
        if (*token)
            for (const FlagDefinition *definition = definitions; definition->name; ++definition)
                if (!std::strcmp(token, definition->name)) {
                    result |= definition->value;
                    break;
                }
    }
    return result;
}

Dword read_float(void *configuration, const char *key, float *out, float fallback) {
    const String *value = invoke<const String *>(configuration, 5, key);
    if (value->length) {
        *out = float(string_number(value));
        return 1;
    }
    *out = fallback;
    return 0;
}

Dword read_string(void *configuration, const char *key, String *out, const String *fallback) {
    if (invoke<Dword>(configuration, 8, key, empty_string(), out))
        return Dword(out->length) + 1;
    string_copy(out, fallback);
    return 0;
}

Dword read_flags(void *configuration, const char *key, Dword *out, const FlagDefinition *definitions,
                Dword fallback) {
    const String *value = invoke<const String *>(configuration, 5, key);
    if (value->length) {
        *out = parse_flags(definitions, value);
        return 1;
    }
    *out = fallback;
    return 0;
}

Dword read_colour(void *configuration, const char *key, Dword *out, const Dword *fallback) {
    const String *value = invoke<const String *>(configuration, 5, key);
    if (value->length) {
        int alpha = int(*out >> 24), red = int((*out >> 16) & 255), green = int((*out >> 8) & 255),
            blue = int(*out & 255);
        const int fields =
            std::sscanf(string_ascii(value), "[ %d , %d , %d , %d ]", &alpha, &red, &green, &blue);
        *out = (Dword(alpha) & 255) << 24 | (Dword(red) & 255) << 16 | (Dword(green) & 255) << 8 |
               (Dword(blue) & 255);
        return Dword(fields);
    }
    *out = *fallback;
    return 0;
}

static FontStyle *construct_font_style(FontStyle *style) {
    style->scale_x = style->scale_y = 1;
    style->flags = 0;
    style->height = 0;
    style->spacing = 0.7799999713897705078125f;
    style->font = 0;
    style->handle = 0;
    return style;
}

static void destroy_font_style_record(FontStyleRecord *record) {
    record->methods = font_style_methods;
    string_destruct(&record->name);
    record->methods = gui::base_methods;
}

static FontStyleRecord *__fastcall delete_font_style_record(FontStyleRecord *record, void *,
                                                            unsigned flags) {
    destroy_font_style_record(record);
    if (flags & 1)
        gui::release(record);
    return record;
}
namespace {
const Destroy font_style_methods[] = {reinterpret_cast<Destroy>(delete_font_style_record)};
} // namespace
void construct_configuration_for_file(void *configuration, const char *filename) {
    String file;
    string_construct_ascii(&file, filename);
    try {
        create_configuration(configuration, &file);
    } catch (...) {
        string_destruct(&file);
        throw;
    }
    string_destruct(&file);
}
FontStyleRecord *create_font_style_record(const String *name) {
    auto *record = static_cast<FontStyleRecord *>(allocate(sizeof(FontStyleRecord)));
    if (!record)
        return nullptr;
    string_construct(&record->name);
    construct_font_style(&record->style);
    record->methods = font_style_methods;
    try {
        string_copy(&record->name, name);
    } catch (...) {
        string_destruct(&record->name);
        gui::release(record);
        throw;
    }
    record->handle = font_style_serial;
    font_style_serial += 4;
    return record;
}
// Invalid indices retain the saved low-address access.
static long double face_size_ratio(int index) {
    return get_font_face(index)->size_ratio;
}

void load_font_styles(Engine *engine) {
    if (!engine->context)
        return;
    engine->font = &engine->default_font;
    alignas(4) Byte configuration[56];
    construct_configuration_for_file(configuration, "system\\fonts.gdc");
    try {
        for (int index = 0; index < invoke<int>(configuration, 1); ++index) {
            const String *name = invoke<const String *>(configuration, 2, Dword(index));
            if (!character_at(name, 0))
                continue;
            FontStyle style{1, 1, 0, 0, 0.7799999713897705078125f, 0, 0};
            select_section(configuration, string_ascii(name));
            FontStyleRecord *record = create_font_style_record(name);
            if (!record)
                continue;
            String face;
            string_construct(&face);
            try {
                read_string(configuration, "face", &face, empty_string());
                read_float(configuration, "scaleX", &style.scale_x, 0);
                read_float(configuration, "scaleY", &style.scale_y, 0);
                read_flags(configuration, "style", &style.flags, font_flags, 0);
                style.font = find_font(&face);
                fonts::Font *font = get_font(style.font);
                const float ratio = float(face_size_ratio(style.font));
                if (font && ratio != 0.0f)
                    normalize_font_scale(&style, ratio);
                initialize_font_metrics(engine, &style);
                style.handle = record->handle;
                record->style = style;
                container_append(&engine->font_styles, record, dispose_object);
            } catch (...) {
                string_destruct(&face);
                throw;
            }
            string_destruct(&face);
        }
    } catch (...) {
        destroy_configuration(configuration);
        throw;
    }
    destroy_configuration(configuration);
}

void load_palettes(Engine *engine) {
    if (!engine->context)
        return;
    alignas(4) Byte configuration[56];
    construct_configuration_for_file(configuration, "system\\Palettes.gdc");
    try {
        for (int index = 0; index < invoke<int>(configuration, 1); ++index) {
            String name;
            string_construct_copy(&name, invoke<const String *>(configuration, 2, Dword(index)));
            try {
                if (name.length) {
                    select_section(configuration, string_ascii(&name));
                    auto *palette = static_cast<Palette *>(allocate(sizeof(Palette)));
                    if (palette)
                        construct_palette(palette);
                    string_copy(&palette->name, &name);
                    const auto read = [&](const char *key, unsigned slot) {
                        read_colour(configuration, key, palette->colours + slot,
                                    palette->colours + slot);
                    };
                    read("CLRS_NORMAL", 0);
                    read("CLRS_FOCUS", 1);
                    read("CLRS_DISABLED", 2);
                    read("CLRS_SELECTED", 3);
                    read("CLRS_EDGE", 5);
                    read("CLRS_BACKGROUND", 4);
                    read("CLRS_CARET", 9);
                    read("CLRS_EDGEHILITE", 6);
                    read("CLRS_EDGESHADOW", 7);
                    read("CLRS_DROPSHADOW", 8);
                    read("CLRS_DIVIDOR", 10);
                    read("CLRS_HILITE", 11);
                    container_append(&engine->palettes, palette, dispose_object);
                }
            } catch (...) {
                string_destruct(&name);
                throw;
            }
            string_destruct(&name);
        }
    } catch (...) {
        destroy_configuration(configuration);
        throw;
    }
    destroy_configuration(configuration);
}
} // namespace nfs::gui::engine::pending
