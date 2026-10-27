#include "frontend_toggle_widget.hpp"
#include "../../input/controllers/controller_interfaces.hpp"
#include "nfs/audio/music/music_clients.hpp"
#include "nfs/network/packets/network_packet_handlers.hpp"
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::invoke;

family::ListItem *construct_toggle_item(family::ListItem *item, const gui::String *text,
                                        Dword data) {
    gui::string_construct(&item->text);
    try {
        gui::string_construct(&item->tooltip);
        try {
            item->methods = family::list_item_methods;
            gui::string_copy(&item->text, text);
            item->data = data;
            item->flags = item->image = 0;
        } catch (...) {
            gui::string_destruct(&item->tooltip);
            throw;
        }
    } catch (...) {
        gui::string_destruct(&item->text);
        throw;
    }
    return item;
}
namespace {
struct ToggleConfiguration {
    alignas(4) std::uint8_t storage[56];
    ToggleConfiguration(ProfileToggle *widget) {
        pending::LocalFrontendString file("lists.ini");
        gui::engine::pending::create_configuration(storage, &file.value);
        try {
            gui::engine::pending::select_section(
                storage,
                gui::string_ascii(widget->list_name.length < 1 ? &widget->button.base.base.name
                                                               : &widget->list_name));
        } catch (...) {
            gui::engine::pending::destroy_configuration(storage);
            throw;
        }
    }
    ~ToggleConfiguration() {
        gui::engine::pending::destroy_configuration(storage);
    }
};
bool read_toggle_value(void *configuration, const gui::String *key, Dword *out,
                       const gui::String **value) {
    *out = 0;
    *value = invoke<const gui::String *>(configuration, 5, gui::string_ascii(key));
    bool found = false;
    if (gui::engine::pending::character_at(*value, 0))
        for (const auto *entry = toggle_values; entry->name; ++entry)
            if (toggle_string_equals(*value, entry->name)) {
                *out = entry->value;
                found = true;
                break;
            }
    if (!found)
        found = gui::engine::pending::read_int(configuration, gui::string_ascii(key),
                                               reinterpret_cast<int *>(out), 0) != 0;
    return found;
}
void append_toggle_item(ProfileToggle *widget, const gui::String *text, Dword value) {
    family::ListItem item;
    construct_toggle_item(&item, text, value);
    try {
        invoke(&widget->items, 8, &item);
    } catch (...) {
        gui::string_destruct(&item.tooltip);
        gui::string_destruct(&item.text);
        throw;
    }
    gui::string_destruct(&item.tooltip);
    gui::string_destruct(&item.text);
}
void append_translated_item(ProfileToggle *widget, const gui::String *key, Dword value) {
    const Dword reference = controllers::resolve_control_reference(gui::string_ascii(key));
    if (!reference) {
        append_toggle_item(widget, key, value);
        return;
    }
    pending::LocalFrontendString translated;
    gui::string_reference(&translated.value, reference);
    append_toggle_item(widget, &translated.value, value);
}
} // namespace

int populate_toggle_list(ProfileToggle *widget) {
    invoke(&widget->items, 1);
    ToggleConfiguration configuration(widget);
    int count = invoke<int>(configuration.storage, 3);
    if (!game::audio::has_multichannel_audio() && widget->binding == 18)
        --count;
    for (int index = 0; index < count; ++index) {
        const auto *key = invoke<const gui::String *>(configuration.storage, 4, Dword(index));
        if (!gui::string_data(key))
            continue;
        Dword value;
        const gui::String *text;
        if (read_toggle_value(configuration.storage, key, &value, &text)) {
            if (gui::engine::pending::character_at(key, 0))
                append_translated_item(widget, key, value);
            if (value == widget->default_value)
                invoke<Dword>(&widget->items, 10, Dword(index), Dword(1));
        }
    }
    return count;
}

void filter_toggle_values(ProfileToggle *widget, const ToggleFilter *filter) {
    const auto *methods = widget->items.methods;
    const int selected_row = family::selected_list_row(&widget->items);
    using Cell = family::ListItem *(__thiscall *)(void *, Dword, Dword);
    auto *selected = reinterpret_cast<Cell>(methods[9])(&widget->items, Dword(selected_row), Dword(0));
    const Dword previous = selected ? selected->data : ~Dword(0);
    invoke(&widget->items, 1);
    if (filter->count != 0) {
        ToggleConfiguration configuration(widget);
        const int count = invoke<int>(configuration.storage, 3);
        for (int index = 0; index < count; ++index) {
            pending::LocalFrontendString key(
                invoke<const gui::String *>(configuration.storage, 4, Dword(index)));
            if (!gui::string_data(&key.value))
                continue;
            Dword value;
            const gui::String *text;
            if (!read_toggle_value(configuration.storage, &key.value, &value, &text) ||
                !gui::engine::pending::character_at(&key.value, 0))
                continue;
            for (int candidate = 0; candidate < filter->count; ++candidate)
                if (filter->values[candidate] == value)
                    append_translated_item(widget, &key.value, value);
        }
    }
    if (filter->count != 0) {
        if (filter->count != 1) {
            set_enabled(&widget->button.base.base, 1);
            const int row = find_toggle_value(widget, previous);
            invoke<Dword>(&widget->items, 10, Dword(row < 0 ? 0 : row), Dword(1));
            return;
        }
        profiles::select_profile_row(widget, 0);
    }
    set_enabled(&widget->button.base.base, 0);
}

void replace_toggle_list(ProfileToggle *widget, const char *name) {
    bool changed = false;
    if (*name) {
        changed = !toggle_string_equals(&widget->list_name, name);
        gui::string_assign(&widget->list_name, name);
    }
    populate_toggle_list(widget);
    set_enabled(&widget->button.base.base, 1);
    if (changed) {
        if (widget->binding) {
            const int row =
                find_toggle_value(widget, network::packets::rule_value(game::runtime::system(72),
                                                                       int(widget->binding)));
            if (row >= 0) {
                profiles::select_profile_row(widget, Dword(row));
                return;
            }
        }
        invoke<Dword>(&widget->items, 10, Dword(0), Dword(1));
    }
}
} // namespace nfs::application::frontend
