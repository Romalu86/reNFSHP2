#include "vehicle_renderable_frame.hpp"
#include "nfs/graphics/primitives/immediate_draw.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include "nfs/core/configuration/launch_options.hpp"
#include "../../gui/lifecycle/gui_images.hpp"
#include <cstdlib>
#include <cmath>
#include <cstring>
namespace nfs::game::runtime::ai::cars {
Dword car_is_traffic(void *);
}
namespace nfs::game::world::vehicle_render {
using kernel::numeric::Extended;
Dword &damage_bounds_enabled = launch::options.flags[57];
static Dword damage_bounds_state[17];
static Byte damage_bounds_guard{};
namespace visual_x87 {
void body_surface_brightness_native80(void *body, void *output);
}
// Preserve the public MSVC aggregate return convention.
Extended body_surface_brightness(void *body) {
    const auto format = kernel::numeric::current_format();
    unsigned char value[10];
    visual_x87::body_surface_brightness_native80(body, value);
    return Extended::from_native80(value, format);
}

static void destroy_damage_bounds_state() {
    graphics::geoprim::destroy(damage_bounds_state);
}

void draw_damage_zone_bounds(void *zone, const float *rotation, const float *position) {
    if (!(damage_bounds_guard & 1)) {
        damage_bounds_guard |= 1;
        lifecycle::construction::render_state(damage_bounds_state);
        std::atexit(destroy_damage_bounds_state);
    }
    graphics::geoprim::shading(damage_bounds_state, 1);
    graphics::geoprim::texture_enable(damage_bounds_state, 0);
    graphics::geoprim::alpha_test(damage_bounds_state, 0);
    graphics::geoprim::alpha_blend_mode(damage_bounds_state, 1);
    graphics::geoprim::transparency(damage_bounds_state, 1);
    graphics::geoprim::culling(damage_bounds_state, 0);
    graphics::geoprim::depth_method(damage_bounds_state, 8);
    const float origin[4] = {position[0], position[1], position[2], 1};
    float matrix[16];
    actor_resources::compose_actor_transform(matrix, rotation, origin);
    Dword colour = ((Dword(0) - Dword(at<Byte>(zone, 52) != 0)) & 4278190591u) - 256u;
    if (at<Byte>(zone, 53))
        colour = 4278255360u;
    auto *draw = graphics::bindings::single_draw;
    graphics::immediate::set_gouraud_state(draw, damage_bounds_state);
    std::memcpy(draw + 84 / 4, matrix, 64);
    constexpr unsigned corners[6][5] = {{0, 2, 3, 1, 0}, {4, 6, 7, 5, 4}, {0, 2, 6, 4, 0},
                                        {1, 3, 7, 5, 1}, {2, 6, 7, 3, 2}, {0, 4, 5, 1, 0}};
    for (const auto &face : corners) {
        if (gui::engine::images::suspend_drawing())
            break;
        graphics::immediate::set_primitive(draw, 3);
        for (unsigned corner : face) {
            float point[4] = {at<float>(zone, corner & 1 ? 40 : 28),
                              at<float>(zone, corner & 2 ? 44 : 32),
                              at<float>(zone, corner & 4 ? 48 : 36), 1};
            graphics::immediate::vertex(draw, point, &colour);
        }
        graphics::immediate::end_primitive(draw);
        graphics::immediate::flush(draw);
    }
}

void draw_damage_bounds(void *damage, const float *rotation, const float *position) {
    if (!at<Byte>(damage, 20) || !damage_bounds_enabled)
        return;
    for (unsigned i = 0; i < 10; ++i)
        draw_damage_zone_bounds(offset(at<void *>(damage, 76), i * 72), rotation, position);
}
} // namespace nfs::game::world::vehicle_render
