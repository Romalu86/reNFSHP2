#include "../lifecycle/gui_render_numeric_x87.hpp"
#include "../lifecycle/gui_images.hpp"
#include "nfs/graphics/primitives/immediate_draw.hpp"
#include "nfs/gui/lifecycle/gui_coordinates.hpp"
#include "nfs/graphics/display/viewport_transform.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include <cstring>
namespace gui_numeric = ::nfs::gui::retained;
namespace nfs::graphics::immediate {

void set_gouraud_state(Dword *draw, const Dword *state) {
    geoprim::assign(draw + 1, state);
    geoprim::primitive(draw + 1, draw[0]);
}

void vertex(Dword *draw, const float *position, const Dword *colour) {
    auto *vertices = reinterpret_cast<Dword *>(draw[76 / 4]);
    const Dword index = draw[80 / 4]++;
    auto *output = reinterpret_cast<float *>(vertices + index * 4);
    reinterpret_cast<Dword *>(output)[3] = *colour;
    output[0] = position[0] / position[3];
    output[1] = position[1] / position[3];
    output[2] = position[2] / position[3];
    const Dword count = draw[80 / 4];
    const int primitive = int(draw[0]);
    if (int(count + 3) < 16385)
        return;
    if (primitive == 4 && int(count) % 3 == 0) {
        flush(draw);
        return;
    }
    if (int(count + 2) < 16385)
        return;
    if (primitive == 2 && int(count) % 2 == 0) {
        flush(draw);
        return;
    }
    if (int(count + 1) < 16385)
        return;
    const Dword last_colour = vertices[count * 4 - 1];
    // Overflow retains XYZ only. Its uninitialized W is an unresolved stack/ABI case.
    float last[4];
    std::memcpy(last, vertices + count * 4 - 4, 12);
    if (primitive == 5 || primitive == 6) {
        Dword *retained = primitive == 5 ? vertices + (count - 2) * 4 : vertices;
        const Dword first_colour = retained[3];
        float first[4];
        std::memcpy(first, retained, 12);
        flush(draw);
        vertex(draw, first, &first_colour);
        vertex(draw, last, &last_colour);
    } else if (primitive == 3) {
        flush(draw);
        vertex(draw, last, &last_colour);
    } else if (primitive == 1 || primitive == -1)
        flush(draw);
}
} // namespace nfs::graphics::immediate
namespace nfs::gui::engine {
Point *to_pixels(Point *, const Point *);
Rectangle *to_pixels(Rectangle *, const Rectangle *);
namespace pending {
Dword update_scissor(Engine *);
namespace {
void emit_point(Dword *draw, int x, int y, Dword colour, const float *bounds) {
    float position[4];
    graphics::gui_coordinates::screen_position(position, float(x), float(y), bounds[0], bounds[1],
                                               bounds[2], bounds[3]);
    graphics::immediate::vertex(draw, position, &colour);
}
Dword *prepare_primitive(Engine *engine, float *bounds) {
    Dword near_plane, far_plane;
    graphics::viewports::read_rectangle(engine->viewport, bounds, bounds + 1, bounds + 2,
                                        bounds + 3, &near_plane, &far_plane);
    Dword *draw = graphics::bindings::single_draw;
    graphics::immediate::set_gouraud_state(draw, engine->flat_state);
    std::memcpy(draw + 84 / 4, engine->matrix, sizeof(engine->matrix));
    return draw;
}
} // namespace

void draw_pixel_line(Engine *engine, int first_x, int first_y, int second_x, int second_y,
                     Dword first_colour, Dword second_colour) {
    float bounds[4];
    Dword *draw = prepare_primitive(engine, bounds);
    if (images::suspend_drawing())
        return;
    graphics::immediate::set_primitive(draw, 3);
    emit_point(draw, first_x, first_y, first_colour, bounds);
    emit_point(draw, second_x, second_y, second_colour, bounds);
    graphics::immediate::end_primitive(draw);
    graphics::immediate::flush(draw);
}

static void draw_pixel_rectangle(Engine *engine, int left, int top, int width, int height,
                                 Dword first, Dword second, Dword third, Dword fourth) {
    float bounds[4];
    Dword *draw = prepare_primitive(engine, bounds);
    if (images::suspend_drawing())
        return;
    graphics::immediate::set_primitive(draw, 5);
    emit_point(draw, left, top, first, bounds);
    emit_point(draw, int(Dword(left) + Dword(width)), top, second, bounds);
    emit_point(draw, left, int(Dword(top) + Dword(height)), third, bounds);
    emit_point(draw, int(Dword(left) + Dword(width)), int(Dword(top) + Dword(height)), fourth, bounds);
    graphics::immediate::end_primitive(draw);
    graphics::immediate::flush(draw);
}

void __fastcall draw_line(Engine *engine, void *, Dword first, Dword second, Dword first_colour,
                          Dword second_colour) {
    if (!update_scissor(engine))
        return;
    Point begin, end;
    to_pixels(&begin, reinterpret_cast<const Point *>(first));
    to_pixels(&end, reinterpret_cast<const Point *>(second));
    draw_pixel_line(engine, begin.x, begin.y, end.x, end.y, first_colour, second_colour);
}

void __fastcall draw_rectangle(Engine *engine, void *, Dword rectangle, Dword first, Dword second,
                               Dword third, Dword fourth, Dword fill) {
    const auto *source = reinterpret_cast<const Rectangle *>(rectangle);
    if (images::suspend_drawing() || !update_scissor(engine) || !(source->width | source->height))
        return;
    Rectangle pixels;
    to_pixels(&pixels, source);
    const Dword first_colour = *reinterpret_cast<const Dword *>(first);
    const Dword second_colour = *reinterpret_cast<const Dword *>(second);
    const Dword third_colour = *reinterpret_cast<const Dword *>(third);
    const Dword fourth_colour = *reinterpret_cast<const Dword *>(fourth);
    if (!fill) {
        pixels.width = int(Dword(pixels.width) - engine->raster_inset);
        pixels.height = int(Dword(pixels.height) - engine->raster_inset);
        const int right = int(Dword(pixels.left) + Dword(pixels.width));
        const int bottom = int(Dword(pixels.top) + Dword(pixels.height));
        draw_pixel_line(engine, pixels.left, pixels.top, right, pixels.top, first_colour,
                        second_colour);
        draw_pixel_line(engine, right, pixels.top, right, bottom, second_colour, third_colour);
        draw_pixel_line(engine, right, bottom, pixels.left, bottom, third_colour, fourth_colour);
        draw_pixel_line(engine, pixels.left, bottom, pixels.left, pixels.top, fourth_colour,
                        first_colour);
    } else
        draw_pixel_rectangle(engine, pixels.left, pixels.top, pixels.width, pixels.height,
                             first_colour, second_colour, fourth_colour, third_colour);
}
} // namespace pending
} // namespace nfs::gui::engine
