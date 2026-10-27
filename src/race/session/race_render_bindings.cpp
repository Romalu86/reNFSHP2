#include "race_render_passes.hpp"
#include "nfs/graphics/primitives/immediate_draw.hpp"
#include "nfs/graphics/display/viewport_transform.hpp"
#include "../../math/precision/extended_float.hpp"
#include <cstring>
#include <initializer_list>
namespace nfs::game::runtime::physics::car_frame {
kernel::numeric::Extended turn_tangent(float);
}
namespace nfs::graphics::bindings {
Dword *drawing_viewport{};
Dword near_plane{}, far_plane{};
float half_fov_tangent{};

void use_screen_depth() {
    for (unsigned index = 0; index < 16; ++index)
        matrix[index] = viewports::identity_values[index];
    enabled = 1;
    for (Dword *state : {triangle_texture_alpha, triangle_texture, triangle_alpha, triangle,
                        line_alpha, line})
        geoprim::depth_method(state, 8);
}

void use_world_depth() {
    enabled = 0;
    geoprim::depth_method(triangle_texture_alpha, previous_depth_method);
    geoprim::depth_method(triangle_texture, previous_depth_method);
    geoprim::depth_method(triangle_alpha, previous_depth_method);
    geoprim::depth_method(triangle, previous_depth_method);
    geoprim::depth_method(line_alpha, previous_depth_method);
    geoprim::depth_method(line, previous_depth_method);
}
static void begin_viewport(Dword *viewport, Dword colour) {
    drawing_viewport = viewport;
    viewports::begin(viewport);
    Dword field_of_view, aspect;
    viewports::parameters(viewport, &field_of_view, &aspect, &near_plane, &far_plane);
    float degrees;
    std::memcpy(&degrees, &field_of_view, sizeof(degrees));
    using kernel::numeric::Extended;
    const auto format = kernel::numeric::current_format();
    const float turns = float(Extended(degrees, format) * 0.00277777784503996372222900390625f * 0.5f);
    half_fov_tangent = float(game::runtime::physics::car_frame::turn_tangent(turns));
    viewports::set_color(viewport, colour);
}

void begin_world_viewport(Dword *viewport) {
    begin_viewport(viewport, 3820858);
}

void begin_coloured_viewport(Dword *viewport, Dword colour) {
    begin_viewport(viewport, colour);
}
} // namespace nfs::graphics::bindings
