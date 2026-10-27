#include "frontend_toggle_widget.hpp"
#include "nfs/network/packets/network_packet_handlers.hpp"
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::invoke;
using game::runtime::system;
using network::packets::rule_value;
static family::ListItem *selected_toggle_item(ProfileToggle *widget) {
    auto *items = &widget->items;
    const auto *methods = items->methods;
    const int row = family::selected_list_row(items);
    using Cell = family::ListItem *(__thiscall *)(void *, Dword, Dword);
    return reinterpret_cast<Cell>(methods[9])(items, Dword(row), Dword(0));
}

int find_toggle_value(ProfileToggle *widget, Dword value) {
    for (int row = 0; row < int(widget->items.grid.rows.count); ++row) {
        auto *item = invoke<family::ListItem *>(&widget->items, 9, Dword(row), Dword(0));
        if (item && item->data == value)
            return row;
    }
    return -1;
}
static void select_and_notify(ProfileToggle *widget, int row) {
    profiles::select_profile_row(widget, Dword(row));
}
static void store_toggle_value(ProfileToggle *widget) {
    auto *item = selected_toggle_item(widget);
    network::packets::pending::set_rule_variant(system(72), int(widget->binding),
                                                         item ? item->data : ~Dword(0));
}

void __fastcall initialize_profile_toggle(ProfileToggle *widget, void *) {
    family::toggle_button_initialize(&widget->button.base.base, nullptr);
    if (!widget->binding)
        return;
    if (widget->populate_from_list)
        populate_toggle_list(widget);
    if (widget->binding) {
        const int row = find_toggle_value(widget, rule_value(system(72), int(widget->binding)));
        if (row >= 0) {
            select_and_notify(widget, row);
            return;
        }
    }
    const int row = find_toggle_value(widget, widget->default_value);
    if (row >= 0) {
        select_and_notify(widget, row);
        return;
    }
    select_and_notify(widget, 0);
    store_toggle_value(widget);
}

Dword __fastcall profile_toggle_key(ProfileToggle *widget, void *, Dword key, Dword modifiers) {
    if (key == 1 || key == 28)
        return 0;
    if (key != 203 && key != 205)
        return family::toggle_button_key_down(&widget->button.base.base, nullptr, key, modifiers);
    if (!widget->items.grid.rows.count)
        return 1;
    const int selected = family::selected_list_row(&widget->items);
    int row;
    if (key == 203) {
        row = family::previous_list_row(&widget->items, selected);
        if (row < 0)
            row = family::previous_list_row(&widget->items, int(widget->items.grid.rows.count));
    } else {
        row = family::next_list_row(&widget->items, selected);
        if (row < 0)
            row = family::next_list_row(&widget->items, -1);
    }
    invoke<Dword>(&widget->items, 10, Dword(row), Dword(1));
    gui::events::dispatch(widget, 70, 0, 0);
    return 1;
}

Dword __fastcall profile_toggle_event(ProfileToggle *widget, void *, Dword event, Dword first,
                                     Dword second) {
    switch (event) {
    case 11:
        if (first == 1 && std::uint8_t(second) == 0) {
            int row = family::previous_list_row(&widget->items,
                                                family::selected_list_row(&widget->items));
            if (row < 0)
                row = family::previous_list_row(&widget->items, int(widget->items.grid.rows.count));
            select_and_notify(widget, row);
            gui::events::dispatch(widget, 70, 0, 0);
        }
        break;
    case 68:
    case 70:
        if (widget->binding)
            if (auto *item = selected_toggle_item(widget))
                network::packets::pending::set_rule_variant(
                    system(72), int(widget->binding), item->data);
        break;
    case 102:
        if (first == widget->binding && second)
            filter_toggle_values(widget, reinterpret_cast<const ToggleFilter *>(second));
        break;
    case 103:
        if (first == widget->binding) {
            const int row = find_toggle_value(widget, second);
            if (row >= 0)
                select_and_notify(widget, row);
        }
        break;
    case 105:
        if (first == widget->binding && second)
            replace_toggle_list(widget, reinterpret_cast<const char *>(second));
        break;
    case 109:
        if (first == widget->binding) {
            set_enabled(&widget->button.base.base, second != 0);
            if (widget->binding) {
                const int row =
                    find_toggle_value(widget, rule_value(system(72), int(widget->binding)));
                if (row >= 0) {
                    select_and_notify(widget, row);
                    break;
                }
            }
            const int row = find_toggle_value(widget, widget->default_value);
            if (row >= 0) {
                select_and_notify(widget, row);
                break;
            }
            select_and_notify(widget, 0);
            store_toggle_value(widget);
        }
        break;
    }
    return family::button_event(&widget->button.base.base, nullptr, event, first, second);
}
} // namespace nfs::application::frontend
