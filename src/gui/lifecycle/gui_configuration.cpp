#include "nfs/gui/engine/gui_engine.hpp"
#include "nfs/resources/ini/ini.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include <cstdio>
namespace nfs::gui::engine::pending {
// GUI's reference-counted INI cache is distinct from ini::CachedDocument.
// Both providers share configuration_documents, owned by GUI cache lifecycle.
ini::Document *acquire_configuration_document(const String *);
void release_configuration_document(const String *);
struct Configuration {
    const Method *methods;
    String section, filename;
    Dword dirty;
    ini::Document *document;
    ini::Section *selected;
};
static_assert(sizeof(Configuration) == 56 && offsetof(Configuration, filename) == 24);
extern const Method configuration_methods[9];

void *create_configuration(void *storage, const String *filename) {
    auto *configuration = static_cast<Configuration *>(storage);
    string_construct(&configuration->section);
    string_construct(&configuration->filename);
    configuration->methods = configuration_methods;
    try {
        string_copy(&configuration->filename, filename);
        configuration->dirty = 0;
        configuration->document = acquire_configuration_document(&configuration->filename);
        configuration->selected = nullptr;
    } catch (...) {
        string_destruct(&configuration->filename);
        string_destruct(&configuration->section);
        configuration->methods = reinterpret_cast<const Method *>(gui::base_methods);
        throw;
    }
    return configuration;
}

void destroy_configuration(void *storage) {
    auto *configuration = static_cast<Configuration *>(storage);
    configuration->methods = configuration_methods;
    if (configuration->dirty && configuration->document && ini::save(configuration->document)) {
        configuration->document->auto_save = 0;
        configuration->dirty = 0;
    }
    release_configuration_document(&configuration->filename);
    configuration->document = nullptr;
    string_destruct(&configuration->filename);
    string_destruct(&configuration->section);
    configuration->methods = reinterpret_cast<const Method *>(gui::base_methods);
}

void select_section(void *storage, const char *name) {
    auto *configuration = static_cast<Configuration *>(storage);
    string_assign(&configuration->section, name);
    configuration->selected =
        configuration->section.length
            ? ini::find_section(configuration->document, string_ascii(&configuration->section))
            : nullptr;
}

Dword read_rectangle(void *storage, const char *key, Rectangle *rectangle,
                    const Rectangle *fallback) {
    const auto *value = game::runtime::invoke<const String *>(storage, 5, key);
    if (value->length)
        return Dword(std::sscanf(string_ascii(value), "[ %d , %d ] %d , %d", &rectangle->left,
                                &rectangle->top, &rectangle->width, &rectangle->height));
    rectangle->left = fallback->left;
    rectangle->top = fallback->top;
    const int height = fallback->height, width = fallback->width;
    rectangle->width = width;
    rectangle->height = height;
    return 0;
}

static Configuration *__fastcall delete_configuration(Configuration *configuration, void *,
                                                      unsigned flags) {
    destroy_configuration(configuration);
    if (flags & 1)
        gui::release(configuration);
    return configuration;
}

static Dword __fastcall section_count(Configuration *configuration, void *) {
    return configuration->document ? configuration->document->sections.count : 0;
}

static const String *__fastcall section_name(Configuration *configuration, void *, Dword index) {
    if (configuration->document && index < configuration->document->sections.count)
        if (auto *section = static_cast<ini::Section *>(
                events::container_value(&configuration->document->sections, index)))
            if (section->name)
                return &section->name->string;
    return &lifecycle::construction::unresolved::shared_string;
}

static Dword __fastcall key_count(Configuration *configuration, void *) {
    return configuration->document && configuration->selected ? configuration->selected->keys.count
                                                              : 0;
}

static const String *__fastcall key_name(Configuration *configuration, void *, Dword index) {
    if (configuration->document && configuration->selected &&
        index < configuration->selected->keys.count)
        if (auto *key = static_cast<ini::Key *>(
                events::container_value(&configuration->selected->keys, index)))
            if (key->name)
                return &key->name->string;
    return &lifecycle::construction::unresolved::shared_string;
}

static const String *__fastcall key_value(Configuration *configuration, void *, const char *name) {
    if (configuration->document && configuration->selected)
        if (auto *key = ini::find(configuration->selected, name))
            return &key->value;
    return &lifecycle::construction::unresolved::shared_string;
}

static const String *__fastcall indexed_value(Configuration *configuration, void *, Dword index) {
    if (configuration->document && configuration->selected &&
        index < configuration->selected->keys.count)
        if (auto *key = static_cast<ini::Key *>(
                events::container_value(&configuration->selected->keys, index)))
            return &key->value;
    return &lifecycle::construction::unresolved::shared_string;
}
// Third/fourth stack parameters are unused in this implementation.
static Dword __fastcall update_key(Configuration *configuration, void *, const char *name,
                                  const String *value, Dword, Dword, Dword remove) {
    ini::Document *document = configuration->document;
    if (document && !ini::read_only(document)) {
        if (!configuration->selected)
            configuration->selected =
                ini::add_section(document, string_ascii(&configuration->section), 1);
        if (configuration->selected) {
            if (remove) {
                ini::remove(configuration->selected, name);
                return 1;
            }
            if (ini::set(configuration->selected, name, string_ascii(value), 1)) {
                configuration->dirty = 1;
                document->auto_save = 1;
                return 1;
            }
        }
    }
    return 0;
}

static Dword __fastcall copy_key_value(Configuration *configuration, void *, const char *name,
                                      const String *fallback, String *out) {
    if (configuration->document && configuration->selected)
        if (auto *key = ini::find(configuration->selected, name)) {
            string_copy(out, &key->value);
            if (out->length)
                return Dword(out->length);
        }
    string_copy(out, fallback);
    return 0;
}
const Method configuration_methods[9] = {reinterpret_cast<Method>(delete_configuration),
                                         reinterpret_cast<Method>(section_count),
                                         reinterpret_cast<Method>(section_name),
                                         reinterpret_cast<Method>(key_count),
                                         reinterpret_cast<Method>(key_name),
                                         reinterpret_cast<Method>(key_value),
                                         reinterpret_cast<Method>(indexed_value),
                                         reinterpret_cast<Method>(update_key),
                                         reinterpret_cast<Method>(copy_key_value)};
} // namespace nfs::gui::engine::pending
