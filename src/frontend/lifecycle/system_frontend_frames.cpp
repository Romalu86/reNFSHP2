#include "nfs/core/lifecycle/system_runtime.hpp"
namespace nfs::lifecycle::table_entries {

void __fastcall load_frontend_registry(System *, void *) {
    gui::clear_registry_objects();
}

void __fastcall unload_frontend_registry(System *, void *) {
    gui::clear_registry_objects();
}
} // namespace nfs::lifecycle::table_entries
