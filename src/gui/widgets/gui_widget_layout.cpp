#include "gui_widgets.hpp"
#include "nfs/core/application/application.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include <algorithm>
namespace nfs::gui::widgets {
using game::runtime::invoke;
void union_rectangle(Rectangle *, const Rectangle *);

Rectangle *client_bounds(Widget *widget, Rectangle *out) {
    const int height = widget->bounds.height, width = widget->bounds.width;
    engine::construct_rectangle(out);
    out->left = out->top = 0;
    out->width = width;
    out->height = height;
    return out;
}

Dword contains_point(const Rectangle *rectangle, const Point *point) {
    return rectangle->left <= point->x && point->x < rectangle->left + rectangle->width &&
           rectangle->top <= point->y && point->y < rectangle->top + rectangle->height;
}

Widget *child_at(Widget *widget, int index, Dword order) {
    const Dword count = widget->children.count;
    if (index < 0 || index > int(count))
        return nullptr;
    Container *container = &widget->children;
    Dword position = Dword(index);
    if (order == 1)
        container = &widget->draw_order;
    else if (order == 2)
        container = &widget->tab_order;
    else if (order == 3) {
        container = &widget->tab_order;
        position = count - Dword(index) - 1;
    }
    return static_cast<Widget *>(events::container_value(container, position));
}

Widget *find_child(Widget *widget, Dword id) {
    if (!id)
        return nullptr;
    const Dword count = widget->children.count;
    if (count > 3)
        return static_cast<Widget *>(
            container_find(&widget->children, compare_id, reinterpret_cast<void *>(id)));
    for (Dword index = 0; index < count; ++index)
        if (auto *child = static_cast<Widget *>(events::container_value(&widget->children, index)))
            if (child->id == id)
                return child;
    return nullptr;
}
namespace pending {

void __fastcall layout(Widget *widget, void *, Dword flags) {
    Rectangle &bounds = widget->bounds;
    if (flags & 256) {
        bounds.left = engine::source_bounds.left;
        bounds.top = engine::source_bounds.top;
        bounds.width = engine::source_bounds.width;
        bounds.height = engine::source_bounds.height;
        return;
    }
    if (flags & 128) {
        if (widget->parent) {
            bounds.left = bounds.top = 0;
            bounds.width = widget->parent->bounds.width;
            bounds.height = widget->parent->bounds.height;
        }
        return;
    }
    Rectangle reference;
    engine::construct_rectangle(&reference);
    if (!widget->parent || (widget->flags & 8)) {
        reference.left = engine::source_bounds.left;
        reference.top = engine::source_bounds.top;
        reference.width = engine::source_bounds.width;
        reference.height = engine::source_bounds.height;
    } else {
        Rectangle client;
        client_bounds(widget->parent, &client);
        reference.left = client.left;
        reference.top = client.top;
        reference.width = client.width;
        reference.height = client.height;
        engine::destroy_rectangle(&client);
    }
    if (flags & 512)
        bounds.width = reference.width;
    if (flags & 1024)
        bounds.height = reference.height;
    if ((flags & 64) && widget->children.count) {
        Rectangle content;
        engine::construct_rectangle(&content);
        int count = 0;
        for (int index = 0; index < int(widget->children.count); ++index)
            if (Widget *child = child_at(widget, index, 0))
                if (!(child->layout_flags & 32896) && !(child->flags & 32808) &&
                    (!(child->flags & 2) || (child->layout_flags & 65536))) {
                    if (!count) {
                        content.left = child->bounds.left;
                        content.top = child->bounds.top;
                        content.width = child->bounds.width;
                        content.height = child->bounds.height;
                    } else
                        union_rectangle(&content, &child->bounds);
                    ++count;
                }
        if (count) {
            bounds.left += content.left;
            bounds.top += content.top;
            bounds.width = content.width;
            bounds.height = content.height;
            const int x = -content.left, y = -content.top;
            if (x || y)
                for (int index = 0; index < int(widget->children.count); ++index)
                    if (Widget *child = child_at(widget, index, 0))
                        if (!(child->layout_flags & 128)) {
                            child->bounds.left += x;
                            child->bounds.top += y;
                        }
        }
        engine::destroy_rectangle(&content);
    }
    if (flags & 2)
        bounds.left = reference.left;
    else if (flags & 1)
        bounds.left = reference.left - bounds.width + reference.width;
    if (flags & 4)
        bounds.top = reference.top;
    else if (flags & 8)
        bounds.top = reference.top - bounds.height + reference.height;
    if (flags & 4096)
        bounds.left = reference.left - bounds.width;
    else if (flags & 2048)
        bounds.left = reference.left + reference.width;
    if (flags & 8192)
        bounds.top = reference.top - bounds.height;
    else if (flags & 16384)
        bounds.top = reference.top + reference.height;
    if (flags & 32)
        bounds.left = (reference.width - bounds.width) / 2 + reference.left;
    if (flags & 16)
        bounds.top = (reference.height - bounds.height) / 2 + reference.top;
    if (flags & 131072) {
        bounds.left =
            std::min(bounds.left + bounds.width, reference.left + reference.width) - bounds.width;
        bounds.top =
            std::min(bounds.top + bounds.height, reference.top + reference.height) - bounds.height;
        bounds.left = std::max(bounds.left, reference.left);
        bounds.top = std::max(bounds.top, reference.top);
    }
    engine::destroy_rectangle(&reference);
}

Dword __fastcall hit_test(Widget *widget, void *) {
    return widget->flags & 2 ? 0 : contains_point(&widget->clip, pointer_position());
}
} // namespace pending

Dword contains_modal(Widget *widget) {
    if (widget->options & 1)
        return 1;
    const int count = int(widget->children.count);
    for (int index = 0; index < count; ++index)
        if (Widget *child = child_at(widget, index, 0))
            if (contains_modal(child))
                return 1;
    return 0;
}

Dword hit_branch(Widget *widget) {
    const Dword flags = widget->flags;
    if (flags & 3)
        return 0;
    Point *point = pending::pointer_position();
    if (!(flags & 128) && !contains_point(&widget->padding, point))
        return 0;
    for (int index = 0; index < int(widget->children.count); ++index)
        if (auto *child =
                static_cast<Widget *>(events::container_value(&widget->draw_order, Dword(index))))
            if (!(child->flags & 3) &&
                ((child->flags & 128) || contains_point(&child->padding, point)))
                if (Dword result = hit_branch(child))
                    return result;
    if (!(widget->flags & 128) && !invoke<Dword>(widget, 9))
        return 0;
    return widget->id;
}

Dword next_pointer_target(Widget *widget) {
    if ((widget->flags & 128) || application::pending::captured_widget_id == widget->id)
        return 0;
    Widget *current = widget;
    while (current->parent) {
        Widget *parent = current->parent;
        Dword position = Dword(-1), count = parent->draw_order.count;
        for (Dword index = 0; index < count; ++index)
            if (events::container_value(&parent->draw_order, index) == current) {
                position = index;
                break;
            }
        Widget *last = current;
        while (++position < parent->draw_order.count) {
            auto *child =
                static_cast<Widget *>(events::container_value(&parent->draw_order, position));
            if (!child)
                break;
            last = child;
            if (!(child->flags & 3))
                if (Dword hit = hit_branch(child))
                    return hit;
        }
        if (last->flags & 128)
            return 0;
        if ((parent->flags & 128) || (!(parent->flags & 1) && invoke<Dword>(parent, 9)))
            return parent->id;
        current = parent;
    }
    return 0;
}
} // namespace nfs::gui::widgets
