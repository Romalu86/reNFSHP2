#include "gui_widget_menu.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
namespace nfs::gui::widgets::family {
using game::runtime::invoke;

static int compare_menu_command(const void *item, const void *key) {
    const Dword command = static_cast<const MenuItem *>(item)->command;
    const Dword wanted = reinterpret_cast<Dword>(key);
    return command < wanted ? -1 : wanted < command;
}
MenuItem *menu_command(Menu *owner, Dword command) {
    return static_cast<MenuItem *>(
        container_find(&owner->items, compare_menu_command, reinterpret_cast<void *>(command)));
}

void remove_child_indices(Widget *parent, Widget *child) {
    container_remove(&parent->draw_order, child);
    container_remove(&parent->tab_order, child);
    container_remove(&parent->children, child);
}

Dword reload_widget_children(Widget *widget, const char *resource) {
    if (!resource)
        return 4;
    String name;
    string_construct(&name);
    try {
        string_assign(&name, resource);
        set_widget_resource(widget, &name);
    } catch (...) {
        string_destruct(&name);
        throw;
    }
    string_destruct(&name);
    const String *current = widget->resource
                                ? &widget->resource->string
                                : &lifecycle::construction::unresolved::shared_string;
    if (current->length) {
        application::pending::load_widget_children(widget, 1);
        const int count = int(widget->children.count);
        for (int index = 0; index < count; ++index)
            if (index >= 0 && index <= int(widget->children.count))
                if (auto *child = static_cast<Widget *>(
                        events::container_value(&widget->children, Dword(index))))
                    events::propagate(child, 18, 0, 0, 0);
    }
    return 0;
}

void measure_menu(Menu *owner) {
    Point size{engine::point_methods, 0, 0};
    auto *render = active_engine();
    if (render) {
        const int count = int(owner->items.count);
        int height = 0, width = 0;
        for (int index = 0; index < count; ++index) {
            if (auto *item = menu_item(owner, Dword(index))) {
                Rectangle measured{engine::rectangle_methods, 0, 0, 0, 0};
                const auto font = invoke<Dword>(menu_widget(owner), 51, render, Dword(index));
                invoke<Dword>(render, 14, font);
                invoke(render, 15, &item->text, &measured);
                const int row_height = (item->flags & 12) == 12
                                           ? int(invoke<Dword>(render, 49, Dword(1))) * 4
                                           : int(invoke<Dword>(render, 17)) + owner->line_spacing;
                measured.width += owner->right_margin + owner->left_margin;
                width = measured.width <= size.x ? size.x : measured.width;
                if (size.x < width)
                    size.x = width;
                item->bounds.left = measured.left;
                item->bounds.top = height;
                item->bounds.height = row_height;
                item->bounds.width = width;
                height += row_height;
                engine::destroy_rectangle(&measured);
            }
            width = size.x;
        }
        if (owner->minimum_width) {
            size.x = owner->minimum_width;
            if (owner->minimum_width < width)
                size.x = width;
        }
        for (int index = 0; index < count; ++index)
            if (auto *item = menu_item(owner, Dword(index)))
                item->bounds.width = size.x;
        menu_widget(owner)->bounds.width = size.x;
        menu_widget(owner)->bounds.height = height;
    }
    engine::destroy_point(&size);
}
} // namespace nfs::gui::widgets::family
