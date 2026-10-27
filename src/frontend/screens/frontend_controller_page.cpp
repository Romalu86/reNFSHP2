#include "../../profiles/ui/frontend_profile_controls.hpp"
#include "../../gui/lifecycle/gui_configuration_cache.hpp"
#include "nfs/audio/music/music_clients.hpp"
#include <cstring>
namespace nfs::gui::widgets {
void register_deferred_widget(ContextState *, Widget *);
}
namespace nfs::application::pending {
void *create_widget(const gui::String *, const gui::String *, const gui::String *, void *);
}
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::at;
using game::runtime::bytes;
using game::runtime::invoke;
using game::runtime::system;
struct ControlRow {
    int row, action;
    Dword label;
};
static ControlRow control_rows[] = {{-1, 0, 1063},  {-1, 1, 1062},  {-1, 2, 344},   {-1, 4, 1274},
                                    {-1, 6, 570},   {-1, 7, 571},   {-1, 14, 1275}, {-1, 10, 575},
                                    {-1, 8, 337},   {-1, 9, 336},   {-1, 11, 576},  {-1, 15, 1309},
                                    {-1, 16, 1310}, {-1, 17, 1311}, {-1, 0, 0}};

Dword menu_list_data(family::ListItems *items, Dword row, Dword column) {
    auto *item = invoke<family::ListItem *>(items, 9, row, column);
    return item ? item->data : 0;
}

const gui::String *menu_list_text(family::ListItems *items, Dword row, Dword column) {
    auto *item = invoke<family::ListItem *>(items, 9, row, column);
    return item ? &item->text : &lifecycle::construction::unresolved::shared_string;
}
static void set_binding_cell(ControllerPage *page, const gui::String *text, Dword row, Dword column,
                             Dword data = 0, Dword flags = 0) {
    family::ListItem item;
    construct_toggle_item(&item, text, data);
    item.flags = flags;
    try {
        invoke(&page->list->items, 7, &item, row, column);
    } catch (...) {
        gui::string_destruct(&item.tooltip);
        gui::string_destruct(&item.text);
        throw;
    }
    gui::string_destruct(&item.tooltip);
    gui::string_destruct(&item.text);
}

ControllerPage *construct_controller_page(ControllerPage *page, void *parent) {
    construct_widget(page, parent);
    page->preferences = nullptr;
    page->label_column = 0;
    page->binding_column = 1;
    page->list = nullptr;
    for (auto &binding : page->bindings)
        binding = 0;
    page->has_wheel = 0;
    page->base.base.methods = controller_page_methods.data();
    try {
        page->preferences = bytes(system(72)) + 2488;
        if (!page->preferences)
            std::memset(page->bindings, 0, sizeof(page->bindings));
        else
            std::memcpy(page->bindings, bytes(system(72)) + 2544, sizeof(page->bindings));
        for (int index = 0; index < int(reinterpret_cast<Dword>(input::device_get(0, 1))); ++index)
            if (void *device = input::device_get(index, 2)) {
                const Dword flags = at<Dword>(device, 4);
                if (flags & 1)
                    if (void *state = input::device_get(index, 6))
                        if (at<std::uint8_t>(state, 0) == 4 && (flags & 2))
                            page->has_wheel = 1;
            }
        return page;
    } catch (...) {
        destroy_general(&page->base);
        throw;
    }
}

static void defer_controller_page(Widget *page, Dword defer) {
    if (defer)
        register_deferred_widget(gui::context_state, page);
    else if (page) {
        const int index = int(gui::container_find_index(&gui::context_state->fourth,
                                                        gui::events::compare_widget_id,
                                                        reinterpret_cast<void *>(page->id)));
        if (index >= 0)
            gui::container_remove_at(&gui::context_state->fourth, Dword(index));
    }
}

void __fastcall initialize_controller_page(ControllerPage *page, void *) {
    gui::widgets::pending::request_general_layout(&page->base, nullptr);
    defer_controller_page(&page->base.base, 1);
    auto *list = find_menu_path(&page->base.base, 1, "<CONTROLS>", Dword(1), "<LIST>", Dword(0));
    page->list = list && invoke<Dword>(list, 2, family::list_box_class_key)
                     ? reinterpret_cast<family::ListBox *>(list)
                     : nullptr;
    if (page->list) {
        invoke(&page->list->items, 1);
        Dword row = 0;
        for (auto *entry = control_rows; entry->label; ++entry, ++row) {
            entry->row = int(row);
            pending::LocalFrontendString text;
            gui::string_reference(&text.value, entry->label);
            set_binding_cell(page, &text.value, row, page->label_column, 1);
            set_binding_cell(page, &lifecycle::construction::unresolved::shared_string,
                             row, page->binding_column);
        }
        set_binding_cell(page, &lifecycle::construction::unresolved::shared_string, row,
                         page->label_column, 0, 2);
        pending::LocalFrontendString text;
        gui::string_reference(&text.value, 581);
        set_binding_cell(page, &text.value, row + 1, page->label_column);
        gui::string_reference(&text.value, 1753);
        set_binding_cell(page, &text.value, row + 1, page->binding_column);
        gui::string_reference(&text.value, 430);
        set_binding_cell(page, &text.value, row + 2, page->label_column);
        gui::string_reference(&text.value, 1796);
        set_binding_cell(page, &text.value, row + 2, page->binding_column);
        invoke(&page->list->items, 10, Dword(0), Dword(1));
    }
    refresh_controller_bindings(page);
}

