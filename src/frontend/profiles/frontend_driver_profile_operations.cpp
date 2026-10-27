#include "../../profiles/ui/frontend_profile_controls.hpp"
#include "../../profiles/storage/player_profile.hpp"
#include "nfs/resources/files/directory_search.hpp"
#include <cstdio>
#include <cstring>
namespace nfs::gui {
int string_find_ascii_folded(const String *, const char *);
}
namespace nfs::profiles {
void assign_player_preferences(void *, const void *, Dword);
const gui::String *selected_profile_name(void *);
gui::widgets::family::ListItem *construct_profile_list_item(gui::widgets::family::ListItem *);
} // namespace nfs::profiles
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::at;
using game::runtime::bytes;
using game::runtime::invoke;
using game::runtime::system;
using pending::LocalFrontendString;
using pending::ScopedMessageBox;
static int profile_notice(Dword reference, Dword flags, const gui::String *argument = nullptr) {
    LocalFrontendString text;
    gui::string_reference(&text.value, reference);
    if (argument)
        gui::string_format(&text.value, &text.value, gui::string_data(argument));
    ScopedMessageBox box(&text.value, flags);
    return pending::run_message_box(&box.value, nullptr, 0);
}
static Widget *profile_selection_dialog(DriverProfilePage *page) {
    LocalFrontendString title;
    gui::string_reference(&title.value, 595);
    LocalFrontendString text("Load Profile"), type("GMsgBox"), name("LoadProfile"),
        file("system\\GDialogs.gui");
    return pending::create_message_widget(
        &file.value, &name.value, &type.value, pending::widget_root(&page->base.base), &text.value,
        &lifecycle::construction::unresolved::shared_string);
}
static ProfileToggle *profile_selection_list(Widget *dialog) {
    auto *child = static_cast<Widget *>(pending::find_child(dialog, "<PROFILES>"));
    return child && invoke<Dword>(child, 2, profiles::profile_list_class_key)
               ? reinterpret_cast<ProfileToggle *>(child)
               : nullptr;
}

static Dword fill_profile_selection(ProfileToggle *toggle) {
    Dword count = 0;
    files::DirectorySearch search;
    search.attributes = 160;
    if (files::find_first("*.sav", &search)) {
        do {
            if (search.attributes & 160) {
                ++count;
                if (files::file::try_length(search.name) == profiles::file_size) {
                    LocalFrontendString visible("%s"), filename(search.name),
                        stem(&lifecycle::construction::unresolved::shared_string);
                    gui::string_substring(&filename.value, 0, filename.value.length - 4,
                                          &stem.value);
                    if (stem.value.length > 0) {
                        gui::string_format(&visible.value, &visible.value,
                                           gui::string_ascii(&stem.value));
                        const Dword row = invoke<Dword>(&toggle->items, 3);
                        family::ListItem item;
                        profiles::construct_profile_list_item(&item);
                        try {
                            gui::string_copy(&item.text, &stem.value);
                            item.data = count;
                            item.flags &= ~Dword(2);
                            invoke(&toggle->items, 7, &item, row, Dword(0));
                        } catch (...) {
                            gui::string_destruct(&item.tooltip);
                            gui::string_destruct(&item.text);
                            throw;
                        }
                        gui::string_destruct(&item.tooltip);
                        gui::string_destruct(&item.text);
                    }
                }
            }
        } while (files::find_next(&search));
        files::find_close(&search);
    }
    return count;
}

static void *__fastcall delete_page_preferences(void *preferences, void *, Dword flags) {
    profiles::destroy_preferences(preferences);
    if (flags & 1)
        gui::application_release(preferences);
    return preferences;
}
static void *new_page_preferences() {
    void *preferences = gui::application_allocate(profiles::preferences_size);
    if (preferences) {
        try {
            profiles::construct_preferences(preferences);
        } catch (...) {
            gui::application_release(preferences);
            throw;
        }
    }
    return preferences;
}

