#include "../storage/player_profile.hpp"
#include "../../gui/widgets/gui_widget_list.hpp"
#include "nfs/resources/files/directory_search.hpp"
#include "nfs/resources/ini/ini.hpp"
#include "nfs/core/configuration/launch_options.hpp"
#include <cstdio>
#include <cstdlib>
namespace nfs::profiles {

gui::widgets::family::ListItem *construct_profile_list_item(gui::widgets::family::ListItem *item) {
    gui::string_construct(&item->text);
    gui::string_construct(&item->tooltip);
    item->methods = gui::widgets::family::list_item_methods;
    item->data = item->flags = item->image = 0;
    return item;
}
// The model is an embedded object, not a pointer at+138.
const gui::String *selected_profile_name(void *widget) {
    auto *model = reinterpret_cast<gui::widgets::family::ListItems *>(bytes(widget) + 312);
    const auto *methods = model->methods;
    const int row = gui::widgets::family::selected_list_row(model);
    using Cell = gui::widgets::family::ListItem *(__thiscall *)(void *, Dword, Dword);
    auto *item = reinterpret_cast<Cell>(methods[9])(model, Dword(row), Dword(0));
    return item ? &item->text : &lifecycle::construction::unresolved::shared_string;
}

Dword select_profile_row(void *widget, Dword row) {
    if (invoke<Dword>(bytes(widget) + 312, 10, row, Dword(1))) {
        gui::events::dispatch(widget, 68, 0, 0);
        return 1;
    }
    return 0;
}
 
const char *__cdecl profile_list_class_key() {
    return "ToggleButton";
}
} // namespace nfs::profiles
namespace nfs::application::pending::frontend_dependencies {
using namespace profiles;

Byte prompt_load_profile(void *parent) {
    if (!parent)
        return 0;
    LocalFrontendString chosen(&lifecycle::construction::unresolved::shared_string), title;
    gui::string_reference(&title.value, 595);
    Widget *dialog;
    {
        LocalFrontendString text("Load Profile"), type("GMsgBox"), name("LoadProfile"),
            file("system\\GDialogs.gui");
        dialog = create_message_widget(
            &file.value, &name.value, &type.value, widget_root(static_cast<Widget *>(parent)),
            &text.value, &lifecycle::construction::unresolved::shared_string);
    }
    if (!dialog)
        return 0;
    void *list = find_child(dialog, "<PROFILES>");
    if (!list || !invoke<Dword>(list, 2, profile_list_class_key))
        return 0;
    void *model = bytes(list) + 312;
    invoke(model, 1);
    int file_count = 0;
    files::DirectorySearch search;
    search.attributes = 160;
    char root[260], saved_directory[260], last_profile_path[260];
    std::sprintf(root, "%s", graphics::startup::configuration_path());
    files::get_directory(saved_directory);
    files::set_directory(root);
    std::sprintf(last_profile_path, "%s%s", graphics::startup::configuration_path(),
                 "lastprof.ini");
    LocalFrontendString last_profile(&lifecycle::construction::unresolved::shared_string);
    if (files::file::exists(last_profile_path)) {
        ini::Reader reader;
        ini::reader_construct(&reader, last_profile_path, 0);
        try {
            char section[80]{}, name[80]{};
            std::sprintf(section, "lastprof");
            ini::get_string(&reader, section, "prof", name, 79);
            gui::string_assign(&last_profile.value, name);
        } catch (...) {
            ini::reader_clear(&reader);
            throw;
        }
        ini::reader_clear(&reader);
    }
    if (files::find_first("*.sav", &search)) {
        do {
            if (search.attributes & 160) {
                file_count = int(Dword(file_count) + 1);
                if (files::file::try_length(search.name) == file_size) {
                    LocalFrontendString visible("%s"), filename(search.name),
                        stem(&lifecycle::construction::unresolved::shared_string);
                    gui::string_substring(&filename.value, 0, filename.value.length - 4,
                                          &stem.value);
                    if (stem.value.length > 0) {
                        gui::string_format(&visible.value, &visible.value,
                                           gui::string_ascii(&stem.value));
                        const int row = invoke<int>(model, 3);
                        gui::widgets::family::ListItem item;
                        construct_profile_list_item(&item);
                        try {
                            gui::string_copy(&item.text, &stem.value);
                            item.data = Dword(file_count);
                            item.flags &= ~Dword(2);
                            invoke(model, 7, &item, Dword(row), Dword(0));
                        } catch (...) {
                            gui::string_destruct(&item.tooltip);
                            gui::string_destruct(&item.text);
                            throw;
                        }
                        gui::string_destruct(&item.tooltip);
                        gui::string_destruct(&item.text);
                        item.methods =
                            reinterpret_cast<const gui::engine::Method *>(gui::base_methods);
                    }
                }
            }
        } while (files::find_next(&search));
    }
    if (!file_count) {
        destroy_screen(dialog);
        files::find_close(&search);
        files::set_directory(saved_directory);
        LocalFrontendString message;
        gui::string_reference(&message.value, 1965);
        ScopedMessageBox question(&message.value, 129);
        if (run_message_box(&question.value, nullptr, 0) == 2) {
            LocalFrontendString name;
            return profiles::pending::create_profile(parent, &name.value) ? 1 : 0;
        }
        return 0;
    }
    select_profile_row(list, 0);
    for (int index = 0; index < file_count; ++index) {
        select_profile_row(list, Dword(index));
        LocalFrontendString name(selected_profile_name(list));
        if (gui::string_equal(&name.value, &last_profile.value))
            break;
        select_profile_row(list, 0);
    }
    int result = 2;
    if (at<int>(list, 328) > 1)
        result = gui::run_modal(dialog, nullptr);
    bool loaded = false;
    if (result == 2) {
        gui::string_copy(&chosen.value, selected_profile_name(list));
        if (chosen.value.length) {
            LocalFrontendString filename("%s.sav");
            gui::string_format(&filename.value, &filename.value, gui::string_ascii(&chosen.value));
            auto *profile = static_cast<SavedProfile *>(
                lifecycle::allocate(file_size, "loaded profile", lifecycle::ui_pool));
            if (profile) {
                try {
                    profile = load_profile(profile, gui::string_ascii(&filename.value));
                } catch (...) {
                    gui::application_release(profile);
                    throw;
                }
            }
            if (profile) {
                if (at<Byte>(profile, 50268)) {
                    at<Byte>(system(72), 102664) = 0;
                    loaded = true;
                    apply_profile(profile, 0);
                    if (launch::field<Dword>(8627896)) {
                        const int addition = std::atoi(&launch::field<char>(8629444));
                        void *preferences = player_preferences(system(72), 0);
                        const int current = at<int>(preferences, 7240);
                        if (current < addition)
                            at<Dword>(preferences, 7240) = Dword(current) + Dword(addition);
                    }
                }
                delete_profile(profile, 1);
            }
        }
    }
    destroy_screen(dialog);
    files::find_close(&search);
    files::set_directory(saved_directory);
    if (result != 2)
        return 0;
    if (!loaded) {
        LocalFrontendString text;
        gui::string_reference(&text.value, 1810);
        ScopedMessageBox question(&text.value, 66);
        if (run_message_box(&question.value, nullptr, 0) == 2)
            return profiles::pending::create_profile(parent, &chosen.value) ? 1 : 0;
        return 0;
    }
    profiles::pending::configure_profile_input();
    LocalFrontendString notification, format;
    gui::string_reference(&format.value, 1400);
    gui::string_format(&notification.value, &format.value, gui::string_data(&chosen.value));
    if (!launch::field<Dword>(8627776)) {
        ScopedMessageBox information(&notification.value, 257);
        run_message_box(&information.value, nullptr, 0);
    }
    set_preference_name(player_preferences(system(72), 0), &chosen.value);
    set_preference_network_name(player_preferences(system(72), 0), &chosen.value);
    return 1;
}
} // namespace nfs::application::pending::frontend_dependencies
