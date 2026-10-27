#include "gui_images.hpp"
#include "nfs/graphics/primitives/immediate_draw.hpp"
#include "nfs/gui/lifecycle/gui_coordinates.hpp"
#include "nfs/graphics/display/viewport_transform.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include "gui_render_numeric_x87.hpp"
namespace gui_numeric = ::nfs::gui::retained;
namespace nfs::gui::engine::images::drawing {

namespace {
void emit(Dword *draw, const Point *point, const Dword *colour, float u, float v,
          const float *viewport) {
    float uv[4], position[4];
    graphics::gui_coordinates::vector(uv, u, v, 1, 1);
    graphics::gui_coordinates::screen_position(position, float(point->x), float(point->y),
                                               viewport[0], viewport[1], viewport[2], viewport[3]);
    graphics::immediate::textured_vertex(draw, position, colour, uv);
}
} // namespace

void __fastcall shape(ShapeImage *image, void *, const Point *first, const Point *second,
                      const Point *third, const Point *fourth, const Dword *colour, Dword *viewport,
                      Dword *draw) {
    if (!image->raster || !image->bounds.width || !image->bounds.height)
        return;
    float bounds[4];
    Dword near_plane, far_plane;
    graphics::viewports::read_rectangle(viewport, bounds, bounds + 1, bounds + 2, bounds + 3,
                                        &near_plane, &far_plane);
    graphics::immediate::set_texture(draw, reinterpret_cast<Dword>(image->raster));

    const float left = float(gui_numeric::number(0.5f) / gui_numeric::integer(image->bounds.width));
    const float top = float(gui_numeric::number(0.5f) / gui_numeric::integer(image->bounds.height));
    if (suspend_drawing())
        return;
    graphics::immediate::set_textured_primitive(draw, 5);
    const Dword packed_colour = *colour;
    emit(draw, first, &packed_colour, left, top, bounds);
    const float right = float(gui_numeric::number(1.f) - gui_numeric::number(left));
    emit(draw, second, &packed_colour, right, top, bounds);
    const float bottom = float(gui_numeric::number(1.f) - gui_numeric::number(top));
    emit(draw, fourth, &packed_colour, left, bottom, bounds);
    emit(draw, third, &packed_colour, right, bottom, bounds);
    graphics::immediate::end_textured_primitive(draw);
    graphics::immediate::flush_textured(draw);
}

void __fastcall shape_uv(ShapeImage *image, void *, const Point *first, const Point *second,
                         const Point *third, const Point *fourth, const Dword *colour,
                         const TexturePoint *uv_first, const TexturePoint *uv_second,
                         Dword *viewport, Dword *draw) {
    if (!image->raster)
        return;
    float bounds[4];
    Dword near_plane, far_plane;
    graphics::viewports::read_rectangle(viewport, bounds, bounds + 1, bounds + 2, bounds + 3,
                                        &near_plane, &far_plane);
    graphics::immediate::set_texture(draw, reinterpret_cast<Dword>(image->raster));
    if (suspend_drawing())
        return;
    graphics::immediate::set_textured_primitive(draw, 5);
    const Dword packed_colour = *colour;
    emit(draw, first, &packed_colour, uv_first->x, uv_first->y, bounds);
    emit(draw, second, &packed_colour, uv_second->x, uv_first->y, bounds);
    emit(draw, fourth, &packed_colour, uv_first->x, uv_second->y, bounds);
    emit(draw, third, &packed_colour, uv_second->x, uv_second->y, bounds);
    graphics::immediate::end_textured_primitive(draw);
    graphics::immediate::flush_textured(draw);
}

void __fastcall atlas(AtlasImage *image, void *, const Point *first, const Point *second,
                      const Point *third, const Point *fourth, const Dword *colour, Dword *viewport,
                      Dword *draw) {
    if (!image->raster || !image->atlas_bounds.width || !image->atlas_bounds.height)
        return;

    const auto atlas_width = gui_numeric::integer(image->atlas_bounds.width);
    const float atlas_height = float(gui_numeric::integer(image->atlas_bounds.height));
    const float left = float((gui_numeric::integer(image->bounds.left) + 0.5f) / atlas_width);
    const float top =
        float((gui_numeric::integer(image->bounds.top) + 0.5f) / gui_numeric::number(atlas_height));
    const int destination_width = int(Dword(second->x) - Dword(first->x));
    const float right =
        float((gui_numeric::integer(int(Dword(image->bounds.width) + Dword(image->bounds.left))) +
               gui_numeric::number(image->bounds.width < destination_width ? -0.5f : 0.5f)) /
              atlas_width);
    const int destination_height = int(Dword(fourth->y) - Dword(first->y));
    const float bottom =
        float((gui_numeric::integer(int(Dword(image->bounds.height) + Dword(image->bounds.top))) +
               gui_numeric::number(image->bounds.height < destination_height ? -0.5f : 0.5f)) /
              gui_numeric::number(atlas_height));
    float bounds[4];
    Dword near_plane, far_plane;
    graphics::viewports::read_rectangle(viewport, bounds, bounds + 1, bounds + 2, bounds + 3,
                                        &near_plane, &far_plane);
    graphics::immediate::set_texture(draw, reinterpret_cast<Dword>(image->raster));
    if (suspend_drawing())
        return;
    graphics::immediate::set_textured_primitive(draw, 5);
    const Dword packed_colour = *colour;
    emit(draw, first, &packed_colour, left, top, bounds);
    emit(draw, second, &packed_colour, right, top, bounds);
    emit(draw, fourth, &packed_colour, left, bottom, bounds);
    emit(draw, third, &packed_colour, right, bottom, bounds);
    graphics::immediate::end_textured_primitive(draw);
    graphics::immediate::flush_textured(draw);
}
} // namespace nfs::gui::engine::images::drawing
