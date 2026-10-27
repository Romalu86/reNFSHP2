#include "race_frame_draw.hpp"
#include "nfs/physics/frame/simulation_runtime.hpp"
namespace nfs::game::world::frame {

void draw_actor_list(void *list, void *director, Dword *viewport) {
    void *node = at<void *>(list, 4);
    while (node) {
        if (at<Dword>(node, 156) != 6666)
            invoke<void>(bytes(node) - 16, 2, director, viewport);
        node = at<void *>(node, 12);
    }
}

void draw_character_manager(void *owner, void *director, Dword *viewport) {
    if (void *list = at<void *>(owner, 92))
        draw_actor_list(list, director, viewport);
}
static void draw_cell(void *cell, void *director, Dword *viewport) {
    if (!cell)
        return;
    void *node = at<void *>(cell, 16);
    while (node) {
        invoke<void>(node, 2, director, viewport);
        node = at<void *>(node, 8);
    }
}
// Incoming this is overwritten before use by the entry.
void draw_spatial_objects(void *, void *director, Dword *viewport) {
    draw_character_manager(subsystem(48), director, viewport);
    draw_cell(runtime::physics::global_cell(), director, viewport);
    int x, z;
    runtime::physics::cell_coordinates(reinterpret_cast<float *>(offset(director, 20)), &x, &z);
    for (int row = int(Dword(z) - 3u); row <= int(Dword(z) + 3u);
         row = int(Dword(row) + 1u))
        for (int column = int(Dword(x) - 3u); column <= int(Dword(x) + 3u);
             column = int(Dword(column) + 1u))
            draw_cell(runtime::physics::cell(column, row), director, viewport);
}
} // namespace nfs::game::world::frame
