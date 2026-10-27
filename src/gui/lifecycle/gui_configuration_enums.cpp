#include "gui_configuration_values.hpp"
#include "gui_images.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
namespace nfs::gui::engine::pending {
using game::runtime::invoke;

String *format_enum(String *out, const FlagDefinition *definitions, Dword value) {
    for (const auto *definition = definitions; definition->name; ++definition)
        if (definition->value == value)
            return string_construct_ascii(out, definition->name);
    return string_construct_ascii(out, "NULL");
}
// The saved comparison reads the input, not the trimmed copy.
Dword parse_enum(const FlagDefinition *definitions, const String *text) {
    if (!text->length)
        return 0;
    String trimmed;
    string_construct_copy(&trimmed, text);
    Dword result = 0;
    try {
        images::trim_left(&trimmed, 32);
        images::trim_right(&trimmed, 32);
        for (const auto *definition = definitions; definition->name; ++definition)
            if (!string_compare_ascii(text, definition->name)) {
                result = definition->value;
                break;
            }
    } catch (...) {
        string_destruct(&trimmed);
        throw;
    }
    string_destruct(&trimmed);
    return result;
}

Dword read_enum(void *configuration, const char *key, Dword *out, const FlagDefinition *definitions,
               Dword fallback) {
    const String *value = invoke<const String *>(configuration, 5, key);
    if (value->length) {
        *out = parse_enum(definitions, value);
        return 1;
    }
    *out = fallback;
    return 0;
}

Dword write_enum(void *configuration, const char *key, Dword value, const FlagDefinition *definitions,
                Dword fallback) {
    String text;
    format_enum(&text, definitions, value);
    Dword result;
    try {
        result = invoke<Dword>(configuration, 7, key, &text, "ENUM", definitions,
                              Dword(value == fallback));
    } catch (...) {
        string_destruct(&text);
        throw;
    }
    string_destruct(&text);
    return result;
}
} // namespace nfs::gui::engine::pending
