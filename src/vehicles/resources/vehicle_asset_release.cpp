#include "nfs/world/resources/runtime_resources.hpp"
#include "nfs/graphics/backend/eagl.hpp"
#include "nfs/core/lifecycle/system_runtime.hpp"
namespace nfs::game::runtime {

void release_vehicle_asset(Dword *object) {
    for (unsigned index = 3; index <= 7; ++index)
        if (object[index]) {
            kernel::memory_free(reinterpret_cast<void *>(object[index]));
            object[index] = 0;
        }
    for (unsigned index = 0; index < 3; ++index) {
        auto *loader = reinterpret_cast<eagl::Loader *>(object[index]);
        if (loader) {
            eagl::loader_destroy(loader);
            eagl::deallocator(loader, 28);
        }
        object[index] = 0;
    }
    object[9] = 0;
    object[8] = 4294967295u;
}

void release_vehicle_assets(Dword *object) {
    for (unsigned index = 0; index < 12; ++index)
        release_vehicle_asset(object + 3 + index * 10);
    for (unsigned index = 0; index < 68; ++index) {
        Dword *entry = object + 123 + index * 4;
        if (entry[0]) {
            kernel::memory_free(reinterpret_cast<void *>(entry[0]));
            entry[0] = 0;
        }
        entry[3] = 0;
    }
    for (unsigned index = 0; index < 3; ++index)
        if (object[index]) {
            kernel::memory_free(reinterpret_cast<void *>(object[index]));
            object[index] = 0;
        }
}
} // namespace nfs::game::runtime
namespace nfs::lifecycle::table_entries {

void __fastcall shutdown_vehicle_assets(System *object, void *) {
    game::runtime::release_vehicle_assets(reinterpret_cast<kernel::Dword *>(object) + 78600 / 4);
}
} // namespace nfs::lifecycle::table_entries