void refresh_controller_bindings(ControllerPage *page) {
    if (!page->preferences || !page->list)
        return;
    Dword has_analog = 0;
    void *bindings = controllers::get_bindings();
    for (const auto *entry = control_rows; entry->label; ++entry) {
        set_binding_cell(page, &lifecycle::construction::unresolved::shared_string,
                         Dword(entry->row), page->binding_column);
        if (entry->action < 0 || entry->action >= 26)
            continue;
        Dword *binding = page->bindings + entry->action;
        void *control = resolve_menu_control(bindings, binding, 0, false);
        const auto kind = std::uint8_t(*binding);
        if (kind == 1 || kind == 3)
            has_analog = 1;
        const auto *old =
            menu_list_text(&page->list->items, Dword(entry->row), page->binding_column);
        if (!gui::string_equal(old, &lifecycle::construction::unresolved::shared_string))
            continue;
        pending::LocalFrontendString text;
        if (!describe_menu_binding(control, &text.value))
            gui::string_reference(&text.value, 758);
        set_binding_cell(page, &text.value, Dword(entry->row), page->binding_column);
    }
    dispatch_target(family::named_child_id(&page->base.base, "<FFB>"), 25, page->has_wheel, 0);
    dispatch_target(family::named_child_id(&page->base.base, "<DEADZONE>"), 25, has_analog, 0);
}

void open_force_feedback_dialog(Widget *) {
    Dword previous[4];
    for (unsigned index = 0; index < 4; ++index)
        previous[index] = network::packets::rule_value(system(72), 53 + int(index));
    pending::LocalFrontendString type("GDialog"), name("FFB"), file("system\\GDialogs.gui");
    auto *dialog = static_cast<Widget *>(
        pending::create_widget(&file.value, &name.value, &type.value, gui::focused_widget()));
    if (dialog) {
        if (gui::run_modal(dialog, nullptr) != 2)
            for (unsigned index = 0; index < 4; ++index)
                network::packets::pending::set_rule_variant(system(72), 53 + int(index),
                                                                     previous[index]);
        pending::destroy_screen(dialog);
    }
}

void __fastcall controller_page_child_event(ControllerPage *page, void *, Widget *sender,
                                            Dword event, Dword, Dword) {
    if (event == 14) {
        if (sender)
            invoke<Dword>(sender, 2, family::button_class_key);
        return;
    }
    if (event != 43 || !sender || !invoke<Dword>(sender, 2, family::button_class_key))
        return;
    if (gui::string_data(&sender->name) && !gui::string_compare_ascii(&sender->name, "<FFB>")) {
        open_force_feedback_dialog(&page->base.base);
        return;
    }
    if (gui::string_data(&sender->name) && !gui::string_compare_ascii(&sender->name, "<DEADZONE>"))
        if (open_deadzone_dialog(page->bindings, 26))
            refresh_controller_bindings(page);
}

void __fastcall release_controller_page(ControllerPage *page, void *) {
    if (page->preferences)
        std::memcpy(bytes(page->preferences) + 56, page->bindings, sizeof(page->bindings));
    clear_layout_request(&page->base.base, nullptr);
}

void modify_controller_binding(ControllerPage *page) {
    if (!page->list)
        return;
    const int row = family::selected_list_row(&page->list->items);
    if (row < 0)
        return;
    if (row < 15 && menu_list_data(&page->list->items, Dword(row), page->label_column)) {
        const Dword action = Dword(control_rows[row].action);
        pending::LocalFrontendString prompt, label;
        gui::string_reference(&prompt.value, 1750);
        gui::string_reference(&label.value, control_rows[row].label);
        gui::string_format(&prompt.value, &prompt.value, gui::string_data(&label.value));
        pending::LocalFrontendString type("HP2ControlRemapDlg"), name("HP2ControlRemap"),
            file("system\\GDialogs.gui");
        auto *dialog = reinterpret_cast<ControlRemapDialog *>(pending::create_message_widget(
            &file.value, &name.value, &type.value, page, &prompt.value,
            &lifecycle::construction::unresolved::shared_string));
        if (dialog) {
            game::audio::play_frontend_event(32, 127);
            const int result = gui::run_modal(dialog, nullptr);
            Dword binding = dialog->binding;
            const Dword value = dialog->value;
            pending::destroy_screen(dialog);
            if (result == 2 && std::uint8_t(binding)) {
                assign_controller_binding(page, action, &binding, value);
                refresh_controller_bindings(page);
            }
        }
        return;
    }
    pending::LocalFrontendString message;
    gui::string_reference(&message.value, 1813);
    pending::ScopedMessageBox box(&message.value, 8449);
    pending::run_message_box(&box.value, nullptr, 0);
}

void __fastcall controller_page_command(ControllerPage *page, void *, Dword, Dword command) {
    if (page->list) {
        Dword hash;
        gui::engine::pending::hash_name(&hash, "MODIFYCONTROL");
        if (command == hash)
            modify_controller_binding(page);
    }
}
} // namespace nfs::application::frontend
