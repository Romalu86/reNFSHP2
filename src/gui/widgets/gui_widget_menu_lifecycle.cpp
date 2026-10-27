#include "gui_widget_menu.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
namespace nfs::gui::widgets::family {
using game::runtime::invoke;

static void request_menu_commands(Widget *receiver, Menu *owner) {
    const Dword initial_count = owner->items.count;
    for (Dword index = 0; index < owner->items.count; ++index)
        if (auto *element = index < initial_count ? owner->items.elements[index] : nullptr)
            if (auto *item = static_cast<MenuItem *>(element->value))
                events::dispatch(receiver, 92, item->command, menu_widget(owner)->id);
    for (Dword index = 0; index < owner->items.count; ++index)
        if (auto *item = menu_item(owner, index))
            if (Widget *child = registered_widget(item->submenu); is_menu(child))
                request_menu_commands(receiver, menu(child));
}

void show_menu(Menu *owner) {
    if (owner->state & 4)
        return;
    Widget *widget = menu_widget(owner);
    if (owner->state & 1) {
        Widget *root = widget;
        while (is_menu(root->parent))
            root = root->parent;
        const String *resource = root->resource
                                     ? &root->resource->string
                                     : &lifecycle::construction::unresolved::shared_string;
        if (resource->length) {
            String name;
            string_construct(&name);
            try {
                string_assign(&name, "__Sub__");
                string_copy(&widget->name, &name);
            } catch (...) {
                string_destruct(&name);
                throw;
            }
            string_destruct(&name);
            reload_widget_children(widget, string_ascii(resource));
            set_widget_resource(widget,
                                &lifecycle::construction::unresolved::shared_string);
            string_copy(&widget->name,
                        &lifecycle::construction::unresolved::shared_string);
        }
    }
    widget->flags &= ~Dword(4);
    move_to_first_draw_position(widget);
    set_enabled(widget, 1);
    application::pending::set_widget_visible(widget, 1);

    if (context_state)
        for (Dword event : {11u, 10u, 12u, 9u, 7u, 6u, 8u, 3u, 4u, 2u, 5u})
            events::remove_event(context_state, event);
    activate(widget);
    owner->state |= 4;
    request_menu_commands(widget, owner);
}

void close_menu(Menu *owner) {
    Widget *widget = menu_widget(owner);
    for (int index = 0; index < int(owner->items.count); ++index)
        if (auto *item = menu_item(owner, Dword(index)))
            if (auto *child = registered_widget(item->submenu); is_menu(child))
                close_menu(menu(child));
    if (!(owner->state & 4))
        return;
    if (owner->state & 1) {
        List detached;
        events::construct_list(&detached, 256);
        try {
            for (int index = 0; index < int(widget->children.count); ++index)
                if (Widget *child = child_at(widget, index, 0))
                    if (!is_menu(child) || !(menu(child)->state & 1))
                        events::push_front(&detached, child, nullptr);
            while (auto *child = static_cast<Widget *>(events::pop_front(&detached)))
                remove_child_indices(widget, child);
        } catch (...) {
            list_clear(&detached);
            throw;
        }
        list_clear(&detached);
    }
    set_enabled(widget, 0);
    application::pending::set_widget_visible(widget, 0);
    owner->state &= ~Dword(4);
}
// The point is passed by value, followed by the anchor mode.
static Dword position_menu(Menu *owner, Point position, Dword anchor) {
    Rectangle bounds;
    client_bounds(menu_widget(owner), &bounds);
    const Rectangle screen = engine::source_bounds;
    switch (anchor) {
    case 0:
    case 2:
        bounds.left = position.x;
        bounds.top = position.y;
        break;
    case 1:
        bounds.left = position.x;
        bounds.top = position.y - bounds.height;
        break;
    case 3:
        bounds.left = position.x - bounds.width;
        bounds.top = position.y - bounds.height;
        break;
    case 4:
        bounds.left = position.x - bounds.width;
        bounds.top = position.y;
        break;
    }
    if (!anchor) {
        if (screen.left + screen.width < bounds.left + bounds.width)
            bounds.left = screen.left + screen.width - bounds.width;
        if (bounds.top < screen.top)
            bounds.top = screen.top;
        if (bounds.left < screen.left)
            bounds.left = screen.left;
        if (screen.top + screen.height < bounds.top + bounds.height)
            bounds.top = screen.top + screen.height - bounds.height;
    } else if (screen.left + screen.width < bounds.left + bounds.width || bounds.top < screen.top ||
               bounds.left < screen.left ||
               screen.top + screen.height < bounds.top + bounds.height) {
        engine::destroy_rectangle(&bounds);
        return 0;
    }
    menu_widget(owner)->bounds.left = bounds.left;
    menu_widget(owner)->bounds.top = bounds.top;
    engine::destroy_rectangle(&bounds);
    return 1;
}

Dword open_submenu(Menu *owner, Dword index) {
    auto *item = menu_item(owner, index);
    if (!item)
        return 0;
    Widget *child = registered_widget(item->submenu);
    if (!is_menu(child) || !(child->flags & 2) || index >= owner->items.count)
        return 0;
    item = menu_item(owner, index);
    if (!item || (item->flags & 4))
        return 0;
    for (int other = 0; other < int(owner->items.count); ++other)
        if (Dword(other) != index)
            if (auto *entry = menu_item(owner, Dword(other)))
                if (Widget *submenu = registered_widget(entry->submenu); is_menu(submenu))
                    close_menu(menu(submenu));
    measure_menu(menu(child));
    Rectangle row = item->bounds;
    row.methods = engine::rectangle_methods;
    Point right{engine::point_methods, row.left + row.width, row.top};
    Point left{engine::point_methods, row.left, row.top};
    to_screen_point(menu_widget(owner), &right, &right);
    to_screen_point(menu_widget(owner), &left, &left);
    right.x -= menu(child)->overlap;
    left.x += menu(child)->overlap;
    if (!position_menu(menu(child), right, 2)) {
        Point below_right{engine::point_methods, right.x, right.y + row.height};
        if (!position_menu(menu(child), below_right, 1) && !position_menu(menu(child), left, 4)) {
            Point below_left{engine::point_methods, left.x, left.y + row.height};
            if (!position_menu(menu(child), below_left, 3))
                position_menu(menu(child), right, 0);
            engine::destroy_point(&below_left);
        }
        engine::destroy_point(&below_right);
    }
    menu(child)->selected = -1;
    show_menu(menu(child));
    engine::destroy_point(&left);
    engine::destroy_point(&right);
    engine::destroy_rectangle(&row);
    return 1;
}

void __fastcall menu_initialize(Widget *widget, void *) {
    pending::request_general_layout(reinterpret_cast<GeneralWidget *>(widget), nullptr);
}

Dword __fastcall menu_enter_modal(Widget *widget, void *) {
    if (!pending::enter_modal(widget, nullptr))
        return 0;
    menu(widget)->selected = -1;
    measure_menu(menu(widget));
    return 1;
}

void __fastcall menu_parent_enter(Widget *widget, void *, Dword parent_address) {
    if ((menu(widget)->state & 1) && !(widget->flags & 2)) {
        bool inside = false;
        if (parent_address) {
            auto *parent = reinterpret_cast<Widget *>(parent_address);
            inside = descendant(widget, parent->id) != nullptr;
        }
        if (!inside)
            close_menu(menu(widget));
    }
    notify_parent_enter(widget, nullptr, parent_address);
}

void __fastcall menu_accept(Widget *widget, void *) {
    if (contains_modal(widget))
        events::post_widget(widget, 24, 2, widget->id);
    const Dword submenu = menu(widget)->state & 1;
    if (submenu && is_menu(widget->parent))
        events::dispatch(widget->parent, 40, 0, 0);
    if (submenu)
        close_menu(menu(widget));
}

void __fastcall menu_cancel(Widget *widget, void *) {
    menu(widget)->selected = -1;
    if (contains_modal(widget))
        events::post_widget(widget, 24, 3, widget->id);
    if (is_menu(widget->parent))
        events::dispatch(widget->parent, 41, 0, 0);
    if (menu(widget)->state & 1)
        close_menu(menu(widget));
}

Dword __fastcall menu_event(Widget *widget, void *, Dword event, Dword first, Dword second) {
    if (event == 91) {
        if (first && second)
            if (Widget *source = registered_widget(second); is_menu(source)) {
                invoke(widget, 47, first, source);
                if (is_menu(widget->parent))
                    events::dispatch(widget->parent, 91, first, second);
            }
    } else if (event != 92)
        return pending::general_event(reinterpret_cast<GeneralWidget *>(widget), nullptr, event,
                                      first, second);
    return 1;
}

void __fastcall synchronize_menu_ancestors(Widget *widget, void *, Dword, Widget *source) {
    if (source != widget || !(menu(widget)->state & 1))
        return;
    Widget *child = widget;
    for (Widget *parent = child->parent; is_menu(parent); child = parent, parent = parent->parent) {
        Dword selected = Dword(-1);
        const Dword count = menu(parent)->items.count;
        if (child)
            for (Dword index = 0; index < count; ++index)
                if (auto *item = menu_item(menu(parent), index);
                    item && item->submenu == child->id) {
                    selected = index;
                    break;
                }
        if (selected != Dword(menu(parent)->selected)) {
            if (int(menu(parent)->items.count) <= int(selected))
                selected = Dword(-1);
            menu(parent)->selected = int(selected);
        }
    }
}

void __fastcall menu_prepare_draw(Widget *widget, void *, Dword render_address) {
    if (!(widget->flags & 2))
        measure_menu(menu(widget));
    prepare_draw(widget, nullptr, reinterpret_cast<engine::Engine *>(render_address));
}
} // namespace nfs::gui::widgets::family
