#include "nfs/world/lifecycle/world_lifecycle.hpp"
#include "nfs/resources/archives/archive_cache.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include "nfs/core/lifecycle/object_state.hpp"
#include "nfs/network/session/network_virtual.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
#include "nfs/core/console/console.hpp"
#include "../../math/vectors/retained_vector.hpp"
#include <initializer_list>
namespace nfs::application::pending {
void release_failed_catalogue_array(void *, const char *, kernel::Dword);
}
namespace nfs::game::world {
namespace {
template <class Construct>
void *allocate_objects(int count, unsigned stride, const char *label, Construct construct,
                       void (*destroy)(void *)) {
    auto *cookie = static_cast<Dword *>(files::archives::allocate_entry(
        int(Dword(count) * stride + 4), label, lifecycle::level_pool));
    if (!cookie)
        return nullptr;
    *cookie = Dword(count);
    try {
        lifecycle::construction::construct_array(cookie + 1, stride, count, construct, destroy);
    } catch (...) {
        application::pending::release_failed_catalogue_array(cookie, label, lifecycle::level_pool);
        throw;
    }
    return cookie + 1;
}
void delete_objects(void *objects, int stride, void (*destroy)(void *)) {
    auto *cookie = static_cast<Dword *>(objects) - 1;
    lifecycle::construction::objects::destroy_array(objects, stride, int(*cookie), destroy);
    lifecycle::construction::network::entries::release_payload(cookie);
}
} // namespace


void destroy_path_data(void *owner) {
    lifecycle::construction::network::entries::release_payload(at<void *>(owner, 0));
    lifecycle::construction::network::entries::release_payload(at<void *>(owner, 4));
    lifecycle::construction::network::entries::release_payload(at<void *>(owner, 8));
}

void load_path_data(void *owner, const void *definition) {
    int source_count = at<int>(const_cast<void *>(definition), 20);
    int count = int(Dword(source_count) * 3 - 2);
    at<int>(owner, 12) = count;
    auto *points = static_cast<float *>(files::archives::allocate_entry(
        int(Dword(count) * 12), "coord3 array", lifecycle::level_pool));
    at<float *>(owner, 0) = points;
    auto *tangents = static_cast<float *>(files::archives::allocate_entry(
        int(Dword(count) * 12), "coord3 array", lifecycle::level_pool));
    at<float *>(owner, 4) = tangents;
    auto *distances = static_cast<float *>(files::archives::allocate_entry(
        int(Dword(count) * 4), "float array", lifecycle::level_pool));
    at<float *>(owner, 8) = distances;
    interpolate_path(source_count, at<const float *>(const_cast<void *>(definition), 32), 0,
                     points);
    build_path_tangents(points, tangents, count);
    for (int index = 0; index < count; ++index) {
        if (index == count - 1) {
            distances[index] = 0;
        } else {
            float delta[3];
            for (unsigned component = 0; component < 3; ++component)
                delta[component] =
                    float(static_cast<long double>(points[index * 3 + 3 + component]) -
                          points[index * 3 + component]);
            distances[index] = float(runtime::retained_vector_length(delta));
        }
    }
    at<Dword>(owner, 64) = at<Dword>(const_cast<void *>(definition), 24);
    at<Dword>(owner, 60) = at<Dword>(const_cast<void *>(definition), 16);
    at<Byte>(owner, 68) = at<Dword>(const_cast<void *>(definition), 28) != 0;
}


void *construct_boom(void *owner) {
    at<const Dword *>(owner, 0) = pending::boom_base_methods;
    at<Dword>(owner, 8) = 0;
    at<Dword>(owner, 12) = 0;
    at<const Dword *>(owner, 0) = pending::boom_methods;
    kernel::memory_zero(offset(owner, 40), 36);
    for (unsigned field : {20u, 24u, 80u, 88u, 84u, 76u, 92u})
        at<Dword>(owner, field) = 0;
    at<Dword>(owner, 100) = at<Dword>(owner, 104) = at<Dword>(owner, 108) = 4294967295u;
    at<Dword>(owner, 112) = 0;
    at<Byte>(owner, 97) = 1;
    at<Byte>(owner, 98) = at<Byte>(owner, 116) = 0;
    return owner;
}

void destroy_boom(void *owner) {
    at<const Dword *>(owner, 0) = pending::boom_methods;
    at<const Dword *>(owner, 0) = pending::boom_base_methods;
}

void *construct_gob_database(void *owner) {
    Dword *definitions = pending::gob_definitions;
    const int definition_paths = int(definitions[7]);
    at<Dword>(owner, 8) = 0;
    at<Dword>(owner, 20) = 0;
    at<Dword>(owner, 12) = 0;
    at<Dword>(owner, 24) = 0;
    at<Dword>(owner, 28) = 0;
    for (int index = 0; index < definition_paths; ++index) {
        void *path = reinterpret_cast<Byte *>(definitions[16]) + index * 36;
        if (at<int>(path, 12) == 20) {
            at<void *>(owner, 28) = path;
            break;
        }
    }
    void *drive_path = at<void *>(owner, 28);
    if (!drive_path || at<int>(drive_path, 20) == 0) {
        at<Dword>(owner, 32) = 0;
        at<float>(owner, 36) = -1;
    } else {
        int count = at<int>(drive_path, 20);
        auto *distances = static_cast<float *>(files::archives::allocate_entry(
            int(Dword(count) * 4), "Drive Path Dists", lifecycle::level_pool));
        at<float *>(owner, 32) = distances;
        distances[0] = 0;
        const float *points = at<const float *>(at<void *>(owner, 28), 32);
        for (int index = 1; index < count; ++index)
            distances[index] =
                float(runtime::retained_point_distance(points + (index - 1) * 3, points + index * 3) +
                      distances[index - 1]);
        points = at<const float *>(at<void *>(owner, 28), 32);
        at<float>(owner, 36) =
            float(runtime::retained_point_distance(points, points + (count - 1) * 3) +
                  distances[count - 1]);
    }
    int paths = int(pending::gob_definitions[7]);
    at<int>(owner, 8) = paths;
    at<void *>(owner, 20) = paths == 0 ? nullptr
                                         : allocate_objects(paths, 72, "Path Data",
                                                            construct_path_data, destroy_path_data);
    for (int index = 0, count = at<int>(owner, 8); index < count; ++index)
        load_path_data(offset(at<void *>(owner, 20), index * 72),
                       reinterpret_cast<Byte *>(pending::gob_definitions[16]) +
                           index * 36);
    int locations = count_path_locations(pending::gob_definitions);
    at<int>(owner, 12) = locations;
    at<void *>(owner, 24) =
        locations == 0 ? nullptr
                       : allocate_objects(locations, 24, "Path Locs", construct_path_location,
                                          destroy_path_location);
    int booms = int(pending::gob_definitions[0]);
    at<int>(owner, 4) = booms;
    if (booms == 0) {
        at<void *>(owner, 16) = nullptr;
        at<void *>(owner, 40) = nullptr;
        at<Dword>(owner, 0) = 0;
        return owner;
    }
    at<void *>(owner, 16) = allocate_objects(booms, 120, "booms", construct_boom, destroy_boom);
    at<void *>(owner, 40) = files::archives::allocate_entry(int(Dword(at<int>(owner, 4)) * 4),
                                                              "boomCRCs", lifecycle::level_pool);
    at<Dword>(owner, 0) = 0;
    return owner;
}

void destroy_gob_database(void *owner) {
    kernel::print::check_console_write("GOBManager deconstruct\n");
    if (void *paths = at<void *>(owner, 20)) {
        kernel::print::check_console_write("mPaths\n");
        delete_objects(paths, 72, destroy_path_data);
        at<void *>(owner, 20) = nullptr;
    }
    if (void *distances = at<void *>(owner, 32)) {
        kernel::print::check_console_write("mDrivePathDists\n");
        lifecycle::construction::network::entries::release_payload(distances);
        at<void *>(owner, 32) = nullptr;
    }
    if (void *locations = at<void *>(owner, 24)) {
        kernel::print::check_console_write("mPathLocs\n");
        delete_objects(locations, 24, destroy_path_location);
        at<void *>(owner, 24) = nullptr;
    }
    if (void *booms = at<void *>(owner, 16)) {
        kernel::print::check_console_write("mBooms\n");
        invoke<void *>(booms, 1, 3u);
        at<void *>(owner, 16) = nullptr;
    }
    if (void *checksums = at<void *>(owner, 40)) {
        kernel::print::check_console_write("mBoomCRCs\n");
        lifecycle::construction::network::entries::release_payload(checksums);
        at<void *>(owner, 40) = nullptr;
    }
    kernel::print::check_console_write("GOBManager deconstruct end\n");
}

void load_gob_database(void *owner) {
    int count = at<int>(owner, 4);
    Byte *boom = at<Byte *>(owner, 16);
    while (count > 0) {
        pending::initialize_boom_entry(boom);
        boom += 120;
        --count;
    }
}

} // namespace nfs::game::world
