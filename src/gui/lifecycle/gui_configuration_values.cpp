#include "gui_configuration_values.hpp"
#include "nfs/gui/resources/gui_format.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cwchar>
namespace nfs::gui::engine::pending {
using game::runtime::invoke;

int string_integer(const String *text) {
    const void *data = string_data(text);
    if (!data)
        return 0;
    return text->wide ? ::_wtoi(static_cast<const wchar_t *>(data))
                      : std::atoi(static_cast<const char *>(data));
}

Dword read_int(void *configuration, const char *key, int *out, int fallback) {
    const String *value = invoke<const String *>(configuration, 5, key);
    if (value->length) {
        *out = string_integer(value);
        return 1;
    }
    *out = fallback;
    return 0;
}

Dword read_integer(void *configuration, const char *key, int *out, int fallback) {
    const String *value = invoke<const String *>(configuration, 5, key);
    if (value->length) {
        *out = string_integer(value);
        return 1;
    }
    *out = fallback;
    return 0;
}

Dword read_reference_string(void *configuration, const char *key, String *out,
                           const String *fallback) {
    const String *value = invoke<const String *>(configuration, 5, key);
    if (!value->length) {
        string_copy(out, fallback);
        return 0;
    }


    assign_symbolic_string(out, string_ascii(value));
    return 1;
}

String *format_flags(String *out, const FlagDefinition *definitions, Dword flags) {
    String value;
    string_construct(&value);
    try {
        if (!flags)
            string_construct_ascii(out, "NULL");
        else {
            for (const FlagDefinition *entry = definitions; entry->name; ++entry)
                if ((entry->value & flags) == entry->value) {
                    if (value.length)
                        string_append_ascii(&value, "|");
                    string_append_ascii(&value, entry->name);
                }
            if (value.length)
                string_construct_copy(out, &value);
            else
                string_construct_ascii(out, "NULL");
        }
    } catch (...) {
        string_destruct(&value);
        throw;
    }
    string_destruct(&value);
    return out;
}

Dword write_ascii(void *configuration, const char *key, const char *value, const char *type,
                 Dword flags, const char *fallback) {
    const Dword remove =
        (!value && !fallback) || (value && fallback && !std::strcmp(value, fallback));
    String text;
    string_construct_ascii(&text, value);
    Dword result;
    try {
        result = invoke<Dword>(configuration, 7, key, &text, type, flags, remove);
    } catch (...) {
        string_destruct(&text);
        throw;
    }
    string_destruct(&text);
    return result;
}

Dword write_integer(void *configuration, const char *key, int value, int fallback) {
    char buffer[256];
    std::sprintf(buffer, "%d", value);
    String text;
    string_construct_ascii(&text, buffer);
    Dword result;
    try {
        result =
            invoke<Dword>(configuration, 7, key, &text, "LONG", Dword(0), Dword(value == fallback));
    } catch (...) {
        string_destruct(&text);
        throw;
    }
    string_destruct(&text);
    return result;
}

static const char *symbolic_string_name(const String *text) {
    if (text->reference) {
        if (!string_resolver)
            return nullptr;
        return invoke<const char *>(string_resolver, 5, text->reference);
    }
    return text->wide ? nullptr : static_cast<const char *>(text->data);
}

Dword write_reference_string(void *configuration, const char *key, const String *value,
                            const String *fallback) {
    const Dword remove = string_equal(value, fallback);
    String text;
    if (!value->reference && value->wide) {
        string_construct_copy(&text, value);
        if (!string_make_narrow(&text)) {
            string_destruct(&text);
            return 0;
        }
    } else
        string_construct_ascii(&text, symbolic_string_name(value));
    Dword result;
    try {
        result = invoke<Dword>(configuration, 7, key, &text, "TEXT", Dword(0), remove);
    } catch (...) {
        string_destruct(&text);
        throw;
    }
    string_destruct(&text);
    return result;
}

Dword write_colour(void *configuration, const char *key, const Dword *value, const Dword *fallback) {
    char buffer[256]{};
    std::sprintf(buffer, "[%d, %d, %d, %d]", *value >> 24, (*value >> 16) & 255,
                 (*value >> 8) & 255, *value & 255);
    String text;
    string_construct_ascii(&text, buffer);
    Dword result;
    try {
        result =
            invoke<Dword>(configuration, 7, key, &text, "COLOR", Dword(0), Dword(*value == *fallback));
    } catch (...) {
        string_destruct(&text);
        throw;
    }
    string_destruct(&text);
    return result;
}

static Dword equal_rectangle(const Rectangle *first, const Rectangle *second) {
    return first->left == second->left && first->top == second->top &&
           first->height == second->height && first->width == second->width;
}

Dword write_rectangle(void *configuration, const char *key, const Rectangle *value,
                     const Rectangle *fallback) {
    const Dword remove = equal_rectangle(value, fallback);
    char buffer[256]{};
    std::sprintf(buffer, "[%d, %d] %d, %d", value->left, value->top, value->width, value->height);
    String text;
    string_construct_ascii(&text, buffer);
    Dword result;
    try {
        result = invoke<Dword>(configuration, 7, key, &text, "RECT", Dword(0), remove);
    } catch (...) {
        string_destruct(&text);
        throw;
    }
    string_destruct(&text);
    return result;
}

Dword write_flags(void *configuration, const char *key, Dword value,
                 const FlagDefinition *definitions, Dword fallback) {
    String text;
    format_flags(&text, definitions, value);
    Dword result;
    try {
        result = invoke<Dword>(configuration, 7, key, &text, "FLAGS", definitions,
                              Dword(value == fallback));
    } catch (...) {
        string_destruct(&text);
        throw;
    }
    string_destruct(&text);
    return result;
}

Dword assign_symbolic_string(String *text, const char *name) {
    string_clear(text);
    Dword reference = 0;
    if (name && string_resolver)
        reference = invoke<Dword>(string_resolver, 3, name);
    if (reference && resolve_string(reference)) {
        if (text->data) {
            release(text->data);
            text->data = nullptr;
        }
        text->length = 0;
        text->wide = 0;
        text->reference = reference;
        text->length = resolve_length(reference);
        text->wide = resolve_wide(reference);
        return 1;
    }
    string_assign(text, name);
    return 0;
}
} // namespace nfs::gui::engine::pending
