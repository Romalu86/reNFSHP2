#include "vehicle_renderable_resources.hpp"
#include "nfs/resources/archives/archive_cache.hpp"
#include "nfs/resources/files/file_cache.hpp"
#include "nfs/network/packets/network_packet_handlers.hpp"
#include "nfs/graphics/fonts/font_faces.hpp"
#include "nfs/graphics/resources/eagl_assets.hpp"
#include "nfs/graphics/particles/particle_lifecycle.hpp"
#include "nfs/physics/frame/simulation_runtime.hpp"
#include <cstdio>
#include <cstring>
namespace nfs::application::pending {
void clear_catalogue_asset_group(kernel::Dword *);
std::uint8_t catalogue_car_is_traffic(void *, int);
} // namespace nfs::application::pending
namespace nfs::game::world::vehicle_render {
char resource_archive_path[260]{};

const char *catalogue_resource(void *catalogue, Dword index) {
    for (int entry = 0; entry < at<int>(catalogue, 0); ++entry)
        if (at<Dword>(catalogue, 4 + Dword(entry) * 1228) == index)
            return reinterpret_cast<const char *>(offset(catalogue, 8 + Dword(entry) * 1228));
    return nullptr;
}

Dword catalogue_index(void *catalogue, const char *name) {
    for (int index = 0; index < at<int>(catalogue, 0); ++index)
        if (!_stricmp(
                name, reinterpret_cast<const char *>(offset(catalogue, 8 + Dword(index) * 1228))))
            return index == -1 ? Dword(-1) : at<Dword>(catalogue, 4 + Dword(index) * 1228);
    return Dword(-1);
}

void *catalogue_skin(void *catalogue, Dword index, Dword skin) {
    for (int item = 0; item < at<int>(catalogue, 0); ++item) {
        if (at<Dword>(catalogue, 4 + Dword(item) * 1228) != index)
            continue;
        Dword field = skin == 16 ? 1032 : skin == 15 ? 1084 : 68 + skin * 48;
        return offset(catalogue, Dword(item) * 1228 + field);
    }
    return offset(catalogue, 68);
}
// Only AL is consumed by the callers.
Byte catalogue_addons(void *catalogue, Dword index, Dword skin) {
    for (int item = 0; item < at<int>(catalogue, 0); ++item) {
        if (at<Dword>(catalogue, 4 + Dword(item) * 1228) != index)
            continue;
        Dword field = skin == 16 ? 1048 : skin == 15 ? 1100 : 84 + skin * 48;
        return at<Byte>(catalogue, Dword(item) * 1228 + field);
    }
    return 0;
}

float catalogue_trunk_bounce(void *catalogue, Dword index) {
    for (int item = 0; item < at<int>(catalogue, 0); ++item)
        if (at<Dword>(catalogue, 4 + Dword(item) * 1228) == index)
            return at<float>(catalogue, 1136 + Dword(item) * 1228);
    return 0;
}

float catalogue_hood_bounce(void *catalogue, Dword index) {
    for (int item = 0; item < at<int>(catalogue, 0); ++item)
        if (at<Dword>(catalogue, 4 + Dword(item) * 1228) == index)
            return at<float>(catalogue, 1140 + Dword(item) * 1228);
    return 0;
}

float catalogue_wheel_speed(void *catalogue, Dword index) {
    for (int item = 0; item < at<int>(catalogue, 0); ++item)
        if (at<Dword>(catalogue, 4 + Dword(item) * 1228) == index)
            return at<float>(catalogue, 1148 + Dword(item) * 1228);
    return 0;
}

void load_archive_resources(void **resources, const char *name) {
    std::sprintf(resource_archive_path, "%s%s\\%s", offset(subsystem(4), 2084), name, "car.viv");
    track_render::TextureOptions options;
    track_render::begin_texture_options(&options, &graphics::textures::database);
    try {
        options.flags[2] = graphics::startup::mipmaps_enabled(&graphics::startup::settings);
        options.flags[4] = 1;
        if (!at<Byte>(subsystem(52), 81296))
            options.flags[5] = 1;
        if (!at<Dword>(subsystem(72), 2380))
            options.flags[3] = 1;
        resources[0] = track_render::load_persistent_fsh(&graphics::textures::database,
                                                         resource_archive_path, "car.fsh");
        resources[1] = track_render::load_persistent_fsh(&graphics::textures::database,
                                                         resource_archive_path, "damage.fsh");
        bool mirrored = false;
        if (!at<Byte>(subsystem(52), 81296)) {
            auto *rules = static_cast<Dword *>(subsystem(72));
            mirrored = network::packets::rule_value(rules, 6) == 2;
            if (!mirrored)
                mirrored = network::packets::rule_value(rules, 6) == 3;
        }
        bool damage = at<Dword>(subsystem(52), 81280) == 1;
        const char *model =
            mirrored ? (damage ? "carM.o" : "carRigidM.o") : (damage ? "car.o" : "carRigid.o");
        resources[2] =
            files::archives::load(resource_archive_path, model, lifecycle::cars_pool | 1024, 0);
        resources[3] = files::archives::load(resource_archive_path, "shadow.o",
                                             lifecycle::cars_pool | 1024, 0);
        resources[4] = files::archives::load(resource_archive_path, "skeleton.o",
                                             lifecycle::cars_pool | 1024, 0);
    } catch (...) {
        track_render::end_texture_options(&options);
        throw;
    }
    track_render::end_texture_options(&options);
}
namespace {
eagl::Loader *link_resource(void *image, const char *label) {
    Dword bytes = kernel::memory_size(image);
    auto *loader =
        static_cast<eagl::Loader *>(lifecycle::allocate(28, label, lifecycle::cars_pool));
    if (loader) {
        try {
            eagl::loader_construct(loader, static_cast<eagl::ElfHeader *>(image), bytes,
                                   graphics::symbols::resolve_module);
        } catch (...) {
            gui::application_release(loader);
            throw;
        }
    }
    return loader;
}
} // namespace

void acquire_resource_group(void *cache, Dword catalog, const char *name, ResourceGroup *result) {
    ResourceGroup *group = nullptr;
    for (unsigned i = 0; i < 12; ++i) {
        auto *candidate = reinterpret_cast<ResourceGroup *>(offset(cache, 12 + i * 40));
        if (candidate->car_image && candidate->catalog == catalog) {
            ++candidate->references;
            std::memcpy(result, candidate, sizeof(*result));
            return;
        }
    }
    for (unsigned i = 0; i < 12; ++i) {
        group = reinterpret_cast<ResourceGroup *>(offset(cache, 12 + i * 40));
        if (!group->car_image)
            break;
    }
    group->car_image = nullptr;
    group->car = group->shadow = group->skeleton = nullptr;
    group->catalog = catalog;
    group->references = 1;
    load_archive_resources(&group->images, name);
    if (graphics::device) {
        void *car = group->car_image, *shadow = group->shadow_image,
             *skeleton = group->skeleton_image;
        if (car)
            group->car = link_resource(car, "VehicleResource: Car Dynamic Loader");
        if (skeleton)
            group->skeleton = link_resource(skeleton, "VehicleResource: Skeleton Dynamic Loader");
        if (shadow)
            group->shadow = link_resource(shadow, "VehicleResource: Skeleton Dynamic Loader");
    }
    for (unsigned i = 0; i < 12; ++i) {
        auto *candidate = reinterpret_cast<ResourceGroup *>(offset(cache, 12 + i * 40));
        if (!candidate->car_image)
            break;
        application::pending::catalogue_car_is_traffic(offset(subsystem(52), 4),
                                                       int(candidate->catalog));
    }
    std::memcpy(result, group, sizeof(*result));
}

void release_resource_group(void *cache, Dword catalog) {
    for (unsigned i = 0; i < 12; ++i) {
        auto *group = reinterpret_cast<ResourceGroup *>(offset(cache, 12 + i * 40));
        if (group->car_image && group->catalog == catalog) {
            if (!--group->references)
                application::pending::clear_catalogue_asset_group(reinterpret_cast<Dword *>(group));
            return;
        }
    }
}

void *acquire_skin(void *cache, Dword catalog, Dword skin, const char *name) {
    for (unsigned i = 0; i < 68; ++i) {
        void *entry = offset(cache, 492 + i * 16);
        if (at<void *>(entry, 0) && at<Dword>(entry, 8) == catalog && at<Dword>(entry, 4) == skin) {
            ++at<Dword>(entry, 12);
            return at<void *>(entry, 0);
        }
    }
    unsigned slot = 0;
    while (at<void *>(cache, 492 + slot * 16))
        if (++slot >= 68)
            return &runtime::particles::lifecycle::state<Byte>(7217172);
    void *entry = offset(cache, 492 + slot * 16);
    at<Dword>(entry, 4) = skin;
    at<Dword>(entry, 8) = catalog;
    at<Dword>(entry, 12) = 1;
    char filename[260];
    if (skin == 15 || skin == 16)
        std::sprintf(filename, "%s%s", skin == 15 ? "skincop" : "skinhp", ".fsh");
    else
        std::sprintf(filename, "%s%02d%s", "skin", skin, ".fsh");
    track_render::TextureOptions options;
    track_render::begin_texture_options(&options, &graphics::textures::database);
    void *image;
    try {
        options.flags[2] = graphics::startup::mipmaps_enabled(&graphics::startup::settings);
        options.flags[4] = 1;
        if (!at<int>(subsystem(72), 2380))
            options.flags[3] = 1;
        if (!at<Byte>(subsystem(52), 81296))
            options.flags[5] = 1;
        std::sprintf(resource_archive_path, "%s%s\\%s", offset(subsystem(4), 2084), name,
                     "skin.viv");
        image = track_render::load_persistent_fsh(&graphics::textures::database,
                                                  resource_archive_path, filename);
        at<void *>(entry, 0) = image;
    } catch (...) {
        track_render::end_texture_options(&options);
        throw;
    }
    track_render::end_texture_options(&options);
    return image;
}

void release_skin(void *cache, Dword catalog, Dword skin) {
    for (unsigned i = 0; i < 68; ++i) {
        void *entry = offset(cache, 492 + i * 16);
        if (at<void *>(entry, 0) && at<Dword>(entry, 8) == catalog && at<Dword>(entry, 4) == skin) {
            if (!--at<Dword>(entry, 12)) {
                kernel::memory_free(at<void *>(entry, 0));
                at<void *>(entry, 0) = nullptr;
            }
            return;
        }
    }
}
} // namespace nfs::game::world::vehicle_render
