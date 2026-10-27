#include "hud_draw_primitives.hpp"
#include "../lifecycle/gui_images.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include <cstring>
namespace nfs::graphics::bindings {
Dword hud_texture{};
std::uint8_t hud_alpha{};
float hud_depth = -1;

void bind_hud_texture(Dword texture) {
    hud_texture = texture;
}

void set_hud_alpha(std::uint8_t alpha) {
    hud_alpha = alpha;
}

void set_hud_depth(float depth) {
    hud_depth = depth;
}

void set_hud_matrix(const Dword *source) {
    std::memcpy(matrix, source, sizeof(matrix));
}

Dword rear_view_texture(void *manager, Dword index) {
    return game::runtime::at<Dword>(game::runtime::at<void *>(manager, 400), 8 + index * 108);
}

void draw_hud_triangle(const HudVertex *first, const HudVertex *second, const HudVertex *third) {
    auto *draw = single_draw;
    std::memcpy(draw + 84 / 4, matrix, sizeof(matrix));
    immediate::set_gouraud_state(draw, hud_alpha ? triangle_alpha : triangle);
    if (gui::engine::images::suspend_drawing())
        return;
    immediate::set_primitive(draw, 4);
    for (auto *vertex : {first, second, third}) {
        const float position[4] = {vertex->x, vertex->y, enabled ? hud_depth : vertex->z, 1};
        immediate::vertex(draw, position, &vertex->colour);
    }
    immediate::end_primitive(draw);
    immediate::flush(draw);
}

void draw_hud_textured_triangle(const HudVertex *first, const HudVertex *second,
                                const HudVertex *third) {
    immediate::set_texture(single_draw_texture, hud_texture);
    auto *draw = single_draw_texture;
    std::memcpy(draw + 84 / 4, matrix, sizeof(matrix));
    immediate::set_render_state(draw, hud_alpha ? triangle_texture_alpha : triangle_texture);
    if (gui::engine::images::suspend_drawing())
        return;
    immediate::set_textured_primitive(draw, 4);
    for (auto *vertex : {first, second, third}) {
        const float position[4] = {vertex->x, vertex->y, enabled ? hud_depth : vertex->z, 1};
        const float uv[4] = {vertex->u, vertex->v, 1, 1};
        immediate::textured_vertex(draw, position, &vertex->colour, uv);
    }
    immediate::end_textured_primitive(draw);
    immediate::flush_textured(draw);
}

void draw_hud_quad(const HudVertex *first, const HudVertex *second, const HudVertex *third,
                   const HudVertex *fourth) {
    if (hud_texture) {
        draw_hud_textured_triangle(first, second, third);
        draw_hud_textured_triangle(first, third, fourth);
    } else {
        draw_hud_triangle(first, second, third);
        draw_hud_triangle(first, third, fourth);
    }
}
} // namespace nfs::graphics::bindings
