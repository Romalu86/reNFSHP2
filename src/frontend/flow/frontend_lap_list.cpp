#include "frontend_pick_list.hpp"
#include "../screens/frontend_dialogs.hpp"
#include "nfs/network/packets/network_packet_handlers.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include <cstdio>
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::invoke;
using game::runtime::system;
namespace {
bool preset_laps(int value) {
    return value == 2 || value == 4 || value == 8;
}
} // namespace

Widget *probe_widget_type(TypeName type, Widget *widget) {
    return widget && invoke<Dword>(widget, 2, type) ? widget : nullptr;
}

void __fastcall initialize_lap_list(LapList *list, void *) {
    initialize_pick_list(&list->base, nullptr);
    add_custom_lap(list, int(network::packets::rule_value(system(72), int(list->base.binding))));
}

Dword __fastcall lap_list_event(LapList *list, void *, Dword event, Dword first, Dword second) {
    if (event == 102) {
        if (first == list->base.binding && second) {
            filter_lap_values(list, reinterpret_cast<const ToggleFilter *>(second));
            return family::combo_box_event(lap_widget(list), nullptr, 102, first, second);
        }
    } else if (event == 103) {
        if (first == list->base.binding) {
            const Dword result = pick_list_event(&list->base, nullptr, 103, first, second);
            remove_custom_laps(list);
            add_custom_lap(
                list, int(network::packets::rule_value(system(72), int(list->base.binding))));
            return result;
        }
    } else
        return pick_list_event(&list->base, nullptr, event, first, second);
    return first;
}

void __fastcall commit_lap_list(LapList *list, void *) {
    if (auto *item = family::combo_item(lap_widget(list), list->base.selected_row)) {
        int value = int(item->data);
        if (!preset_laps(value)) {
            pending::LocalFrontendString entered;
            {
                char number[8];
                std::sprintf(
                    number, "%d",
                    int(network::packets::rule_value(system(72), int(list->base.binding))));
                pending::LocalFrontendString initial(number);
                pending::LocalFrontendString name(
                    &lifecycle::construction::unresolved::shared_string);
                pending::LocalFrontendString title("laps");
                pending::LocalFrontendString text("Please enter the number of laps");
                auto *parent =
                    probe_widget_type(interface_class_name, pending::widget_root(lap_widget(list)));
                pending::prompt_text_entry(parent, text.release(), title.release(), &entered.value,
                                           &initial.value, 3, name.release(), 0, nullptr, 0, 0);
            }
            value = gui::engine::pending::string_integer(&entered.value);
            if (preset_laps(value) && value != -1) {
                int row = family::find_combo_value(lap_widget(list), Dword(value));
                if (row <= -1) {
                    remove_custom_laps(list);
                    add_custom_lap(list, value);
                    row = family::find_combo_value(lap_widget(list), Dword(value));
                }
                if (row > -1)
                    select_pick_row(&list->base, row);
            }
        }
        if (value >= 100)
            value = 100;
        network::packets::pending::set_rule_variant(system(72), int(list->base.binding),
                                                             Dword(value));
        remove_custom_laps(list);
        add_custom_lap(list,
                       int(network::packets::rule_value(system(72), int(list->base.binding))));
    }
    commit_pick_list(&list->base, nullptr);
}

Dword select_lap_value(LapList *list, int value) {
    if (value != -1) {
        int row = family::find_combo_value(lap_widget(list), Dword(value));
        if (row > -1)
            return select_pick_row(&list->base, row);
        remove_custom_laps(list);
        add_custom_lap(list, value);
        row = family::find_combo_value(lap_widget(list), Dword(value));
        if (row > -1)
            return select_pick_row(&list->base, row);
    }
    return 0;
}

void add_custom_lap(LapList *list, int value) {
    if (preset_laps(value) || value == 666) {
        pending::LocalFrontendString text("Custom");  
        family::append_combo_item(lap_widget(list), &text.value, 666, 0);
    } else if (family::find_combo_value(lap_widget(list), Dword(value)) == -1) {
        char text[128];
        std::sprintf(text, "%s%d", "Custom: ", value);
        gui::string_assign(&list->custom_label, text);
        family::append_combo_item(lap_widget(list), &list->custom_label, Dword(value), 0);
        if (value != -1) {
            int row = family::find_combo_value(lap_widget(list), Dword(value));
            if (row > -1) {
                select_pick_row(&list->base, row);
                return;
            }
            remove_custom_laps(list);
            add_custom_lap(list, value);
            row = family::find_combo_value(lap_widget(list), Dword(value));
            if (row > -1)
                select_pick_row(&list->base, row);
        }
    }
}

void remove_custom_laps(LapList *list) {
    for (int row = 0; row < family::combo_row_count(lap_widget(list)); ++row)
        if (auto *item = family::combo_item(lap_widget(list), row);
            item && !preset_laps(int(item->data)))
            family::erase_combo_row(lap_widget(list), row);
}

void filter_lap_values(LapList *list, const ToggleFilter *filter) {
    const Dword previous = selected_pick_value(&list->base);
    family::clear_combo_rows(lap_widget(list));
    if (filter->count)
        populate_filtered_pick_list(&list->base, filter);
    const int count = filter->count;
    bool custom = false;
    if (count > 0) {
        const Dword *values = filter->values;
        for (int entry = 0; entry < count; ++entry)
            if (!preset_laps(int(values[entry]))) {
                add_custom_lap(list, int(values[entry]));
                custom = true;
            }
    }
    if (count) {
        if (count != 1) {
            set_enabled(lap_widget(list), 1);
            if (previous == Dword(-1))
                select_pick_row(&list->base, 0);
            else if (!select_lap_value(list, int(previous)))
                select_pick_row(&list->base, 0);
        } else {
            select_pick_row(&list->base, 0);
            set_enabled(lap_widget(list), 0);
        }
    } else
        set_enabled(lap_widget(list), 0);
    if (!custom)
        add_custom_lap(list,
                       int(network::packets::rule_value(system(72), int(list->base.binding))));
}
} // namespace nfs::application::frontend
