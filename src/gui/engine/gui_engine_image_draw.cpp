#include "../lifecycle/gui_images.hpp"
#include "nfs/graphics/primitives/immediate_draw.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include "../lifecycle/gui_render_numeric_x87.hpp"
#include <cmath>
namespace gui_numeric = ::nfs::gui::retained;
namespace nfs::gui::engine {

Point *to_pixels(Point *out, const Point *source) {

    const float y = float(gui_numeric::integer(source->y) * gui_numeric::number(vertical_scale) +
                          gui_numeric::number(vertical_offset));
    const int pixel_y = int(gui_numeric::round_f32(y));
    const float x = float(gui_numeric::integer(source->x) * gui_numeric::number(horizontal_scale) +
                          gui_numeric::number(horizontal_offset));
    *out = Point{point_methods, int(gui_numeric::round_f32(x)), pixel_y};
    return out;
}

Point *top_left(Point *out, const Rectangle *rectangle) {
    *out = Point{point_methods, rectangle->left, rectangle->top};
    return out;
}

Point *top_right(Point *out, const Rectangle *rectangle) {
    *out =
        Point{point_methods, int(Dword(rectangle->left) + Dword(rectangle->width)), rectangle->top};
    return out;
}

Point *bottom_left(Point *out, const Rectangle *rectangle) {
    *out =
        Point{point_methods, rectangle->left, int(Dword(rectangle->top) + Dword(rectangle->height))};
    return out;
}

Point *bottom_right(Point *out, const Rectangle *rectangle) {
    *out = Point{point_methods, int(Dword(rectangle->left) + Dword(rectangle->width)),
                 int(Dword(rectangle->top) + Dword(rectangle->height))};
    return out;
}
namespace pending {
Dword update_scissor(Engine *);
namespace {
int compare_image_handle(const void *image, const void *key) {
    const Dword handle = static_cast<const images::Image *>(image)->handle;
    const Dword wanted = reinterpret_cast<Dword>(key);
    return handle < wanted ? -1 : wanted < handle;
}
void *image(Engine *engine, Dword handle) {
    return container_find(&engine->images, compare_image_handle, reinterpret_cast<void *>(handle));
}
void rectangle_points(const Rectangle *rectangle, Point *points) {
    Point corners[4];
    bottom_left(corners + 3, rectangle);
    to_pixels(points + 3, corners + 3);
    bottom_right(corners + 2, rectangle);
    to_pixels(points + 2, corners + 2);
    top_right(corners + 1, rectangle);
    to_pixels(points + 1, corners + 1);
    top_left(corners, rectangle);
    to_pixels(points, corners);
}
} // namespace

void __fastcall draw_image_uv(Engine *engine, void *, Dword handle, Dword bounds, Dword colour,
                              Dword uv_first, Dword uv_second, Dword blend) {
    if (images::suspend_drawing() || !update_scissor(engine))
        return;
    graphics::immediate::set_render_state(
        engine->textured_draw, blend == 1 ? engine->additive_state : engine->textured_state);
    if (void *frame = handle ? image(engine, handle) : nullptr) {
        Point points[4];
        rectangle_points(reinterpret_cast<const Rectangle *>(bounds), points);
        game::runtime::invoke(frame, 2, points, points + 1, points + 2, points + 3,
                              reinterpret_cast<const Dword *>(colour),
                              reinterpret_cast<const images::TexturePoint *>(uv_first),
                              reinterpret_cast<const images::TexturePoint *>(uv_second),
                              engine->viewport, engine->textured_draw);
    }
}

void __fastcall draw_image_quad(Engine *engine, void *, Dword handle, Dword first, Dword second,
                                Dword third, Dword fourth, Dword colour, Dword blend) {
    if (images::suspend_drawing() || !update_scissor(engine))
        return;
    graphics::immediate::set_render_state(
        engine->textured_draw, blend == 1 ? engine->additive_state : engine->textured_state);
    if (void *frame = handle ? image(engine, handle) : nullptr) {
        Point points[4];
        to_pixels(points + 3, reinterpret_cast<const Point *>(fourth));
        to_pixels(points + 2, reinterpret_cast<const Point *>(third));
        to_pixels(points + 1, reinterpret_cast<const Point *>(second));
        to_pixels(points, reinterpret_cast<const Point *>(first));
        game::runtime::invoke(frame, 1, points, points + 1, points + 2, points + 3,
                              reinterpret_cast<const Dword *>(colour), engine->viewport,
                              engine->textured_draw);
    }
}

void __fastcall draw_image_rectangle(Engine *engine, void *, Dword handle, Dword bounds, Dword colour,
                                     Dword blend) {
    if (images::suspend_drawing() || !update_scissor(engine))
        return;
    graphics::immediate::set_render_state(
        engine->textured_draw, blend == 1 ? engine->additive_state : engine->textured_state);
    if (void *frame = handle ? image(engine, handle) : nullptr) {
        Point points[4];
        rectangle_points(reinterpret_cast<const Rectangle *>(bounds), points);
        game::runtime::invoke(frame, 1, points, points + 1, points + 2, points + 3,
                              reinterpret_cast<const Dword *>(colour), engine->viewport,
                              engine->textured_draw);
    }
}
} // namespace pending
} // namespace nfs::gui::engine
