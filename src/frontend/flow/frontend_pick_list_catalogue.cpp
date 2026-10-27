#include "frontend_pick_list.hpp"
#include "nfs/network/packets/network_packet_handlers.hpp"
namespace nfs::application::frontend {
using game::runtime::invoke;
using game::runtime::system;
namespace family = gui::widgets::family;
void populate_filtered_pick_list(PickList *list, const ToggleFilter *filter) {
    PickListConfiguration configuration(&pick_widget(list)->name);
    for (int row = 0; row < invoke<int>(configuration.storage, 3); ++row) {
        pending::LocalFrontendString key(
            invoke<const gui::String *>(configuration.storage, 4, row));
        if (!gui::string_data(&key.value))
            continue;
        pending::LocalFrontendString text;
        if (!gui::engine::pending::assign_symbolic_string(&text.value,
                                                          gui::string_ascii(&key.value)))
            gui::string_copy(&text.value, &key.value);
        int value;
        if (gui::engine::pending::read_int(configuration.storage, gui::string_ascii(&text.value),
                                           &value, 0) &&
            text.value.length) {
            const int count = filter->count;
            if (count > 0) {
                const Dword *values = filter->values;
                for (int candidate = 0; candidate < count; ++candidate)
                    if (values[candidate] == Dword(value))
                        family::append_combo_item(pick_widget(list), &text.value, Dword(value), 0);
            }
        }
    }
}

Dword add_pick_value(PickList *list, Dword value) {
    if (family::find_combo_value(pick_widget(list), value) > -1)
        return 0;
    PickListConfiguration configuration(&pick_widget(list)->name);
    for (int row = 0; row < invoke<int>(configuration.storage, 3); ++row) {
        pending::LocalFrontendString key(
            invoke<const gui::String *>(configuration.storage, 4, row));
        if (!gui::string_data(&key.value))
            continue;
        pending::LocalFrontendString text;
        if (!gui::engine::pending::assign_symbolic_string(&text.value,
                                                          gui::string_ascii(&key.value)))
            gui::string_copy(&text.value, &key.value);
        int entry;
        if (gui::engine::pending::read_int(configuration.storage, gui::string_ascii(&text.value),
                                           &entry, 0) &&
            text.value.length && Dword(entry) == value) {
            family::append_combo_item(pick_widget(list), &text.value, Dword(entry), 0);
            return 0;
        }
    }
    return 6;
}

void filter_pick_values(PickList *list, const ToggleFilter *filter) {
    const Dword previous = selected_pick_value(list);
    family::clear_combo_rows(pick_widget(list));
    if (filter->count)
        populate_filtered_pick_list(list, filter);
    if (filter->count) {
        if (filter->count != 1) {
            gui::widgets::set_enabled(pick_widget(list), 1);
            if (!select_pick_value(list, previous))
                select_pick_row(list, 0);
            return;
        }
        const Dword before = selected_pick_value(list);
        const Dword changed = family::select_combo_row(pick_widget(list), 0);
        const Dword after = selected_pick_value(list);
        if (changed && after != before)
            network::packets::pending::set_rule_variant(system(72), int(list->binding),
                                                                 after);
    }
    gui::widgets::set_enabled(pick_widget(list), 0);
}
} // namespace nfs::application::frontend
