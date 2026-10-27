#include "nfs/resources/ini/ini.hpp"
#include "nfs/gui/resources/gui_file.hpp"
#include "nfs/resources/files/file_cache.hpp"
#include "nfs/gui/resources/gui_format.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include <exception>
#include <io.h>
namespace nfs::ini {
using namespace gui;
static const String &empty = lifecycle::construction::unresolved::shared_string;
// Retain the selected name owners across both calls.
static String *path_string(String *result, PoolString *directory, PoolString *name) {
    return concatenate(result, directory ? &directory->string : &empty,
                       name ? &name->string : &empty);
}
// Normal exits release the string's storage directly; exception cleanup uses its destructor.
static void release_path_storage(String *path) {
    void *storage = path->data;
    path->methods = string_methods;
    if (storage)
        kernel::memory_free(storage);
}

Dword read_only(Document *doc) {
    PoolString *name = doc->name;
    PoolString *directory = doc->directory;
    String path;
    path_string(&path, directory, name);
    try {
        if (!string_data(&path)) {
            release_path_storage(&path);
            return 1;
        }
        if (files::file::exists(string_ascii(&path)) && _access(string_ascii(&path), 2)) {
            release_path_storage(&path);
            return 1;
        }
        release_path_storage(&path);
        return 0;
    } catch (...) {
        try {
            string_destruct(&path);
        } catch (...) {
            std::terminate();
        }
        throw;
    }
}

Dword save(Document *doc) {
    PoolString *name = doc->name;
    PoolString *directory = doc->directory;
    String path, second;
    File output;
    path_string(&path, directory, name);
    // Cleanup states are path 0, second 1 and File 2; second and File both unwind to state 0.
    int cleanup_state = 0;
    try {
        if (!string_data(&path)) {
            release_path_storage(&path);
            return 0;
        }
        path_string(&second, directory, name);
        cleanup_state = 1;
        if (!string_data(&second) ||
            (files::file::exists(string_ascii(&second)) && _access(string_ascii(&second), 2))) {
            string_destruct(&second);
            release_path_storage(&path);
            return 0;
        }
        cleanup_state = 0;
        string_destruct(&second);
        file_construct(&output, string_ascii(&path), 6, 100);
        cleanup_state = 2;
        for (int i = 0; i < static_cast<int>(doc->sections.count); ++i) {
            Element *element = doc->sections.elements[i];
            auto *section = element ? static_cast<Section *>(element->value) : nullptr;
            if (!section)
                continue;
            PoolString *section_name = section->name;
            const String *section_text = section_name ? &section_name->string : &empty;
            file_write(&output, "[");
            file_write(&output, section_text);
            file_write(&output, "]\r\n");
            const int count = section->keys.count;
            for (int j = 0; j < count; ++j) {
                if (static_cast<Dword>(j) >= section->keys.count)
                    continue;
                element = section->keys.elements[j];
                auto *key = element ? static_cast<Key *>(element->value) : nullptr;
                if (!key)
                    continue;
                file_write(&output, key->name ? &key->name->string : &empty);
                file_write(&output, "=");
                file_write(&output, &key->value);
                file_write(&output, "\r\n");
            }
            file_write(&output, "\r\n\r\n");
        }
        file_close(&output);
        cleanup_state = 0;
        file_destruct(&output);
        release_path_storage(&path);
        return 1;
    } catch (...) {
        // A second C++ exception while running the saved unwind actions terminates.
        try {
            if (cleanup_state == 1)
                string_destruct(&second);
            else if (cleanup_state == 2)
                file_destruct(&output);
            string_destruct(&path);
        } catch (...) {
            std::terminate();
        }
        throw;
    }
}
} // namespace nfs::ini
