#include "frontend_scene_widgets.hpp"
#include "../../gui/widgets/gui_widget_list.hpp"
#include "nfs/network/packets/network_packet_handlers.hpp"
namespace nfs::gui::widgets::family {

Dword set_list_row_disabled(ListItems *items, Dword row, Dword disabled) {
    auto *item = game::runtime::invoke<ListItem *>(items, 9, row, Dword(0));
    if (!item)
        return 0;
    item->flags = disabled ? item->flags | 2 : item->flags & ~Dword(2);
    return 1;
}
} // namespace nfs::gui::widgets::family
namespace nfs::application::frontend {
using game::runtime::invoke;
namespace family = nfs::gui::widgets::family;
using family::ListItem;

int selected_type_spin_row(TypeSpinControl *widget) {
    return family::selected_list_row(&widget->items);
}

Dword type_spin_count(TypeSpinControl *widget) {
    return widget->items.grid.rows.count;
}

void enable_type_spin_value(TypeSpinControl *widget, int value, std::uint8_t enabled) {
    const Dword old = type_spin_value(widget, family::selected_list_row(&widget->items));
    for (int row = 0; row < int(widget->items.grid.rows.count); ++row) {
        auto *item = invoke<ListItem *>(&widget->items, 9, Dword(row), Dword(0));
        if ((item ? int(item->data) : 0) == value)
            family::set_list_row_disabled(&widget->items, Dword(row), enabled == 0);
    }
    if (int(old) == value && !enabled) {
        int row = family::next_list_row(&widget->items, family::selected_list_row(&widget->items));
        if (row < 0)
            row = 0;
        invoke<void>(&widget->items, 10, Dword(row), Dword(1));
        if (widget->display) {
            auto *item = invoke<ListItem *>(
                &widget->items, 9, Dword(family::selected_list_row(&widget->items)), Dword(0));
            invoke<void>(widget->display, 7,
                         item ? &item->text
                              : &lifecycle::construction::unresolved::shared_string);
        }
        const Dword selected = type_spin_value(widget, family::selected_list_row(&widget->items));
        network::packets::pending::set_rule_variant(game::runtime::system(72),
                                                             int(widget->binding), selected);
    }
}

void clear_menu_input_queue() {
    if (gui::context_state)
        for (Dword event : {11u, 10u, 12u, 9u, 7u, 6u, 8u, 3u, 4u, 2u, 5u})
            gui::events::remove_event(gui::context_state, event);
}

void restore_menu_rule_bindings(void *rules) {
    for (int rule = 1; rule < 63; ++rule)
        network::packets::pending::set_rule_variant(
            static_cast<Dword *>(rules), rule,
            *game::runtime::at<Dword *>(rules, 102672 + Dword(rule - 1) * 4));
}

void set_menu_image_enabled(Dword id, Dword enabled) {
    if (id)
        if (auto *item = family::find_image_item(id))
            item->enabled = enabled;
}
} // namespace nfs::application::frontend
