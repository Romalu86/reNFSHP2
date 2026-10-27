#include "gui_widget_menu.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
namespace nfs::gui::engine {
int seconds_to_ticks(float);
}
namespace nfs::gui::widgets::family {
using game::runtime::invoke;
static bool enabled_menu_command(Menu *owner, Dword index) {
    auto *item = menu_item(owner, index);
    if (!item || !item->command)
        return false;
    auto *command = menu_command(owner, item->command);
    return command && !(command->flags & 4);
}
static Widget *item_submenu(Menu *owner, Dword index) {
    auto *item = menu_item(owner, index);
    if (!item)
        return nullptr;
    Widget *child = registered_widget(item->submenu);
    return is_menu(child) ? child : nullptr;
}

Dword menu_row_at_pointer(Menu *owner) {
    Point local;
    from_screen_point(menu_widget(owner), &local, pending::pointer_position());
    Dword row = Dword(-1);
    for (int index = 0; index < int(owner->items.count); ++index)
        if (auto *item = menu_item(owner, Dword(index)))
            if (contains_point(&item->bounds, &local)) {
                row = (item->flags & 12) == 12 ? Dword(owner->selected) : Dword(index);
                break;
            }
    engine::destroy_point(&local);
    return row;
}

void update_menu_pointer(Menu *owner, Dword force) {
    Widget *widget = menu_widget(owner);
    const int previous = owner->selected;
    owner->selected = int(menu_row_at_pointer(owner));
    if (previous != owner->selected) {
        auto *item = menu_item(owner, Dword(owner->selected));
        events::post_widget(widget, 91, item ? item->command : 0, widget->id);
        owner->state |= 2;
        owner->delay_ticks = engine::seconds_to_ticks(owner->delay);
        if (owner->style & 1) {
            auto *selected = menu_item(owner, Dword(owner->selected));
            auto *command =
                selected && selected->command ? menu_command(owner, selected->command) : nullptr;
            string_copy(&widget->tooltip,
                        command ? &command->tooltip
                                : &lifecycle::construction::unresolved::shared_string);
        }
    }
    if (!force && !(owner->state & 2))
        return;
    const Dword index = Dword(owner->selected);
    if (int(index) >= 0) {
        if (owner->delay != 0 && owner->delay_ticks >= 0 && !force)
            return;
        if (item_submenu(owner, index) && enabled_menu_command(owner, index))
            open_submenu(owner, index);
        else
            activate(widget);
    }
    owner->state &= ~Dword(2);
}

Dword __fastcall menu_key_down(Widget *widget, void *, Dword key, Dword modifiers) {
    Menu *owner = menu(widget);
    const Dword state = owner->state;
    int remaining = int(owner->items.count);
    owner->state &= ~Dword(2);
    switch (key) {
    case 1:
        events::dispatch(widget, 41, 0, 0);
        return 1;
    case 28:
        if (!std::uint8_t(modifiers)) {
            const Dword index = Dword(owner->selected);
            if (enabled_menu_command(owner, index)) {
                if (item_submenu(owner, index)) {
                    open_submenu(owner, index);
                    return 1;
                }
                events::post_widget(widget, 40, 0, 0);
            }
        }
        return 1;
    case 200:
    case 208:
        if (remaining) {
            Dword index = Dword(owner->selected) + (key == 200 ? Dword(-1) : Dword(1));
            bool found = false;
            while (remaining) {
                --remaining;
                if (auto *item = menu_item(owner, index); item && (item->flags & 12) != 12) {
                    found = true;
                    break;
                }
                if (key == 200) {
                    --index;
                    if (int(index) < 0)
                        index = owner->items.count;
                } else {
                    ++index;
                    if (index >= owner->items.count)
                        index = 0;
                }
            }
            if (!found)
                index = Dword(owner->selected);
            const Dword id = widget->id;
            owner->selected = int(index);
            auto *item = menu_item(owner, index);
            events::post_widget(widget, 91, item ? item->command : 0, id);
        }
        return 1;
    case 203:
        if (!std::uint8_t(modifiers) && (state & 1)) {
            close_menu(owner);
            activate(widget);
        }
        return 1;
    case 205:
        if (!std::uint8_t(modifiers)) {
            const int index = owner->selected;
            if (index < 0) {
                owner->selected = remaining < 1 ? -1 : 0;
                return 1;
            }
            if (enabled_menu_command(owner, Dword(index)) && item_submenu(owner, Dword(index)) &&
                open_submenu(owner, Dword(index)))
                if (auto *child = item_submenu(owner, Dword(owner->selected)))
                    menu(child)->selected = int(menu(child)->items.count) < 1 ? -1 : 0;
        }
        return 1;
    default:
        return pending::general_key(reinterpret_cast<GeneralWidget *>(widget), nullptr, key,
                                    modifiers);
    }
}
// The entry discards two unused stack arguments.
Dword __fastcall menu_pointer_release(Widget *widget, void *, Dword, Dword) {
    if (invoke<Dword>(widget, 9)) {
        update_menu_pointer(menu(widget), 1);
        const int selected = menu(widget)->selected;
        if (selected >= 0 && selected < int(menu(widget)->items.count) &&
            enabled_menu_command(menu(widget), Dword(selected))) {
            if (item_submenu(menu(widget), Dword(selected)))
                return 1;
            events::post_widget(widget, 40, 0, 0);
        }
    }
    return 1;
}

Dword __fastcall menu_pointer_press(Widget *widget, void *, Dword, Dword modifiers) {
    if (invoke<Dword>(widget, 9))
        update_menu_pointer(menu(widget), std::uint8_t(modifiers) == 0);
    else if (!std::uint8_t(modifiers))
        events::dispatch(widget, 41, 0, 0);
    return 1;
}

Dword __fastcall menu_pointer_move(Widget *widget, void *, Dword, Dword) {
    if (invoke<Dword>(widget, 9))
        update_menu_pointer(menu(widget), 0);
    if (menu(widget)->style & 1)
        post_target(inherited_tooltip_owner(widget), 76, 0, 0);
    return 1;
}

Dword __fastcall menu_pointer_idle(Widget *widget, void *, Dword ticks, Dword) {
    Menu *owner = menu(widget);
    if (!invoke<Dword>(widget, 9)) {
        if (owner->style & 1)
            string_copy(&widget->tooltip,
                        &lifecycle::construction::unresolved::shared_string);
    } else if ((owner->state & 2) && owner->selected >= 0) {
        const Dword index = Dword(owner->selected);
        if (owner->delay != 0 && owner->delay_ticks >= 0) {
            owner->delay_ticks = int(Dword(owner->delay_ticks) - ticks);
            return 1;
        }
        if (item_submenu(owner, index) && enabled_menu_command(owner, index))
            open_submenu(owner, index);
        else
            activate(widget);
        owner->state &= ~Dword(2);
    }
    return 1;
}
} // namespace nfs::gui::widgets::family
