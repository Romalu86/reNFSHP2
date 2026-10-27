#include "nfs/audio/music/music_clients.hpp"
#include "nfs/resources/files/file_cache.hpp"
#include "nfs/resources/archives/archive_cache.hpp"
#include "nfs/core/console/console.hpp"
#include "nfs/audio/backend/sound_ownership.hpp"
#include <cstring>
#include <cstdio>
namespace audio_layout = nfs::game::audio::state::layout;
namespace nfs::game::audio {
using namespace state;

void load_bank(const char *name, int index, Dword pool) {
    if (!configuration<Dword>(audio_layout::audio_enabled))
        return;
    char filename[260];
    std::strcpy(filename, name);
    std::strcat(filename, ".bnk");
    void *data = files::file::try_load(filename, pool);
    if (!data) {
        kernel::print::check_console_write("file %s Not found!!!!!\n", filename);
        banks()[index].data = nullptr;
        banks()[index].handle = -1;
        return;
    }
    int handle;
    if (sound::initialize_bank(&handle, data) == 7) {
        int bytes = sound::bank_bytes(handle);
        sound::release_bank(handle);
        kernel::memory_free(data);
        data = kernel::memory_allocate("bhdr", bytes, pool);
        void *loaded = files::file::try_load(filename, pool);
        sound::initialize_bank(&handle, loaded);
        sound::relocate_bank(data, handle);
        kernel::memory_free(loaded);
    }
    banks()[index].handle = handle;
    banks()[index].data = data;
}

int load_archive_bank(const char *archive, const char *name, int index, Dword pool) {
    if (!configuration<Dword>(audio_layout::audio_enabled))
        return 0;
    void *data = files::archives::load(archive, name, pool, 0);
    if (!data) {
        kernel::print::check_console_write("BANK FILE DOES NOT EXIST THIS IS BADDD %s!!!!\n", name);
        return 0;
    }
    int handle;
    if (sound::initialize_bank(&handle, data) == 7) {
        int bytes = sound::bank_bytes(handle);
        sound::release_bank(handle);
        kernel::memory_free(data);
        data = kernel::memory_allocate("bhdr", bytes, pool);
        void *loaded = files::archives::load(archive, name, pool, 0);
        sound::initialize_bank(&handle, loaded);
        sound::relocate_bank(data, handle);
        kernel::memory_free(loaded);
    }
    banks()[index].data = data;
    banks()[index].handle = handle;
    return 1;
}

void release_frontend_bank() {
    sound::release_bank(-1);
    if (banks()[48].data) {
        banks()[48].handle = -1;
        kernel::memory_free(banks()[48].data);
        banks()[48].data = nullptr;
    }
}

void load_frontend_bank() {
    char filename[260];
    std::strcpy(filename, path(1304));
    std::strcat(filename, "fesfx");
    load_bank(filename, 48, lifecycle::audio_pool | 1280);
}

void load_reverb_resources() {
    if (!configuration<Dword>(audio_layout::audio_enabled))
        return;
    for (unsigned index = 0; index < 7; ++index) {
        char filename[260];
        std::sprintf(filename, "%ssoftvrb%d.fx", path(1304),
                     configuration<int>(1740 + index * 4));
        sound::field<void *>(effects_storage, 4 + index * 4) =
            files::file::try_load(filename, lifecycle::audio_pool | 1024);
    }
    sound::backend::configure_effect_groups(16, 0, 0, Dword(-1), Dword(-1));
    if (configuration<Dword>(1688))
        sound::backend::configure_effect_groups(128, 0, 0, Dword(-1), Dword(-1));
    configuration<int>(audio_layout::reverb_preset) = -1;
}
} // namespace nfs::game::audio
