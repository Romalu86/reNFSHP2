#include "nfs/core/services/service_lifecycle.hpp"
#include "nfs/graphics/configuration/render_settings.hpp"
#include "nfs/core/lifecycle/object_state.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include "nfs/core/lifecycle/system_destruction.hpp"
#include "nfs/graphics/resources/eagl_graphics.hpp"
#include "nfs/resources/archives/archive_cache.hpp"
#include "nfs/resources/ini/ini.hpp"
#include <cstdio>

namespace nfs::services {
namespace {
void target_destructor(void *target) {
    lifecycle::construction::destruction::environment_target(static_cast<Dword *>(target));
}
} // namespace

void initialize_environment(void *object) {
    create_environment_targets(object);
}

void shutdown_environment(void *object) {
    release_environment_targets(object);
}

void create_environment_targets(void *object) {
    field<Dword>(object, 596) = 0;
    field<Dword>(object, 592) = 1;
    field<Dword>(object, 588) = 2;
    void *caps = graphics::startup::display_caps;
    if (!caps || (field<Byte>(caps, 760) & 1))
        return;
    field<Dword>(object, 20) = 6;
    field<Dword>(object, 16) = field<Dword>(object, 12) = field<Dword>(object, 8) = 255;
    char filename[260];
    const char *paths = reinterpret_cast<char *>(lifecycle::manager->slots[1]);
    std::sprintf(filename, "%s%s", paths + 2084, "reflection.ini");
    ini::Reader reader;
    ini::reader_construct(&reader, filename, 0);
    try {
        ini::get_int(&reader, "brightness", "red", &field<int>(object, 8));
        ini::get_int(&reader, "brightness", "green", &field<int>(object, 12));
        ini::get_int(&reader, "brightness", "blue", &field<int>(object, 16));
        void *allocation = files::archives::allocate_entry(652, "REFLECTION RENDER TARGET LIST",
                                                           lifecycle::track_pool);
        void *targets = nullptr;
        if (allocation) {
            field<Dword>(allocation, 0) = 6;
            targets = static_cast<Byte *>(allocation) + 4;
            try {
                lifecycle::construction::construct_array(
                    targets, 108, 6, lifecycle::construction::objects::environment_target,
                    target_destructor);
            } catch (...) {
                gui::application_release(allocation);
                throw;
            }
        }
        field<void *>(object, 400) = targets;
        const Dword colour_depth = graphics::startup::settings.race.colour_depth;
        pending::create_reflection_target(static_cast<Byte *>(object) + 404, 0, 128, 128, colour_depth,
                                          128.0f, 128.0f, 4278190080u);
        const Dword colours[] = {4289331370u, 4289331200u, 4278233600u,
                                4278190250u, 4294967120u, 4294967295u};
        for (unsigned index = 0; index < 6; ++index)
            pending::create_reflection_target(static_cast<Byte *>(targets) + index * 108, 1, 128,
                                              128, colour_depth, 128.0f, 128.0f, colours[index]);
        pending::initialize_reflection_camera(object);
        void *vehicles = lifecycle::manager->slots[52 / 4];
        field<Byte>(object, 396) = field<Dword>(vehicles, 81272) == 2 ? 1 : 0;
        field<Byte>(object, 4) = 1;
    } catch (...) {
        ini::reader_clear(&reader);
        throw;
    }
    ini::reader_clear(&reader);
}

void release_environment_targets(void *object) {
    field<Byte>(object, 4) = 0;
    if (void *meshes = field<void *>(object, 512)) {
        lifecycle::construction::objects::destroy_array(
            meshes, 128, field<int>(static_cast<Byte *>(meshes) - 4, 0),
            pending::destroy_reflection_mesh);
        eagl::deallocator(static_cast<Byte *>(meshes) - 4, 128);
    }
    field<void *>(object, 512) = nullptr;
    if (void *items = field<void *>(object, 516)) {
        lifecycle::construction::objects::destroy_array(
            items, 84, field<int>(static_cast<Byte *>(items) - 4, 0),
            pending::destroy_reflection_item);
        eagl::deallocator(static_cast<Byte *>(items) - 4, 84);
    }
    field<void *>(object, 516) = nullptr;
    pending::release_reflection_target(static_cast<Byte *>(object) + 404);
    if (void *targets = field<void *>(object, 400)) {
        for (unsigned index = 0; index < 6; ++index)
            pending::release_reflection_target(static_cast<Byte *>(targets) + index * 108);
        lifecycle::construction::objects::destroy_array(
            targets, 108, field<int>(static_cast<Byte *>(targets) - 4, 0), target_destructor);
        gui::application_release(static_cast<Byte *>(targets) - 4);
    }
    field<void *>(object, 400) = nullptr;
}
} // namespace nfs::services
