#include "nfs/physics/solver/solver_runtime.hpp"
#include "nfs/resources/files/file_cache.hpp"
#include "nfs/resources/archives/big_directory.hpp"
#include "nfs/graphics/resources/eagl_assets.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include <cstdio>
namespace nfs::game::runtime::track {

void make_compartment_path(Dword *object, char *output) {
    std::sprintf(output, "%s%s\\comp%02d.viv", bytes(system(4)) + 524, bytes(system(40)) + 140,
                 object[0]);
}
namespace pending {

Dword load_resource(const char *path, Dword output, Dword flags) {
    return files::file::load_into(path, reinterpret_cast<void *>(output), static_cast<int>(flags));
}
// Label/flag arguments are not read by the callee.
void release_named(void *data, const char *, Dword) {
    if (data)
        kernel::memory_free(data);
}

Dword *construct_light_glows(Dword *object) {
    lifecycle::construction::render_state(object);
    at<Byte>(object, 84) = 0;
    at<Dword>(object, 76) = at<Dword>(object, 100) = at<Dword>(object, 80) = 0;
    return object;
}

void destroy_light_glows(Dword *object) {
    graphics::geoprim::base_destroy(object);
}

void reset_compartment(Dword *object) {
    if (Dword *pool = at<Dword *>(object, 160)) {
        particles::destroy_pool(pool);
        release_named(pool, "Particle Manager", lifecycle::track_pool | 256);
        at<Dword>(object, 160) = 0;
    }
    Dword *lights = at<Dword *>(object, 164);
    at<Dword>(object, 68) = 0;
    at<Byte>(object, 72) = 0;
    if (lights) {
        unload_light_glows_entry(lights);
        destroy_light_glows(lights);
        gui::application_release(lights);
        at<Dword>(object, 164) = 0;
    }
    direction_changed(object);
    at<Dword>(object, 60) = Dword(-1);
}

void load_compartment_entry(Dword *object, Dword destination) {
    char path[260];
    make_compartment_path(object, path);
    at<Dword>(object, 68) = destination;
    Dword loaded = load_resource(path, destination, 0);
    for (int retry = 0; !loaded && retry < 5; ++retry)
        loaded = load_resource(path, destination, 0);
    at<Byte>(object, 72) = 1;
}
// Existing BIG directory/FILE implementations supply real archive reads.
void bind_compartment(Dword *object, Dword) {
    Dword archive = at<Dword>(object, 68), offset, size;
    if (nfs::big_find_entry(reinterpret_cast<Byte *>(archive), "mesh.sim", 0, &offset, &size))
        if (Dword address = archive + offset)
            relocate_collision(object + 132 / 4, reinterpret_cast<Dword *>(address));
    Dword *server = system(108);
    if (at<Dword>(server, 44) && at<Dword>(server, 48))
        return;
    archive = at<Dword>(object, 68);
    at<Dword>(object, 160) = 0;
    if (nfs::big_find_entry(reinterpret_cast<Byte *>(archive), "particle.mpb", 0, &offset, &size)) {
        const Dword definitions = archive + offset;
        if (definitions && nfs::big_find_entry(reinterpret_cast<Byte *>(archive), "particle.fsh", 0,
                                               &offset, &size)) {
            const Dword image = archive + offset;
            if (image) {
                Dword *pool = static_cast<Dword *>(lifecycle::allocate(
                    816, "Particle Manager", lifecycle::track_pool | 256));
                if (pool) {
                    try {
                        pool = particles::construct_pool(pool, image, lifecycle::track_pool | 256);
                    } catch (...) {
                        release_named(pool, "Particle Manager", lifecycle::track_pool | 256);
                        throw;
                    }
                }
                at<Dword *>(object, 160) = pool;
                particles::pending::load_pool_entry(pool, reinterpret_cast<Dword *>(definitions));
                particles::enable_emitters(pool);
            }
        }
    }
    archive = at<Dword>(object, 68);
    Dword *definitions = nullptr;
    if (nfs::big_find_entry(reinterpret_cast<Byte *>(archive), "lightglow.dat", 0, &offset, &size))
        definitions = reinterpret_cast<Dword *>(archive + offset);
    // An unsuccessful lookup leaves size unchanged.
    if (static_cast<int>(size) > 8 && definitions) {
        Dword descriptor[4] = {definitions[0], reinterpret_cast<Dword>(definitions + 1),
                              definitions[1 + definitions[0] * 5],
                              reinterpret_cast<Dword>(definitions + 2 + definitions[0] * 5)};
        if (static_cast<int>(descriptor[0]) > 0)
            if (Dword lights = compartment_lights(system(36))) {
                Dword *glows = static_cast<Dword *>(lifecycle::allocate(
                    104, "Track LightGlow", lifecycle::track_pool | 256));
                if (glows) {
                    try {
                        glows = construct_light_glows(glows);
                    } catch (...) {
                        release_named(glows, "Track LightGlow", lifecycle::track_pool | 256);
                        throw;
                    }
                }
                at<Dword *>(object, 164) = glows;
                bind_light_glows_entry(glows, descriptor, lights);
            }
    }
}
} // namespace pending
} // namespace nfs::game::runtime::track
