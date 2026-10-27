#include "race_render_passes.hpp"
#include "nfs/graphics/display/viewport_transform.hpp"
namespace nfs::graphics::model_render {
// The target selector has one canonical owner in model_render.
void enable_render_target(Dword *target) {
    set_active_target(target);
    target[132 / 4] = 1;
}

void disable_render_target(Dword *target) {
    set_active_target(nullptr);
    target[132 / 4] = 0;
}
} // namespace nfs::graphics::model_render
namespace nfs::graphics::passes {

void begin(TargetPass *pass) {
    if (!pass->enabled)
        return;
    viewports::set_matrix(pass->viewport, pass->matrix);
    if (pass->projection == 1)
        viewports::perspective(pass->viewport, pass->field_of_view, 1.f, pass->near_plane,
                               pass->far_plane);
    model_render::enable_render_target(pass->target);
    bindings::begin_coloured_viewport(pass->viewport, 4278190080u);
    viewports::clear(pass->viewport, 3);
}

void end(TargetPass *pass) {
    if (pass->enabled) {
        viewports::end(pass->viewport);
        model_render::disable_render_target(pass->target);
    }
}

void set_camera(TargetPass *pass, const float *matrix, float field_of_view, float aspect,
                 float near_plane, float far_plane) {
    for (unsigned index = 0; index < 16; ++index)
        pass->matrix[index] = matrix[index];
    pass->field_of_view = field_of_view;
    pass->aspect = aspect;
    pass->near_plane = near_plane;
    pass->far_plane = far_plane;
}
// The table has three quality levels.
void set_distance_level(ReflectionDistance *settings, int level) {
    static const float distances[] = {100.f, 50.f, 25.f};
    settings->distance = distances[level];
}
} // namespace nfs::graphics::passes
