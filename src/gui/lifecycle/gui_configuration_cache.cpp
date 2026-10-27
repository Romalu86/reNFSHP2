#include "nfs/gui/engine/gui_engine.hpp"
#include "nfs/resources/ini/ini.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include "gui_configuration_cache.hpp"
namespace nfs::gui::engine::pending {
namespace {
const Dword name_crc[256] = {
#include "gui_name_crc.inc"
};
} // namespace
// Empty input maps to0; nonempty input retains the uncomplemented CRC.
Dword *hash_name(Dword *out, const char *name) {
    if (!name || !*name) {
        *out = 0;
        return out;
    }
    Dword hash = Dword(-1);
    for (const auto *cursor = reinterpret_cast<const Byte *>(name); *cursor; ++cursor)
        hash = (hash >> 8) ^ name_crc[(*cursor ^ hash) & 255];
    *out = hash;
    return out;
}

String *uppercase_copy(const String *source, String *out) {
    String temporary;
    string_construct(&temporary);
    try {
        string_copy(&temporary, source);
        uppercase(&temporary);
        string_construct_copy(out, &temporary);
    } catch (...) {
        string_destruct(&temporary);
        throw;
    }
    string_destruct(&temporary);
    return out;
}

Dword configuration_name_hash(const String *name) {
    String upper;
    uppercase_copy(name, &upper);
    Dword hash;
    try {
        hash_name(&hash, string_ascii(&upper));
    } catch (...) {
        string_destruct(&upper);
        throw;
    }
    string_destruct(&upper);
    return hash;
}

ini::Document *acquire_configuration_document(const String *name) {
    if (!name->length)
        return nullptr;
    const Dword hash = configuration_name_hash(name);
    const Dword count = configuration_documents->count;
    Element **entries = configuration_documents->elements;
    for (Dword index = 0; index < count; ++index)
        if (Element *entry = entries[index])
            if (auto *document = static_cast<ConfigurationDocument *>(entry->value))
                if (document->name_hash == hash) {
                    ++document->references;
                    return &document->document;
                }
    auto *document = static_cast<ConfigurationDocument *>(
        allocate_flags(sizeof(ConfigurationDocument), 0, "GINIDataCache"));
    if (document) {
        try {
            construct_configuration_document(document, name);
        } catch (...) {
            gui::release(document);
            throw;
        }
    }
    container_append(configuration_documents, document, dispose_object);
    ++document->references;
    return &document->document;
}

void release_configuration_document(const String *name) {
    if (!string_data(name))
        return;
    const Dword hash = configuration_name_hash(name);
    const Dword count = configuration_documents->count;
    Element **entries = configuration_documents->elements;
    for (Dword index = 0; index < count; ++index)
        if (Element *entry = entries[index])
            if (auto *document = static_cast<ConfigurationDocument *>(entry->value))
                if (document->name_hash == hash) {
                    if (!--document->references)
                        container_remove(configuration_documents, document);
                    return;
                }
}
} // namespace nfs::gui::engine::pending
