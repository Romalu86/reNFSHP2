#include "gui_widget_controls.hpp"
#include "../lifecycle/gui_configuration_cache.hpp"
#include "nfs/gui/resources/gui_format.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
namespace nfs::gui::engine::pending {
ini::Document *acquire_configuration_document(const String *);
void release_configuration_document(const String *);
} // namespace nfs::gui::engine::pending
namespace nfs::gui::widgets::family {

Widget *sibling_by_id(Widget *widget, Dword id) {
    if (!widget->parent || !id)
        return nullptr;
    Widget *sibling = find_child(widget->parent, id);
    return sibling != widget ? sibling : nullptr;
}

Widget *named_sibling(Widget *widget, const String *name) {
    Widget *parent = widget->parent;
    if (!parent || !name->length)
        return nullptr;
    const int count = int(parent->children.count);
    for (int index = 0; index < count; ++index)
        if (index >= 0 && index <= int(parent->children.count))
            if (auto *child =
                    static_cast<Widget *>(events::container_value(&parent->children, Dword(index))))
                if (child != widget && string_equal(&child->name, name))
                    return child;
    return nullptr;
}

void set_widget_resource(Widget *widget, const String *name) {
    auto *previous_resource = widget->resource;
    const String *previous = previous_resource
                                 ? &previous_resource->string
                                 : &lifecycle::construction::unresolved::shared_string;
    if (string_equal(previous, name))
        return;
    if (widget->options & 16) {
        engine::pending::release_configuration_document(previous);
        widget->options &= ~Dword(16);
    }
    string_release(previous_resource);
    auto *resource = string_acquire(string_ascii(name));
    widget->resource = resource;
    if (container_mode) {
        engine::pending::acquire_configuration_document(
            resource ? &resource->string
                             : &lifecycle::construction::unresolved::shared_string);
        widget->options |= 16;
    }
}

void set_z_order(Widget *widget, Dword order) {
    widget->z_order = int(order);
    if (widget->parent) {
        Container *ordered = &widget->parent->draw_order;
        container_remove(ordered, widget);
        container_sorted_insert(ordered, compare_order, reinterpret_cast<void *>(widget->z_order),
                                widget, nullptr);
    }
}

Dword destroy_widget_tree(Widget *widget) {
    events::propagate(widget, 20, 0, 0, 1);
    return 0;
}

int selected_list_row(ListItems *items) {
    const int count = int(items->grid.rows.count);
    for (int row = 0; row < count; ++row)
        if (auto *item = game::runtime::invoke<ListItem *>(items, 9, row, Dword(0)))
            if (item->flags & 1)
                return row;
    return -1;
}

int next_list_row(ListItems *items, int row) {
    const int count = int(items->grid.rows.count);
    row = int(Dword(row) + 1);
    while (row < count) {
        if (auto *item = game::runtime::invoke<ListItem *>(items, 9, row, Dword(0)))
            if (!(item->flags & 2))
                return row;
        row = int(Dword(row) + 1);
    }
    return -1;
}

int previous_list_row(ListItems *items, int row) {
    row = int(Dword(row) - 1);
    while (row >= 0) {
        if (auto *item = game::runtime::invoke<ListItem *>(items, 9, row, Dword(0)))
            if (!(item->flags & 2))
                return row;
        row = int(Dword(row) - 1);
    }
    return -1;
}
} // namespace nfs::gui::widgets::family