void create_page_profile(DriverProfilePage *page) {
    LocalFrontendString unused(&lifecycle::construction::unresolved::shared_string),
        chosen, filename;
    gui::string_reference(&chosen.value, 2064);
    const char *path = "";
    for (;;) {
        {
            LocalFrontendString text, initial, title("New Profile"), name("CreateProfile");
            gui::string_reference(&text.value, 602);
            gui::string_reference(&initial.value, 2064);
            if (pending::prompt_text_entry(pending::widget_root(&page->base.base), text.release(),
                                           title.release(), &chosen.value, &initial.value, 12,
                                           name.release(), 16384, nullptr, 0, 0) != 2)
                return;
        }
        if (!gui::string_make_narrow(&chosen.value))
            return;
        {
            gui::String extended;
            gui::append_ascii_value(&extended, &chosen.value, ".sav");
            gui::string_copy(&filename.value, &extended);
            gui::string_destruct(&extended);
        }
        path = gui::string_ascii(&filename.value);
        if (gui::string_equal_folded(&filename.value, "*.sav") || !chosen.value.length) {
            profile_notice(1808, 8449, &chosen.value);
            continue;
        }
        if (gui::string_find_ascii_folded(&filename.value, "/") >= 0) {
            profile_notice(1744, 8449, &chosen.value);
            continue;
        }
        if (enumerate_page_profiles(path, nullptr)) {
            if (profile_notice(1455, 66) != 2)
                continue;
            delete_profile_files(path);
        }
        break;
    }
    void *fresh = new_page_preferences();
    void *backup = new_page_preferences();
    profiles::copy_profile_dwords(backup, bytes(system(72)) + 2488,
                                   profiles::preferences_size / sizeof(Dword));
    if (fresh) {
        pending::set_preference_name(fresh, &chosen.value);
        pending::set_preference_network_name(fresh, &chosen.value);
        void *current = pending::player_preferences(system(72), 0);
        if (bytes(current) + 56 && bytes(fresh) + 56)
            profiles::copy_profile_dwords(bytes(fresh) + 56, bytes(current) + 56, 26);
        profiles::assign_player_preferences(system(72), fresh, 0);
    }
    LocalFrontendString preparing;
    gui::string_reference(&preparing.value, 1399);
    gui::string_format(&preparing.value, &preparing.value, gui::string_data(&chosen.value));
    ScopedMessageBox progress(&preparing.value, 257);
    pending::run_message_box(&progress.value, nullptr, 0);
    if (!save_named_profile(path)) {
        LocalFrontendString text;
        gui::string_reference(&text.value, 1407);
        gui::string_format(&text.value, &text.value, gui::string_data(&chosen.value));
        ScopedMessageBox error(&text.value, 8449);
        pending::run_message_box(&error.value, nullptr, 0);
        if (page->current_name)
            invoke(page->current_name, 7,
                   &lifecycle::construction::unresolved::shared_string);
        if (backup)
            profiles::assign_player_preferences(system(72), backup, 0);
    } else {
        LocalFrontendString text;
        gui::string_reference(&text.value, 1398);
        gui::string_format(&text.value, &text.value, gui::string_data(&chosen.value));
        ScopedMessageBox information(&text.value, 257);
        pending::run_message_box(&information.value, nullptr, 0);
        if (page->current_name)
            invoke(page->current_name, 7, &chosen.value);
        profiles::assign_player_preferences(system(72), fresh, 0);
        LocalFrontendString remembered(&chosen.value);
        remember_profile(remembered.release());
    }
    if (fresh)
        delete_page_preferences(fresh, nullptr, 1);
    if (backup)
        delete_page_preferences(backup, nullptr, 1);
    refresh_profile_statistics(page);
}

void save_page_profile(DriverProfilePage *page) {
    if (gui::string_equal(invoke<const gui::String *>(page->current_name, 8),
                          &lifecycle::construction::unresolved::shared_string)) {
        profile_notice(1745, 8449);
        return;
    }
    gui::String name;
    pending::copy_preference_name(page->preferences, &name);
    struct NameOwner {
        gui::String *name;
        ~NameOwner() {
            gui::string_destruct(name);
        }
    } owner{&name};
    LocalFrontendString filename(&name);
    gui::string_append_ascii(&filename.value, ".sav");
    if (!save_named_profile(gui::string_ascii(&filename.value)))
        profile_notice(1407, 8449, &name);
    else {
        LocalFrontendString text;
        gui::string_reference(&text.value, 1398);
        gui::string_format(&text.value, &text.value, gui::string_data(&name));
        ScopedMessageBox information(&text.value, 257);
        pending::run_message_box(&information.value, nullptr, 0);
        LocalFrontendString remembered(&name);
        remember_profile(remembered.release());
    }
    refresh_profile_statistics(page);
}

