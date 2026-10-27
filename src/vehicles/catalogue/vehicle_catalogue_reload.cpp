#include "../../network/session/server_runtime.hpp"
#include "nfs/world/lifecycle/world_lifecycle.hpp"
#include "nfs/core/lifecycle/object_state.hpp"
#include "nfs/resources/archives/archive_cache.hpp"
#include "nfs/graphics/backend/eagl.hpp"
namespace nfs::application::pending {
// Placement array-delete arguments after the allocation are ignored.
void release_failed_catalogue_array(void *allocation, const char *, Dword) {
    if (allocation)
        kernel::memory_free(allocation);
}

void clear_catalogue_asset_group(Dword *group) {
    for (unsigned i = 3; i < 8; ++i)
        if (group[i]) {
            kernel::memory_free(reinterpret_cast<void *>(group[i]));
            group[i] = 0;
        }
    for (unsigned i = 0; i < 3; ++i) {
        auto *loader = reinterpret_cast<eagl::Loader *>(group[i]);
        if (loader) {
            eagl::loader_destroy(loader);
            eagl::deallocator(loader, 28);
        }
        group[i] = 0;
    }
    group[9] = 0;
    group[8] = Dword(-1);
}

void clear_catalogue_assets(void *cache) {
    for (unsigned i = 0; i < 12; ++i)
        clear_catalogue_asset_group(reinterpret_cast<Dword *>(bytes(cache) + 12 + i * 40));
    for (unsigned i = 0; i < 68; ++i) {
        Dword *entry = reinterpret_cast<Dword *>(bytes(cache) + 492 + i * 16);
        if (entry[0]) {
            kernel::memory_free(reinterpret_cast<void *>(entry[0]));
            entry[0] = 0;
        }
        entry[3] = 0;
    }
}

void reload_catalogue_renderables(void *catalogue) {
    void *vehicles = at<void *>(catalogue, 81268);
    if (vehicles) {
        int count = at<int>(catalogue, 81304);
        if (count > 0) {
            Byte *vehicle = bytes(vehicles);
            do {
                game::world::pending::unload_vehicle_renderable(vehicle);
                vehicle += 4064;
                --count;
            } while (count);
            at<Dword>(catalogue, 81304) = 0;
        }
        lifecycle::construction::objects::destroy_array(
            vehicles, 4064, at<int>(bytes(vehicles) - 4, 0),
            game::world::pending::destroy_vehicle_renderable);
        kernel::memory_free(bytes(vehicles) - 4);
    }
    at<void *>(catalogue, 81268) = nullptr;
    clear_catalogue_assets(bytes(catalogue) + 78600);
    auto *allocation = static_cast<Dword *>(files::archives::allocate_entry(
        65028, "VEHICLE MANAGER VEHICLE LIST", lifecycle::cars_pool));
    if (!allocation) {
        at<void *>(catalogue, 81268) = nullptr;
        return;
    }
    allocation[0] = 16;
    try {
        lifecycle::construction::construct_array(
            allocation + 1, 4064, 16, game::world::pending::construct_vehicle_renderable,
            game::world::pending::destroy_vehicle_renderable);
    } catch (...) {
        // Release the allocation with its label and CLASS_CARS pool.
        release_failed_catalogue_array(allocation, "VEHICLE MANAGER VEHICLE LIST",
                                       lifecycle::cars_pool);
        throw;
    }
    at<void *>(catalogue, 81268) = allocation + 1;
}
} // namespace nfs::application::pending
