#include "nfs/physics/frame/simulation_runtime.hpp"
#include <algorithm>
namespace nfs::game::runtime::physics::pending {
alignas(4) Byte spatial_storage[98352]{};
Byte (&collision_grid)[64 * 64 * 24] =
    *reinterpret_cast<Byte (*)[64 * 64 * 24]>(spatial_storage);
Byte (&global_collision_cell)[24] = *reinterpret_cast<Byte (*)[24]>(spatial_storage + 98328);
Byte (&moved_sentinel)[12] = *reinterpret_cast<Byte (*)[12]>(spatial_storage + 98312);
Dword *&moved_nodes = *reinterpret_cast<Dword **>(moved_sentinel + 8);
} // namespace nfs::game::runtime::physics::pending
namespace nfs::game::runtime::physics {

Byte *global_cell() {
    return pending::global_collision_cell;
}

Byte *cell(int x, int z) {
    if (x < 0 || x >= 64 || z < 0 || z >= 64)
        return nullptr;
    return pending::collision_grid + (x * 64 + z) * 24;
}
// Preserve the next link before the virtual position call.
void reinsert_moved_nodes() {
    Dword *node = pending::moved_nodes;
    while (node) {
        Dword *next = at<Dword *>(node, 8);
        float position[3];
        invoke(node, 0, position);
        int x, z;
        cell_coordinates(position, &x, &z);
        if (Byte *destination = cell(x, z)) {
            Dword *head = at<Dword *>(destination, 16);
            at<Dword *>(node, 8) = head;
            at<void *>(node, 12) = destination + 8;
            if (head)
                at<Dword *>(head, 12) = node;
            at<Dword *>(destination, 16) = node;
        }
        node = next;
    }
    pending::moved_nodes = nullptr;
}
} // namespace nfs::game::runtime::physics
