#include "hud_display.hpp"
#include "hud_draw_primitives.hpp"
#include "../../race/session/race_frame_draw.hpp"
#include "nfs/graphics/skids/skid_lifecycle.hpp"
namespace nfs::application::frontend::hud {
using game::runtime::invoke;
Dword rear_view_id{};

void __fastcall prepare_rear_view(RearView *widget, void *, gui::engine::Engine *engine) {
    gui::widgets::prepare_draw(reinterpret_cast<Widget *>(widget), nullptr, engine);
    pending::set_widget_visible(widget, game::world::frame::dynamic_reflection_enabled());
}
static const graphics::bindings::HudVertex mirror_vertices[4] = {
    {-1, 1, 0, 4294967295u, 1, .25f},
    {1, 1, 0, 4294967295u, 0, .25f},
    {1, -1, 0, 4294967295u, 0, .75f},
    {-1, -1, 0, 4294967295u, 1, .75f}};

void __fastcall draw_rear_view(RearView *, void *, gui::engine::Engine *engine) {
    if (!game::world::frame::dynamic_reflection_enabled())
        return;
    const Dword texture = graphics::bindings::rear_view_texture(game::runtime::system(32), 2);
    if (!texture)
        return;
    invoke(engine, 64, gui::engine::current_viewport_bounds(&engine->base));
    const auto *viewport_bounds = gui::engine::current_viewport_bounds(&engine->base);
    gui::engine::Rectangle bounds;
    bounds.methods = gui::engine::rectangle_methods;
    bounds.top = viewport_bounds->top;
    bounds.left = viewport_bounds->left;
    bounds.width = viewport_bounds->width;
    bounds.height = viewport_bounds->height;
    const Dword black = 4278190080u;
    invoke(engine, 48, &bounds, &black, Dword(1));
    invoke(engine, 56, Dword(1), Dword(1), Dword(0));
    graphics::bindings::set_hud_matrix(game::runtime::skids::lifecycle::identity_matrix);
    graphics::bindings::bind_hud_texture(texture);
    graphics::bindings::draw_hud_quad(mirror_vertices, mirror_vertices + 1, mirror_vertices + 2,
                                      mirror_vertices + 3);
    graphics::bindings::bind_hud_texture(0);
    invoke(engine, 65);
}
} // namespace nfs::application::frontend::hud
