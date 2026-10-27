#include "gui_configuration_cache.hpp"
#include "nfs/gui/resources/gui_file.hpp"
#include "nfs/gui/resources/gui_format.hpp"
#include "nfs/resources/files/filesystem.hpp"
#include "nfs/resources/archives/big_directory.hpp"
#include "nfs/core/services/service_lifecycle.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include "nfs/core/memory/memory_primitives.hpp"
#include "../../frontend/flow/frontend_runtime.hpp"
#include <cstring>
namespace nfs::gui {

Dword seek_file(File *file, int offset, int origin) {
    if (file->file) {
        if (!origin) {
            file->offset = offset;
            return 1;
        }
        if (origin == 1) {
            file->offset = int(files::size_sync(file->file, file->priority) + Dword(offset));
            return 1;
        }
        if (origin == 2) {
            file->offset = int(Dword(file->offset) + Dword(offset));
            return 1;
        }
    }
    return 0;
}

int read_file(File *file, void *destination, int bytes) {
    if (!file->file)
        return 0;
    const int previous = file->offset;
    const int transferred =
        int(files::read_sync(file->file, previous, destination, bytes, file->priority));
    file->offset = int(Dword(previous) + Dword(transferred));
    return transferred;
}
} // namespace nfs::gui
namespace nfs::gui::engine::pending {
namespace {
char configuration_directory[264]{},
    export_directory[260]{};
int configuration_file_priority = 100;
extern const Destroy configuration_document_methods[];
const String *text_or_empty(const PoolString *text) {
    return text ? &text->string : &lifecycle::construction::unresolved::shared_string;
}
} // namespace
// The saved setter copies through the terminator without a bound.
void set_configuration_directory(const char *directory) {
    if (directory)
        std::strcpy(configuration_directory, directory);
    else
        configuration_directory[0] = 0;
}

void set_export_directory(const char *directory) {
    if (directory)
        std::strcpy(export_directory, directory);
    else
        export_directory[0] = 0;
}
// Archive offsets and filesystem reads use the shared GUI owners.
Dword load_text_resource(const String *name, String *output) {
    const auto *archive = static_cast<const Byte *>(services::pending::gui_archive);
    if (archive) {
        Dword offset = 0, bytes = 0;
        if (big_find_entry(archive, string_ascii(name), 0, &offset, &bytes) && bytes)
            if (auto *memory = static_cast<char *>(
                    kernel::memory_allocate(nullptr, int(bytes + 1), gui::pool | 256))) {
                kernel::memory_copy(memory, archive + offset, int(bytes));
                memory[bytes] = 0;
                string_assign(output, memory);
                kernel::memory_free(memory);
                return 1;
            }
    }
    application::pending::LocalFrontendString path(configuration_directory);
    string_append(&path.value, name);
    File input;
    file_construct(&input, string_ascii(&path.value), 0, configuration_file_priority);
    Dword result = 0;
    try {
        const int bytes = input.file ? int(files::size_sync(input.file, input.priority)) : 0;
        if (input.file && bytes > 0)
            if (auto *memory = static_cast<char *>(
                    kernel::memory_allocate(nullptr, int(Dword(bytes) + 1), gui::pool | 256))) {
                if (read_file(&input, memory, bytes)) {
                    memory[bytes] = 0;
                    string_assign(output, memory);
                    result = 1;
                }
                kernel::memory_free(memory);
            }
    } catch (...) {
        file_destruct(&input);
        throw;
    }
    file_destruct(&input);
    return result;
}

static void destroy_configuration_document(ConfigurationDocument *cache) {
    cache->document.methods = configuration_document_methods;
    ini::clear(&cache->document);
}

static ConfigurationDocument *__fastcall delete_configuration_document(ConfigurationDocument *cache,
                                                                       void *, unsigned flags) {
    destroy_configuration_document(cache);
    if (flags & 1)
        gui::release(cache);
    return cache;
}
namespace {
const Destroy configuration_document_methods[] = {
    reinterpret_cast<Destroy>(delete_configuration_document)};
}

ConfigurationDocument *construct_configuration_document(ConfigurationDocument *cache,
                                                        const String *name) {
    ini::construct(&cache->document, configuration_directory, string_ascii(name));
    try {
        cache->name_hash = configuration_name_hash(name);
        cache->document.methods = configuration_document_methods;
        cache->references = 0;
        String path;
        concatenate(&path, text_or_empty(cache->document.directory),
                    text_or_empty(cache->document.name));
        try {
            if (path.length) {
                const auto *archive = static_cast<const Byte *>(services::pending::gui_archive);
                bool found_in_archive = false;
                const String *filename = text_or_empty(cache->document.name);
                if (archive && filename->length) {
                    const char *wanted = string_ascii(filename);
                    const int count = int(big_entry_count(archive));
                    for (int index = 0; index < count; ++index) {
                        Dword offset = 0, bytes = 0;
                        const char *candidate =
                            big_find_entry(archive, nullptr, index, &offset, &bytes);
                        if (candidate && !_stricmp(candidate, wanted)) {
                            if (bytes) {
                                ini::parse(&cache->document,
                                           reinterpret_cast<const char *>(archive + offset),
                                           int(bytes), 1, 1);
                                found_in_archive = true;
                            }
                            break;
                        }
                    }
                }
                if (!found_in_archive) {
                    File input;
                    file_construct(&input, string_ascii(&path), 0, configuration_file_priority);
                    try {
                        const int bytes =
                            input.file ? int(files::size_sync(input.file, input.priority)) : 0;
                        if (input.file && bytes && seek_file(&input, 0, 0)) {
                            void *memory =
                                kernel::memory_allocate("GFile::Load", bytes, gui::pool | 256);
                            if (memory) {
                                try {
                                    if (read_file(&input, memory, bytes))
                                        ini::parse(&cache->document,
                                                   static_cast<const char *>(memory),
                                                   kernel::memory_size(memory), 1, 1);
                                } catch (...) {
                                    kernel::memory_free(memory);
                                    throw;
                                }
                                kernel::memory_free(memory);
                            }
                        }
                    } catch (...) {
                        file_destruct(&input);
                        throw;
                    }
                    file_destruct(&input);
                }
            }
        } catch (...) {
            string_destruct(&path);
            throw;
        }
        string_destruct(&path);
    } catch (...) {
        ini::clear(&cache->document);
        throw;
    }
    return cache;
}
} // namespace nfs::gui::engine::pending
