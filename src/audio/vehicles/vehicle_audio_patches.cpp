#include "nfs/audio/vehicles/vehicle_audio.hpp"
#include "nfs/resources/archives/archive_cache.hpp"
#include "nfs/core/console/console.hpp"
#include <cstring>
#include <cstdio>
namespace nfs::game::audio::vehicle {

void load_patch(const char *archive, const char *prefix, const char *suffix, int patch, int exhaust,
                int listener) {
    kernel::cooperate(0);
    char filename[260];
    std::strcpy(filename, prefix);
    std::strcat(filename, suffix);
    std::strcat(filename, ".bnk");
    kernel::print::check_console_write("Loading engine patch: %s\n", filename);
    void *data = files::archives::load(archive, filename, lifecycle::audio_pool | 1024, 0);
    if (!data) {
        kernel::print::check_console_write(
            "ENGINE PATCH FILE DOES NOT EXIST THIS IS BADDD!!!! %s\n", filename);
        return;
    }
    int handle;
    if (sound::initialize_bank(&handle, data) == 7) {
        int size = sound::bank_bytes(handle);
        sound::release_bank(handle);
        kernel::memory_free(data);
        data = kernel::memory_allocate("bhdr", size, lifecycle::audio_pool | 1024);
        void *loaded = files::archives::load(archive, filename, lifecycle::audio_pool | 1024, 0);
        sound::initialize_bank(&handle, loaded);
        sound::relocate_bank(data, handle);
        kernel::memory_free(loaded);
    }
    auto &entry = bank(listener, patch);
    entry.data = data;
    entry.handle = handle;
    entry.exhaust = exhaust;
}

void load_patch_group(const char *archive, const char *prefix, int group, int listener) {
    switch (group) {
    case 100:
        load_patch(archive, prefix, "lelo", 0, 0, listener);
        load_patch(archive, prefix, "lemd", 1, 0, listener);
        load_patch(archive, prefix, "lehi", 2, 0, listener);
        return;
    case 101:
        load_patch(archive, prefix, "ceid", 6, 0, listener);
        load_patch(archive, prefix, "cemd", 7, 0, listener);
        load_patch(archive, prefix, "cehi", 8, 0, listener);
        return;
    case 102:
        load_patch(archive, prefix, "lxlo", 3, 1, listener);
        load_patch(archive, prefix, "lxmd", 4, 1, listener);
        load_patch(archive, prefix, "lxhi", 5, 1, listener);
        return;
    case 103:
        load_patch(archive, prefix, "cxmd", 10, 1, listener);
        load_patch(archive, prefix, "cxhi", 11, 1, listener);
        return;
    case 108:
        load_patch(archive, prefix, "cxid", 9, 1, listener);
        return;
    case 109:
        load_patch(archive, prefix, "2cxid", 9, 1, listener);
        return;
    }
}
} // namespace nfs::game::audio::vehicle
