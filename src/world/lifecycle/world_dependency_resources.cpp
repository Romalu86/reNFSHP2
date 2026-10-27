#include "nfs/world/lifecycle/world_dependency.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
#include "nfs/graphics/resources/eagl_assets.hpp"
#include "nfs/graphics/particles/particle_lifecycle.hpp"
#include "nfs/resources/files/file_cache.hpp"
#include "nfs/graphics/textures/texture_conversion.hpp"
#include "nfs/graphics/configuration/display_configuration.hpp"
#include <cstdio>
#include <initializer_list>
namespace nfs::game::world::dependency {

void configure_track_material(void *owner, Dword sun_density, Dword sky_density, Dword sun_light,
                              Dword sky_light, Dword fog_mode, Dword color, Dword layer) {
    at<Dword>(owner, 108) = layer;
    at<Dword>(owner, 104) = fog_mode;
    at<Dword>(owner, 68) = color >> 24;
    at<Dword>(owner, 72) = (color >> 16) & 255;
    at<Dword>(owner, 80) = color & 255;
    at<Dword>(owner, 84) = sun_light;
    at<Dword>(owner, 92) = sun_density;
    at<Dword>(owner, 76) = (color >> 8) & 255;
    at<Dword>(owner, 88) = sky_light;
    at<Dword>(owner, 96) = sky_density;
}

void bind_track_texture_state(void *owner) {
    void *section = offset(owner, at<Dword>(owner, 48) * 336);
    configure_track_material(subsystem(128), at<Dword>(section, 172), at<Dword>(section, 176),
                             at<Dword>(section, 180), at<Dword>(section, 184),
                             at<Dword>(section, 188), at<Dword>(section, 192),
                             at<Dword>(section, 220));
}

void reset_track_directory(void *owner) {
    at<Dword>(owner, 24) = at<Dword>(owner, 88) = at<Dword>(owner, 28) = at<Dword>(owner, 84) =
        0;
    for (unsigned field = 32; field < 80; field += 4)
        at<Dword>(owner, field) = 0;
}

void release_compartment(void *owner) {
    if (auto *loader = at<eagl::Loader *>(owner, 76)) {
        eagl::loader_destroy(loader);
        runtime::track::pending::release_named(loader, "EDL file",
                                                        lifecycle::track_pool | 256);
        at<void *>(owner, 76) = nullptr;
    }
    if (void *data = at<void *>(owner, 64)) {
        kernel::memory_free(data);
        at<void *>(owner, 64) = nullptr;
    }
}

void destroy_sun_glow(void *owner) {
    graphics::geoprim::destroy(reinterpret_cast<Dword *>(offset(owner, 340)));
    graphics::geoprim::destroy(reinterpret_cast<Dword *>(offset(owner, 272)));
    world::destroy_light_glow(offset(owner, 24));
}

void unload_compartment_directory(void *owner) {
    for (unsigned field : {24u, 80u})
        if (auto *loader = at<eagl::Loader *>(owner, field)) {
            eagl::loader_destroy(loader);
            runtime::track::pending::release_named(loader, "EDL file",
                                                            lifecycle::level_pool);
            at<void *>(owner, field) = nullptr;
        }
    for (unsigned field : {20u, 0u, 4u, 16u, 12u, 8u})
        if (void *data = at<void *>(owner, field)) {
            kernel::memory_free(data);
            at<void *>(owner, field) = nullptr;
        }
    if (void *glow = at<void *>(owner, 88)) {
        destroy_sun_glow(glow);
        kernel::memory_free(glow);
        at<void *>(owner, 88) = nullptr;
    }
}

void load_shared_vehicle_textures(void *owner) {
    if (void *image = at<void *>(owner, 0)) {
        kernel::memory_free(image);
        at<void *>(owner, 0) = nullptr;
    }
    char filename[264];
    std::sprintf(filename, "%scommon.fsh", offset(subsystem(4), 2084));
    using namespace runtime::particles::lifecycle;
    TextureScratch scratch;
    acquire_texture_scratch(&scratch, &graphics::textures::database);
    try {
        if (files::file::load_into(filename, texture_scratch_data(&scratch),
                                   int(texture_scratch_capacity(&scratch))))
            at<void *>(owner, 0) = runtime::particles::lifecycle::pending::convert_fsh_image(
                &graphics::textures::database, texture_scratch_data(&scratch),
                lifecycle::cars_pool | 1024);
        if (void *image = at<void *>(owner, 8)) {
            kernel::memory_free(image);
            at<void *>(owner, 8) = nullptr;
        }
        std::sprintf(filename, "%s%s\\staticenvmap.fsh", offset(subsystem(4), 524),
                     offset(subsystem(40), 140));
        if (files::file::load_into(filename, texture_scratch_data(&scratch),
                                   int(texture_scratch_capacity(&scratch))))
            at<void *>(owner, 8) = runtime::particles::lifecycle::pending::convert_fsh_image(
                &graphics::textures::database, texture_scratch_data(&scratch),
                lifecycle::cars_pool | 1024);
    } catch (...) {
        release_texture_scratch(&scratch);
        throw;
    }
    release_texture_scratch(&scratch);
}

void release_vehicle_character_bindings(void *owner) {
    int count = at<int>(owner, 81304);
    Byte *vehicle = at<Byte *>(owner, 81268);
    while (count > 0) {
        pending::rebind_vehicle_character(vehicle);
        vehicle += 4064;
        --count;
    }
}

void release_car(void *owner) {
    runtime::physics::release_vehicle(at<void *>(owner, 1768));
}
// Receiver is the client pointer, not the car or particle manager.
void bind_car_client(void *client, void *car) {
    at<void *>(client, 368) = car;
    world::invoke(client, 6, 0u);
}

Dword bind_vehicle_slot(void *owner, Dword catalog, Dword variant) {
    Dword count = at<Dword>(owner, 81304);
    Byte *list = at<Byte *>(owner, 81268);
    pending::load_vehicle_renderable(list + count * 4064, catalog, variant);
    at<Dword>(owner, 81304) = count + 1;
    return reinterpret_cast<Dword>(list + count * 4064);
}

void initialize_track_objects() {
    pending::load_track_object_audio();
    int count = at<int>(runtime::track_collision, 4);
    for (int index = 0; index < count; ++index) {
        Dword displacement = Dword(index) * 120;
        void *object =
            std::int32_t(displacement) >= 0 && index < at<int>(runtime::track_collision, 4)
                ? offset(at<void *>(runtime::track_collision, 16), displacement)
                : nullptr;
        pending::initialize_track_object(object, index);
    }
}

void release_track_objects() {
    int count = at<int>(runtime::track_collision, 4);
    for (int index = 0; index < count; ++index) {
        Dword displacement = Dword(index) * 120;
        void *object =
            std::int32_t(displacement) >= 0 && index < at<int>(runtime::track_collision, 4)
                ? offset(at<void *>(runtime::track_collision, 16), displacement)
                : nullptr;
        if (void *vehicle = at<void *>(object, 80))
            runtime::physics::release_vehicle(vehicle);
    }
}

void initialize_boom(void *owner) {
    void *marker = at<void *>(owner, 76);
    if (!marker || at<std::uint16_t>(marker, 30) == 65535) {
        if (at<std::int16_t>(marker, 28) != 2)
            pending::insert_grid_object(owner);
        return;
    }
    unsigned parent_index = at<std::uint16_t>(marker, 30);
    void *parent = int(parent_index) < at<int>(runtime::track_collision, 4)
                       ? offset(at<void *>(runtime::track_collision, 16), parent_index * 120)
                       : nullptr;
    if (void *tail = at<void *>(parent, 20)) {
        while (at<void *>(tail, 24))
            tail = at<void *>(tail, 24);
        at<void *>(tail, 24) = owner;
    } else
        at<void *>(parent, 20) = owner;
}

const void *static_track_shape(void *owner, int index) {
    return offset(at<void *>(owner, 168), Dword(index) * 44);
}

void release_track_sky() {
    kernel::memory_free(static_references);
    static_references = nullptr;
}
} // namespace nfs::game::world::dependency
