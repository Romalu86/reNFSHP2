#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "gui_registry_owners.hpp"
#include <cstdlib>
namespace nfs::gui {
Container combo_boxes{}, tree_nodes{}, script_instances{}, global_script_variables{};
namespace {
std::uint8_t combo_boxes_destroyed{};
std::uint8_t tree_nodes_destroyed{};
std::uint8_t script_instances_destroyed{};
} // namespace

static void destroy_combo_boxes() {
    if (!(combo_boxes_destroyed & 1)) {
        combo_boxes_destroyed |= 1;
        container_destruct(&combo_boxes);
    }
}

static void initialize_combo_boxes() {
    container_construct(&combo_boxes);
    std::atexit(destroy_combo_boxes);
}

static void destroy_tree_nodes() {
    if (!(tree_nodes_destroyed & 1)) {
        tree_nodes_destroyed |= 1;
        container_destruct(&tree_nodes);
    }
}

static void initialize_tree_nodes() {
    container_construct(&tree_nodes);
    std::atexit(destroy_tree_nodes);
}

static void destroy_global_script_variables() {
    container_destruct(&global_script_variables);
}

void initialize_global_script_variables() {
    container_construct(&global_script_variables);
    std::atexit(destroy_global_script_variables);
}

static void destroy_script_instances() {
    if (!(script_instances_destroyed & 1)) {
        script_instances_destroyed |= 1;
        container_destruct(&script_instances);
    }
}

void initialize_script_instances() {
    container_construct(&script_instances);
    std::atexit(destroy_script_instances);
}
} // namespace nfs::gui
NFS_KERNEL_INITIALIZER(startup_initialize_combo_boxes, ".CRT$XCU_NFS_00690708",
                       nfs::gui::initialize_combo_boxes);
NFS_KERNEL_INITIALIZER(startup_initialize_tree_nodes, ".CRT$XCU_NFS_0069070C",
                       nfs::gui::initialize_tree_nodes);
