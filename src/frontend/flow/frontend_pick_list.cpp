#include "frontend_pick_list.hpp"
#include "../../input/controllers/controller_interfaces.hpp"
#include "nfs/network/packets/network_packet_handlers.hpp"
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::invoke;
using game::runtime::system;

void __fastcall initialize_pick_list(PickList *list, void *) {
    auto *widget = pick_widget(list);
    family::combo_box_initialize(widget, nullptr);
    if (auto *button = static_cast<Widget *>(pending::find_child(widget, "<BUTTON>"));
        button && invoke<Dword>(button, 2, family::button_class_key))
        invoke(button, 7, &list->label);
    family::clear_combo_rows(widget);
    PickListConfiguration configuration(&widget->name);
    for (int row = 0; row < invoke<int>(configuration.storage, 3); ++row) {
        const auto *key = invoke<const gui::String *>(configuration.storage, 4, row);
        if (!gui::string_data(key))
            continue;
        int value;
        if (!gui::engine::pending::read_int(configuration.storage, gui::string_ascii(key), &value,
                                            0) ||
            !gui::engine::pending::character_at(key, 0))
            continue;
        const Dword reference = controllers::resolve_control_reference(gui::string_ascii(key));
        if (reference) {
            pending::LocalFrontendString text;
            bind_string_reference(&text.value, reference);
            family::append_combo_item(widget, &text.value, Dword(value), 0);
        } else
            family::append_combo_item(widget, key, Dword(value), 0);
    }
    const Dword value = network::packets::rule_value(system(72), int(list->binding));
    const int row = family::find_combo_value(widget, value);
    if (row > -1) {
        const Dword before = selected_pick_value(list);
        const Dword changed = family::select_combo_row(widget, row);
        const Dword after = selected_pick_value(list);
        if (changed && after != before)
            network::packets::pending::set_rule_variant(system(72), int(list->binding),
                                                                 after);
    }
}

Dword __fastcall pick_list_event(PickList *list, void *, Dword event, Dword first, Dword second) {
    if (event == 102) {
        if (first == list->binding && second)
            filter_pick_values(list, reinterpret_cast<const ToggleFilter *>(second));
    } else if (event == 103 && first == list->binding) {
        const int row = family::find_combo_value(pick_widget(list), second);
        if (row > -1) {
            const Dword before = selected_pick_value(list);
            const Dword changed = family::select_combo_row(pick_widget(list), row);
            const Dword after = selected_pick_value(list);
            if (changed && after != before)
                network::packets::pending::set_rule_variant(system(72),
                                                                     int(list->binding), after);
        }
    }
    return family::combo_box_event(pick_widget(list), nullptr, event, first, second);
}

void __fastcall commit_pick_list(PickList *list, void *) {
    family::combo_box_item_operation_55(pick_widget(list), nullptr);
    if (auto *item = family::combo_item(pick_widget(list), list->selected_row))
        network::packets::pending::set_rule_variant(system(72), int(list->binding),
                                                             item->data);
}

Dword select_pick_text(PickList *list, const gui::String *text) {
    const Dword before = selected_pick_value(list);
    const Dword changed = family::select_combo_text(pick_widget(list), text);
    const Dword after = selected_pick_value(list);
    if (changed && after != before)
        network::packets::pending::set_rule_variant(system(72), int(list->binding),
                                                             after);
    return changed;
}

Dword select_pick_row(PickList *list, int row) {
    const Dword before = selected_pick_value(list);
    const Dword changed = family::select_combo_row(pick_widget(list), row);
    const Dword after = selected_pick_value(list);
    if (changed && after != before)
        network::packets::pending::set_rule_variant(system(72), int(list->binding),
                                                             after);
    return changed;
}

Dword select_pick_value(PickList *list, Dword value) {
    const int row = family::find_combo_value(pick_widget(list), value);
    if (row <= -1)
        return 0;
    const Dword before = selected_pick_value(list);
    const Dword changed = family::select_combo_row(pick_widget(list), row);
    const Dword after = selected_pick_value(list);
    if (changed && after != before)
        network::packets::pending::set_rule_variant(system(72), int(list->binding),
                                                             after);
    return changed;
}

void erase_pick_value(PickList *list, Dword value) {
    const int row = family::find_combo_value(pick_widget(list), value);
    if (row != -1)
        family::erase_combo_row(pick_widget(list), row);
}

Dword enable_pick_value(PickList *list, Dword value, Dword enabled) {
    const int row = family::find_combo_value(pick_widget(list), value);
    return row == -1 ? 0 : family::enable_combo_row(pick_widget(list), row, enabled);
}

void __fastcall fit_pick_list(PickList *list, void *, Dword visible) {
    auto *render = gui::active_engine();
    if (!visible || !render)
        return;
    const int count = family::combo_row_count(pick_widget(list));
    auto *rows = family::combo_list(pick_widget(list));
    if (!rows)
        return;
    rows->layout_flags &= ~Dword(512);
    auto *cell = invoke<Widget *>(rows, 47, Dword(0));
    if (!cell || !invoke<Dword>(cell, 2, family::text_class_key))
        return;
    invoke(render, 14, inherited_font_style(cell));
    int width = pick_widget(list)->bounds.width;
    for (int row = 0; row < count; ++row)
        if (auto *item = family::combo_item(pick_widget(list), row)) {
            invoke(cell, 7, &item->text);
            invoke(cell, 6, Dword(64));
            const int right = cell->bounds.left + cell->bounds.width;
            if (width <= right)
                width = right;
        }
    if (width)
        rows->bounds.width = width;
}
} // namespace nfs::application::frontend
