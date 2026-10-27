#include "gui_widget_interaction.hpp"
namespace nfs::gui::engine {

Rectangle *copy_rectangle_fields(Rectangle *out, const Rectangle *source) {
    out->left = source->left;
    out->top = source->top;
    const int height = source->height, width = source->width;
    out->height = height;
    out->width = width;
    return out;
}

Rectangle *union_copy(Rectangle *out, const Rectangle *first, const Rectangle *second) {
    Rectangle result{rectangle_methods, first->top, first->left, first->width, first->height};
    widgets::union_rectangle(&result, second);
    *out = result;
    return out;
}
} // namespace nfs::gui::engine
namespace nfs::gui::widgets::family {

Point *to_screen_point(Widget *widget, Point *out, const Point *source) {
    Dword x = Dword(source->x), y = Dword(source->y);
    do {
        x += Dword(widget->bounds.left);
        y += Dword(widget->bounds.top);
        const Dword flags = widget->flags;
        if (flags & 8)
            break;
        widget = widget->parent;
        if (!widget)
            break;
        if (!(flags & 64)) {
            x += Dword(widget->scroll.x);
            y += Dword(widget->scroll.y);
        }
    } while (widget);
    *out = {engine::point_methods, int(x), int(y)};
    return out;
}

Point *from_screen_point(Widget *widget, Point *out, const Point *source) {
    Dword x = Dword(source->x), y = Dword(source->y);
    do {
        x -= Dword(widget->bounds.left);
        y -= Dword(widget->bounds.top);
        const Dword flags = widget->flags;
        if (flags & 8)
            break;
        widget = widget->parent;
        if (!widget)
            break;
        if (!(flags & 64)) {
            x -= Dword(widget->scroll.x);
            y -= Dword(widget->scroll.y);
        }
    } while (widget);
    *out = {engine::point_methods, int(x), int(y)};
    return out;
}
} // namespace nfs::gui::widgets::family
