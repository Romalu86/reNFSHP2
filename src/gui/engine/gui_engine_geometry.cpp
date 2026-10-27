#include "nfs/gui/engine/gui_engine.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include "../lifecycle/gui_render_numeric_x87.hpp"
namespace gui_numeric = ::nfs::gui::retained;
namespace nfs::gui::engine {
Rectangle default_bounds{}, source_bounds{}, destination_bounds{};
Point default_point{};
float horizontal_scale = 1, vertical_scale = 1;
float inverse_horizontal_scale = 1, inverse_vertical_scale = 1;
float horizontal_offset{}, vertical_offset{};
 
Point *construct_point(Point *point) {
    point->methods = gui::base_methods;
    point->methods = point_methods;
    point->x = point->y = 0;
    return point;
}

Rectangle *construct_rectangle(Rectangle *rectangle) {
    rectangle->methods = gui::base_methods;
    rectangle->methods = rectangle_methods;
    rectangle->top = rectangle->left = rectangle->width = rectangle->height = 0;
    return rectangle;
}

Rectangle *construct_rectangle_bounds(Rectangle *rectangle, int left, int top, int width,
                                      int height) {
    rectangle->left = left;
    rectangle->top = top;
    rectangle->methods = rectangle_methods;
    rectangle->width = width;
    rectangle->height = height;
    return rectangle;
}

void destroy_rectangle(Rectangle *rectangle) {
    rectangle->methods = gui::base_methods;
}

static Rectangle *__fastcall delete_rectangle(Rectangle *rectangle, void *, unsigned flags) {
    destroy_rectangle(rectangle);
    if (flags & 1)
        gui::release(rectangle);
    return rectangle;
}
const Destroy rectangle_methods[] = {reinterpret_cast<Destroy>(delete_rectangle)};

void destroy_point(Point *point) {
    point->methods = gui::base_methods;
}

Point *__fastcall delete_point(Point *point, void *, unsigned flags) {
    destroy_point(point);
    if (flags & 1)
        gui::release(point);
    return point;
}
// The position-point owner has its own destructor identity.
void destroy_coordinate(Point *point) {
    point->methods = gui::base_methods;
}

Point *__fastcall delete_coordinate(Point *point, void *, unsigned flags) {
    destroy_coordinate(point);
    if (flags & 1)
        gui::release(point);
    return point;
}
const Destroy point_methods[] = {reinterpret_cast<Destroy>(delete_coordinate)};

void intersect_in_place(Rectangle *rectangle, const Rectangle *other) {
    const int left = rectangle->left > other->left ? rectangle->left : other->left;
    const int top = rectangle->top > other->top ? rectangle->top : other->top;
    const int right = int(Dword(rectangle->left) + Dword(rectangle->width));
    const int other_right = int(Dword(other->left) + Dword(other->width));
    const int bottom = int(Dword(rectangle->top) + Dword(rectangle->height));
    const int other_bottom = int(Dword(other->top) + Dword(other->height));
    const int width = int(Dword(right < other_right ? right : other_right) - Dword(left));
    const int height = int(Dword(bottom < other_bottom ? bottom : other_bottom) - Dword(top));
    rectangle->left = left;
    rectangle->top = top;
    rectangle->width = width > 0 ? width : 0;
    rectangle->height = height > 0 ? height : 0;
}
namespace pending {

Rectangle *intersect_rectangle(Rectangle *out, const Rectangle *first, const Rectangle *second) {
    Rectangle intersection{rectangle_methods, first->top, first->left, first->width, first->height};
    intersect_in_place(&intersection, second);
    *out = intersection;
    return out;
}
} // namespace pending

void configure_coordinates(const Rectangle *source, const Rectangle *destination) {
    if (!source->width || !source->height || !destination->width || !destination->height)
        return;
    const int source_width = source->width, source_height = source->height;
    const int destination_width = destination->width, destination_height = destination->height;
    const int source_left = source->left, source_top = source->top;
    source_bounds.height = source_height;
    source_bounds.width = source_width;
    destination_bounds.width = destination_width;
    destination_bounds.height = destination_height;
    destination_bounds.left = destination->left;
    destination_bounds.top = destination->top;
    const int left_offset = int(Dword(destination_bounds.left) - Dword(source_left));
    const int top_offset = int(Dword(destination_bounds.top) - Dword(source_top));
    source_bounds.top = source_top;
    source_bounds.left = source_left;

    horizontal_scale =
        float(gui_numeric::integer(destination_width) / gui_numeric::integer(source_width));
    vertical_scale =
        float(gui_numeric::integer(destination_height) / gui_numeric::integer(source_height));
    horizontal_offset = float(gui_numeric::integer(left_offset));
    vertical_offset = float(gui_numeric::integer(top_offset));
    inverse_horizontal_scale =
        float(gui_numeric::number(1.0f) / gui_numeric::number(horizontal_scale));
    inverse_vertical_scale = float(gui_numeric::number(1.0f) / gui_numeric::number(vertical_scale));
}
} // namespace nfs::gui::engine
