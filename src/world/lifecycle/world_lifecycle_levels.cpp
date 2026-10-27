#include "nfs/world/lifecycle/world_lifecycle.hpp"
#include "nfs/graphics/particles/particle_lifecycle.hpp"
#include "nfs/world/resources/runtime_resources.hpp"
#include "nfs/resources/files/file_cache.hpp"
#include "nfs/graphics/resources/eagl_graphics.hpp"
#include "nfs/graphics/configuration/display_configuration.hpp"
#include "nfs/graphics/backend/eagl.hpp"
#include "nfs/core/services/service_lifecycle.hpp"
#include "nfs/network/session/network_virtual.hpp"
#include "nfs/core/console/console.hpp"
#include <cstdio>
#include <cstring>
namespace nfs::game::world {


void load_level_resources(void *owner) {
    at<Dword>(owner, 4) = 0;
    at<Byte>(owner, 8) = 0;
    for (unsigned field = 12; field <= 28; field += 4)
        at<Dword>(owner, field) = 0;
    std::memset(offset(owner, 60), 0, 128);
    void *data = files::file::try_load(runtime::level_file("level.dat"), lifecycle::level_pool);
    at<void *>(owner, 12) = data;
    if (!data)
        return;
    at<Byte>(owner, 8) = 1;
    void *memory = lifecycle::allocate(72, "GOBDefs", lifecycle::level_pool);
    pending::gob_definitions =
        memory ? static_cast<Dword *>(construct_gob_definitions(memory)) : nullptr;
    Dword *header =
        relocate_level_gob_blocks(pending::gob_definitions, static_cast<Dword *>(data));
    const Dword base = reinterpret_cast<Dword>(header);
    at<Dword>(owner, 116) = header[14];
    std::memcpy(offset(owner, 60), header, 24);
    at<Dword>(owner, 124) = header[16];
    at<Dword>(owner, 132) = header[30];
    at<Dword>(owner, 108) = header[12];
    at<Dword>(owner, 148) = header[22];
    at<Dword>(owner, 164) = header[20];
    at<Dword>(owner, 140) = header[31];
    at<Dword>(owner, 180) = header[28];
    at<Dword>(owner, 172) = header[26];
    at<Dword>(owner, 156) = 0;
    int race_mode = -1, race_variant = -1;
    if (Byte(services::special_race_mode(subsystem(72)))) {
        int mode = at<int>(subsystem(72), 148);
        if (mode == 2)
            race_mode = 0;
        else if (mode == 3)
            race_mode = 1;
        else if (mode == 4)
            race_mode = 2;
        race_variant = at<int>(subsystem(72), 102920);
    }
    for (unsigned kind = 0; kind < 6; ++kind) {
        int source_count = at<int>(owner, 60 + kind * 4);
        auto *source = reinterpret_cast<Dword *>(base + header[6 + kind]);
        at<void *>(owner, 84 + kind * 4) = source;
        int accepted = 0;
        Dword *output = source;
        for (int index = 0; index < source_count; ++index, source += 9) {
            if ((int(source[7]) == -1 && int(source[8]) == -1) ||
                (int(source[7]) == race_mode && int(source[8]) == race_variant)) {
                if (accepted != index)
                    std::memcpy(output, source, 36);
                ++accepted;
                output += 9;
            }
        }
        at<int>(owner, 60 + kind * 4) = accepted;
    }
    at<Dword>(owner, 112) = base + header[13];
    at<Dword>(owner, 136) = base + header[33];
    at<Dword>(owner, 128) = base + header[17];
    at<Dword>(owner, 144) = base + header[32];
    at<Dword>(owner, 152) = base + header[23];
    at<Dword>(owner, 120) = base + header[15];
    at<Dword>(owner, 168) = base + header[21];
    at<Dword>(owner, 184) = base + header[29];
    at<Dword>(owner, 176) = base + header[27];
    Dword points = base + header[34];
    for (int index = 0; index < at<int>(owner, 156); ++index) {
        auto *entry = reinterpret_cast<Dword *>(at<Dword>(owner, 160)) + index * 19;
        entry[18] = points;
        points += entry[17] * 12;
    }
    for (unsigned kind = 0; kind < 6; ++kind) {
        auto *markers = at<float *>(owner, 84 + kind * 4);
        for (int index = 0; index < at<int>(owner, 60 + kind * 4); ++index)
            markers[index * 9 + 2] = -markers[index * 9 + 2];
    }
    for (int index = 0; index < at<int>(owner, 116); ++index) {
        void *entry = offset(at<void *>(owner, 120), index * 140);
        at<float>(entry, 20) *= -1.0f;
    }
    fix_level_points(owner);
    for (int index = 0; index < at<int>(owner, 180); ++index) {
        void *point = offset(at<void *>(owner, 184), index * 16);
        at<float>(point, 12) = -at<float>(point, 12);
    }
    if (at<Byte>(owner, 8)) {
        char filename[24];
        std::sprintf(filename, "level%s", ".fsh");
        int bytes;
        runtime::particles::lifecycle::load_fsh_file(
            runtime::level_file(filename), reinterpret_cast<Dword *>(&bytes),
            &at<Dword *>(owner, 24), lifecycle::level_pool | 1024);
        pending::register_level_texture_symbols(at<void *>(owner, 24));
        at<eagl::Loader *>(owner, 20) = graphics::load_module(
            runtime::level_file("levelG.o"), graphics::symbols::resolve_module,
            &at<void *>(owner, 16), lifecycle::level_pool | 1024);
        pending::unregister_level_texture_symbols(at<void *>(owner, 24));
    }
}

void unload_level_resources(void *owner) {
    kernel::print::check_console_write("levelmanager: start level deinit\n");
    if (void *data = at<void *>(owner, 12)) {
        kernel::memory_free(data);
        at<void *>(owner, 12) = nullptr;
        at<Byte>(owner, 8) = 0;
    }
    if (void *texture = at<void *>(owner, 24))
        kernel::memory_free(texture);
    at<void *>(owner, 24) = nullptr;
    if (void *data = at<void *>(owner, 16))
        kernel::memory_free(data);
    eagl::Loader *module = at<eagl::Loader *>(owner, 20);
    at<void *>(owner, 16) = nullptr;
    if (module) {
        eagl::loader_destroy(module);
        eagl::deallocator(module, 28);
    }
    at<void *>(owner, 20) = nullptr;
    if (void *data = at<void *>(owner, 4)) {
        lifecycle::construction::network::entries::release_payload(data);
        at<void *>(owner, 4) = nullptr;
    }
    void *definitions = pending::gob_definitions;
    if (definitions) {
        destroy_gob_definitions(definitions);
        kernel::memory_free(definitions);
    }
    pending::gob_definitions = nullptr;
    kernel::print::check_console_write("levelmanager: end level deinit\n");
}
} // namespace nfs::game::world
