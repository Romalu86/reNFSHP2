#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "nfs/gui/engine/gui_engine.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include <cstdlib>

namespace nfs::gui::engine::startup {

void destroy_default_bounds() {
    destroy_rectangle(&default_bounds);
}

 
void initialize_default_bounds() {
    construct_rectangle(&default_bounds);
    std::atexit(destroy_default_bounds);
}

// Restore the shared base table.
void destroy_default_point() {
    destroy_point(&default_point);
}

 
void initialize_default_point() {
    construct_point(&default_point);
    std::atexit(destroy_default_point);
}


void destroy_shared_string() {
    gui::string_destruct(&lifecycle::construction::unresolved::shared_string);
}

 
void initialize_shared_string() {
    gui::string_construct(&lifecycle::construction::unresolved::shared_string);
    std::atexit(destroy_shared_string);
}


void destroy_source_bounds() {
    destroy_rectangle(&source_bounds);
}

 
void initialize_source_bounds() {
    construct_rectangle_bounds(&source_bounds, 0, 0, 640, 480);
    std::atexit(destroy_source_bounds);
}


void destroy_destination_bounds() {
    destroy_rectangle(&destination_bounds);
}

 
void initialize_destination_bounds() {
    construct_rectangle_bounds(&destination_bounds, 0, 0, 640, 480);
    std::atexit(destroy_destination_bounds);
}
} // namespace nfs::gui::engine::startup

NFS_KERNEL_INITIALIZER(startup_initialize_default_bounds, ".CRT$XCU_NFS_006906CC",
                       nfs::gui::engine::startup::initialize_default_bounds);
NFS_KERNEL_INITIALIZER(startup_initialize_default_point, ".CRT$XCU_NFS_006906D0",
                       nfs::gui::engine::startup::initialize_default_point);
NFS_KERNEL_INITIALIZER(startup_initialize_shared_string, ".CRT$XCU_NFS_006906E0",
                       nfs::gui::engine::startup::initialize_shared_string);
NFS_KERNEL_INITIALIZER(startup_initialize_source_bounds, ".CRT$XCU_NFS_006906F0",
                       nfs::gui::engine::startup::initialize_source_bounds);
NFS_KERNEL_INITIALIZER(startup_initialize_destination_bounds, ".CRT$XCU_NFS_006906F4",
                       nfs::gui::engine::startup::initialize_destination_bounds);
