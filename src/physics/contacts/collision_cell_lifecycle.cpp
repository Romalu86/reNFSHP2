#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "nfs/core/lifecycle/object_state.hpp"
#include "nfs/physics/frame/simulation_runtime.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include "nfs/world/lifecycle/world_lifecycle.hpp"
#include <cstdlib>
#include <cstring>

namespace nfs::game::runtime::physics::cells {
extern const Dword cell_methods[6];


void *construct_cell(void *cell) {
    at<const Dword *>(cell, 8) = world::pending::boom_base_methods;
    at<Dword>(cell, 16) = at<Dword>(cell, 20) = 0;
    at<const Dword *>(cell, 8) = cell_methods;
    return cell;
}

// This destructor receives the embedded node, not the cell.
void destroy_cell_node(void *node) {
    at<const Dword *>(node, 0) = cell_methods;
    at<const Dword *>(node, 0) = world::pending::boom_base_methods;
}


void destroy_cell(void *cell) {
    destroy_cell_node(bytes(cell) + 8);
}


float *__fastcall cell_position(void *, void *, float *position) {
    std::memcpy(position, saved::zero_vector, 12);
    return position;
}

// The saved deleting destructor frees the node receiver.
void *__fastcall delete_cell_node(void *node, void *, Dword flags) {
    destroy_cell_node(node);
    if (flags & 1)
        kernel::memory_free(node);
    return node;
}

// Complete saved leaf: RET8.
void __fastcall draw_cell_node(void *, void *, Dword, void *) {
}

// Complete saved leaf: RET.
void __fastcall advance_cell_node(void *, void *) {
}


Dword __fastcall cell_body(void *, void *) {
    return 0;
}


void __fastcall cell_rotation(void *, void *, Dword *rotation) {
    std::memcpy(rotation, saved::identity_matrix, sizeof(saved::identity_matrix));
}

const Dword cell_methods[6] = {
    reinterpret_cast<Dword>(cell_position),  reinterpret_cast<Dword>(delete_cell_node),
    reinterpret_cast<Dword>(draw_cell_node), reinterpret_cast<Dword>(advance_cell_node),
    reinterpret_cast<Dword>(cell_body),      reinterpret_cast<Dword>(cell_rotation)};


void destroy_global_cell() {
    destroy_cell(pending::global_collision_cell);
}

 
void initialize_global_cell() {
    construct_cell(pending::global_collision_cell);
    std::atexit(destroy_global_cell);
}


void destroy_moved_cell() {
    destroy_cell(pending::spatial_storage + 98304);
}

 
void initialize_moved_cell() {
    construct_cell(pending::spatial_storage + 98304);
    std::atexit(destroy_moved_cell);
}


void destroy_collision_cells() {
    nfs::lifecycle::construction::objects::destroy_array(pending::collision_grid, 24,
                                                         4096, destroy_cell);
}

 
void initialize_collision_cells() {
    nfs::lifecycle::construction::construct_array(pending::collision_grid, 24, 4096,
                                                  construct_cell, destroy_cell);
    std::atexit(destroy_collision_cells);
}
} // namespace nfs::game::runtime::physics::cells

NFS_KERNEL_INITIALIZER(startup_initialize_global_cell, ".CRT$XCU_NFS_00690034",
                       nfs::game::runtime::physics::cells::initialize_global_cell);
NFS_KERNEL_INITIALIZER(startup_initialize_moved_cell, ".CRT$XCU_NFS_00690038",
                       nfs::game::runtime::physics::cells::initialize_moved_cell);
NFS_KERNEL_INITIALIZER(startup_initialize_collision_cells, ".CRT$XCU_NFS_0069003C",
                       nfs::game::runtime::physics::cells::initialize_collision_cells);
