#include "race_world_draw.hpp"
#include "../../graphics/primitives/gouraud_array_runtime.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
#include "../../math/precision/extended_float.hpp"
#include "../../math/vectors/retained_vector.hpp"
namespace nfs::game::world::camera {
void active_camera_position(void *, float *);
}
namespace nfs::game::world::track_render {
static void draw_start_object(void *node, const float *eye) {
    if (!at<void *>(node, 4))
        return;
    const auto format = kernel::numeric::current_format();
    const auto distance = runtime::retained_point_distance(
        eye, reinterpret_cast<float *>(offset(node, 12)));
    if (kernel::numeric::Extended(300.f, format) < distance)
        return;
    graphics::backend::set_depth_write(graphics::context, 0);
    const Dword identity[16] = {1065353216, 0, 0, 0, 0, 1065353216, 0, 0,
                               0, 0, 1065353216, 0, 0, 0, 0, 1065353216};
    Dword *model = at<Dword *>(node, 4);
    graphics::gouraud::set_matrix(model, identity);
    graphics::gouraud::draw(model);
    graphics::backend::set_depth_write(graphics::context, 1);
}

void draw_start_nodes(void *node, const float *eye) {
    if (void *next = at<void *>(node, 8))
        draw_start_nodes(next, eye);
    draw_start_object(node, eye);
}

void draw_start_line(void *owner, void *director) {
    if (void *node = at<void *>(owner, 4)) {
        float eye[3];
        camera::active_camera_position(director, eye);
        if (void *next = at<void *>(node, 8))
            draw_start_nodes(next, eye);
        draw_start_object(node, eye);
    }
}
} // namespace nfs::game::world::track_render
