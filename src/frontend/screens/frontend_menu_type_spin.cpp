#include "frontend_menu_controls.hpp"
#include "../../input/controllers/controller_interfaces.hpp"
#include "../../gui/widgets/gui_widget_list.hpp"
#include "nfs/network/packets/network_packet_handlers.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
namespace nfs::application::frontend {
using namespace gui::widgets;
using family::ListItem;
using game::runtime::invoke;
using game::runtime::system;

bool read_catalog_value(void *configuration, const gui::String *key, Dword *value) {
    const auto *text = invoke<const gui::String *>(configuration, 5, gui::string_ascii(key));
    if (gui::engine::pending::character_at(text, 0))
        for (const auto *entry = toggle_values; entry->name; ++entry)
            if (toggle_string_equals(text, entry->name)) {
                *value = entry->value;
                return true;
            }
    return gui::engine::pending::read_int(configuration, gui::string_ascii(key),
                                          reinterpret_cast<int *>(value), 0) != 0;
}

Dword type_spin_value(TypeSpinControl *widget, int row) {
    auto *item = invoke<ListItem *>(&widget->items, 9, Dword(row), Dword(0));
    return item ? item->data : 0;
}

static const gui::String *type_spin_text(TypeSpinControl *widget, int row) {
    auto *item = invoke<ListItem *>(&widget->items, 9, Dword(row), Dword(0));
    return item ? &item->text : &lifecycle::construction::unresolved::shared_string;
}
static void publish_type_spin(TypeSpinControl *widget) {
    if (widget->display)
        invoke(widget->display, 7,
               type_spin_text(widget, family::selected_list_row(&widget->items)));
    const Dword value = type_spin_value(widget, family::selected_list_row(&widget->items));
    network::packets::pending::set_rule_variant(system(72), int(widget->binding), value);
}
static void publish_type_spin_key(TypeSpinControl *widget) {
    if (widget->display) {
        const auto *methods = widget->display->methods;
        const int row = family::selected_list_row(&widget->items);
        const auto *text = type_spin_text(widget, row);
        using SetText = void(__thiscall *)(void *, const gui::String *);
        reinterpret_cast<SetText>(methods[7])(widget->display, text);
    }
    auto *rules = system(72);
    const Dword value = type_spin_value(widget, family::selected_list_row(&widget->items));
    network::packets::pending::set_rule_variant(rules, int(widget->binding), value);
}

void select_type_spin(TypeSpinControl *widget, int row) {
    invoke<Dword>(&widget->items, 10, Dword(row), Dword(1));
    publish_type_spin(widget);
}
namespace {
struct ConsumedString {
    gui::String *value;
    ~ConsumedString() {
        gui::string_destruct(value);
    }
};
struct ListConfiguration {
    alignas(4) std::uint8_t storage[56];
    explicit ListConfiguration(const gui::String *section) {
        pending::LocalFrontendString file("lists.ini");
        gui::engine::pending::create_configuration(storage, &file.value);
        try {
            gui::engine::pending::select_section(storage, gui::string_ascii(section));
        } catch (...) {
            gui::engine::pending::destroy_configuration(storage);
            throw;
        }
    }
    ~ListConfiguration() {
        gui::engine::pending::destroy_configuration(storage);
    }
};
void append_item(TypeSpinControl *widget, const gui::String *text, Dword value) {
    ListItem item;
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
} // namespace

ListItem *construct_ascii_item(ListItem *item, const char *text, Dword value) {
    gui::string_construct(&item->text);
    try {
        gui::string_construct(&item->tooltip);
        try {
            item->methods = family::list_item_methods;
            gui::string_assign(&item->text, text);
            item->data = value;
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
// The section String is consumed by value.
static Dword populate_type_spin(TypeSpinControl *widget, gui::String section) {
    ConsumedString consumed{&section};
    invoke(&widget->items, 1);
    ListConfiguration configuration(&section);
    const int count = invoke<int>(configuration.storage, 3);
    for (int index = 0; index < count; ++index) {
        const auto *key = invoke<const gui::String *>(configuration.storage, 4, Dword(index));
        if (!gui::string_data(key))
            continue;
        Dword value = 0;
        if (!read_catalog_value(configuration.storage, key, &value)) {
            ListItem item;
            construct_ascii_item(&item, "(Null)", 0);
            try {
                invoke(&widget->items, 8, &item);
            } catch (...) {
                gui::string_destruct(&item.tooltip);
                gui::string_destruct(&item.text);
                throw;
            }
            gui::string_destruct(&item.tooltip);
            gui::string_destruct(&item.text);
        } else {
            const Dword reference = controllers::resolve_control_reference(gui::string_ascii(key));
            if (reference) {
                pending::LocalFrontendString label;
                gui::string_reference(&label.value, reference);
                append_item(widget, &label.value, value);
            } else
                append_item(widget, key, value);
        }
    }
    return widget->items.grid.rows.count;
}

void __fastcall initialize_type_spin(TypeSpinControl *widget, void *) {
    gui::widgets::pending::request_general_layout(&widget->base, nullptr);
    widget->display = static_cast<Widget *>(pending::find_child(widget, "<t0>"));
    widget->previous_button = family::named_child_id(&widget->base.base, "<bL>");
    widget->next_button = family::named_child_id(&widget->base.base, "<bR>");
    pending::LocalFrontendString section(&widget->list_name);
    const Dword count = populate_type_spin(widget, section.release());
    const Dword selected = network::packets::rule_value(system(72), int(widget->binding));
    for (int row = 0; row < int(count); ++row)
        if (type_spin_value(widget, row) == selected) {
            select_type_spin(widget, row);
            break;
        }
}

void __fastcall draw_type_spin(TypeSpinControl *widget, void *, void *render) {
    if (!widget->display)
        return;
    invoke(render, 21, family::inherited_palette(widget->display));
    auto *active =
        static_cast<Widget *>(gui::events::find_widget(gui::context_state, active_widget_id));
    const Dword focused =
        active && (active == &widget->base.base || descendant(&widget->base.base, active->id));
    Dword colour;
    widget->display->colour = *invoke<Dword *>(render, 23, &colour, focused);
}
static int adjacent_type_spin(TypeSpinControl *widget, bool previous) {
    const int selected = family::selected_list_row(&widget->items);
    int row = previous ? family::previous_list_row(&widget->items, selected)
                       : family::next_list_row(&widget->items, selected);
    if (row < 0) {
        if (!previous)
            return 0;
        row = int(widget->items.grid.rows.count);
        do {
            --row;
        } while (row >= 0 && family::list_row_disabled(&widget->items, Dword(row)));
    }
    return row;
}

Dword __fastcall type_spin_event(TypeSpinControl *widget, void *, Dword event, Dword first,
                                Dword second) {
    bool previous;
    if (event == 6 && (first == 203 || first == 205))
        previous = first == 203;
    else if (event == 106 || event == 107)
        previous = event == 107;
    else {
        if (event == 24) {
            const Dword value = type_spin_value(widget, family::selected_list_row(&widget->items));
            network::packets::pending::set_rule_variant(system(72), int(widget->binding),
                                                                 value);
        }
        return gui::widgets::pending::general_event(&widget->base, nullptr, event, first, second);
    }
    const int row = adjacent_type_spin(widget, previous);
    if (event == 6) {
        invoke<Dword>(&widget->items, 10, Dword(row), Dword(1));
        publish_type_spin_key(widget);
    } else
        select_type_spin(widget, row);
    latch_menu_spin(previous ? widget->previous_button : widget->next_button);
    return 1;
}

void __fastcall type_spin_child_event(TypeSpinControl *widget, void *, Widget *child, Dword event,
                                      Dword, Dword) {
    const Dword id = child->id;
    if (event == 43) {
        if (id == widget->previous_button)
            select_type_spin(widget, adjacent_type_spin(widget, true));
        else if (id == widget->next_button)
            select_type_spin(widget, adjacent_type_spin(widget, false));
    }
}

Dword __fastcall load_type_spin(TypeSpinControl *widget, void *, void *configuration) {
    gui::engine::pending::read_reference_string(
        configuration, "TypeSpinCtl.m_listtype", &widget->list_name,
        &lifecycle::construction::unresolved::shared_string);
    gui::engine::pending::read_enum(configuration, "TypeSpinCtl.m_DataEnum", &widget->binding,
                                    type_spin_bindings, 0);
    return gui::widgets::pending::load_general(&widget->base, nullptr, configuration);
}

Dword __fastcall save_type_spin(TypeSpinControl *widget, void *, void *configuration) {
    gui::engine::pending::write_reference_string(
        configuration, "TypeSpinCtl.m_listtype", &widget->list_name,
        &lifecycle::construction::unresolved::shared_string);
    gui::engine::pending::write_enum(configuration, "TypeSpinCtl.m_DataEnum", widget->binding,
                                     type_spin_bindings, 0);
    return gui::widgets::pending::save_general(&widget->base, nullptr, configuration);
}
} // namespace nfs::application::frontend