void delete_page_profile(DriverProfilePage *page) {
    Widget *dialog = profile_selection_dialog(page);
    if (!dialog)
        return;
    auto *toggle = profile_selection_list(dialog);
    if (!toggle) {
        pending::destroy_screen(dialog);
        return;
    }
    invoke(&toggle->items, 1);
    char root[260], saved_directory[260];
    std::sprintf(root, "%s", graphics::startup::configuration_path());
    files::get_directory(saved_directory);
    files::set_directory(root);
    const Dword count = fill_profile_selection(toggle);
    files::set_directory(saved_directory);
    if (!count) {
        pending::destroy_screen(dialog);
        profile_notice(1809, 257);
        return;
    }
    profiles::select_profile_row(toggle, 0);
    const int result = at<int>(toggle, 328) > 1 ? gui::run_modal(dialog, nullptr) : 2;
    // The saved cancel branch leaves this dialog owned by the GUI context.
    if (result != 2)
        return;
    LocalFrontendString chosen(profiles::selected_profile_name(toggle));
    pending::destroy_screen(dialog);
    if (!chosen.value.length) {
        profile_notice(1745, 8449);
        return;
    }
    LocalFrontendString confirmation;
    gui::string_reference(&confirmation.value, 1394);
    gui::string_format(&confirmation.value, &confirmation.value, gui::string_data(&chosen.value));
    ScopedMessageBox question(&confirmation.value, 16450);
    if (pending::run_message_box(&question.value, nullptr, 0) == 2) {
        LocalFrontendString filename(&chosen.value);
        gui::string_append_ascii(&filename.value, ".sav");
        delete_profile_files(gui::string_ascii(&filename.value));
        profile_notice(1397, 257, &chosen.value);
    }
    refresh_profile_statistics(page);
}

void load_page_profile(DriverProfilePage *page) {
    if (!page->current_name)
        return;
    LocalFrontendString chosen(&lifecycle::construction::unresolved::shared_string);
    Widget *dialog = profile_selection_dialog(page);
    if (!dialog)
        return;
    auto *toggle = profile_selection_list(dialog);
    if (!toggle) {
        pending::destroy_screen(dialog);
        return;
    }
    invoke(&toggle->items, 1);
    char root[260], saved_directory[260];
    std::sprintf(root, "%s", graphics::startup::configuration_path());
    files::get_directory(saved_directory);
    files::set_directory(root);
    const Dword count = fill_profile_selection(toggle);
    if (!count) {
        pending::destroy_screen(dialog);
        files::set_directory(saved_directory);
        LocalFrontendString text;
        gui::string_reference(&text.value, 1965);
        ScopedMessageBox question(&text.value, 66);
        if (pending::run_message_box(&question.value, nullptr, 0) == 2)
            create_page_profile(page);
        refresh_profile_statistics(page);
        return;
    }
    profiles::select_profile_row(toggle, 0);
    const int result = at<int>(toggle, 328) > 1 ? gui::run_modal(dialog, nullptr) : 2;
    bool loaded = false;
    if (result == 2) {
        gui::string_copy(&chosen.value, profiles::selected_profile_name(toggle));
        if (chosen.value.length) {
            LocalFrontendString filename("%s.sav");
            gui::string_format(&filename.value, &filename.value, gui::string_ascii(&chosen.value));
            auto *profile = static_cast<profiles::SavedProfile *>(
                lifecycle::allocate(profiles::file_size, "loaded profile", lifecycle::ui_pool));
            if (profile) {
                try {
                    profile = profiles::load_profile(profile, gui::string_ascii(&filename.value));
                } catch (...) {
                    gui::application_release(profile);
                    throw;
                }
            }
            if (profile) {
                if (at<std::uint8_t>(profile, 50268)) {
                    at<std::uint8_t>(system(72), 102664) = 0;
                    profiles::apply_profile(profile, 0);
                    loaded = true;
                    invoke(page->current_name, 7, &chosen.value);
                }
                profiles::destroy_profile(profile);
                gui::application_release(profile);
            }
        }
    }
    pending::destroy_screen(dialog);
    files::set_directory(saved_directory);
    if (result != 2)
        return;
    if (!loaded) {
        LocalFrontendString text;
        gui::string_reference(&text.value, 1810);
        ScopedMessageBox question(&text.value, 66);
        if (pending::run_message_box(&question.value, nullptr, 0) == 2)
            create_page_profile(page);
    } else {
        LocalFrontendString text;
        gui::string_reference(&text.value, 1400);
        gui::string_format(&text.value, &text.value, gui::string_data(&chosen.value));
        ScopedMessageBox information(&text.value, 257);
        pending::run_message_box(&information.value, nullptr, 0);
        pending::set_preference_name(pending::player_preferences(system(72), 0), &chosen.value);
        pending::set_preference_network_name(pending::player_preferences(system(72), 0),
                                             &chosen.value);
        invoke(page->current_name, 7, &chosen.value);
        LocalFrontendString remembered(&chosen.value);
        remember_profile(remembered.release());
    }
    refresh_profile_statistics(page);
}
} // namespace nfs::application::frontend
