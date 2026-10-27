#include "../ui/frontend_profile_controls.hpp"
#include "player_profile.hpp"
#include "nfs/resources/files/directory_search.hpp"
#include "nfs/resources/ini/ini.hpp"
#include <cstdio>
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::at;
using game::runtime::bytes;
using game::runtime::invoke;
using game::runtime::system;
// The incoming String is passed by value and consumed.
void remember_profile(gui::String name) {
    struct Incoming {
        gui::String *name;
        ~Incoming() {
            gui::string_destruct(name);
        }
    } incoming{&name};
    char path[260];
    std::sprintf(path, "%s%s", graphics::startup::configuration_path(), "lastprof.ini");
    pending::LocalFrontendString unused(
        &lifecycle::construction::unresolved::shared_string);
    ini::Reader reader;
    ini::reader_construct(&reader, path, 0);
    try {
        char section[80]{};
        std::sprintf(section, "lastprof");
        ini::set_string(&reader, section, "prof", gui::string_ascii(&name));
    } catch (...) {
        ini::reader_clear(&reader);
        throw;
    }
    ini::reader_clear(&reader);
}

std::uint8_t save_named_profile(const char *filename) {
    char root[260], saved_directory[260];
    std::sprintf(root, "%s", graphics::startup::configuration_path());
    files::get_directory(saved_directory);
    files::set_directory(root);
    std::uint8_t saved = 0;
    auto *profile = static_cast<profiles::SavedProfile *>(lifecycle::allocate(
        profiles::file_size, "save driver profile on name", lifecycle::ui_pool));
    if (profile) {
        try {
            profiles::construct_profile(profile);
        } catch (...) {
            gui::application_release(profile);
            throw;
        }
        profiles::capture_profile(profile, 0);
        saved = profiles::save_profile(profile, filename);
        profiles::destroy_profile(profile);
        gui::application_release(profile);
    }
    files::set_directory(saved_directory);
    return saved;
}

void save_current_controller_profile() {
    gui::String name;
    pending::copy_preference_name(bytes(system(72)) + 2488, &name);
    struct NameOwner {
        gui::String *name;
        ~NameOwner() {
            gui::string_destruct(name);
        }
    } owner{&name};
    pending::LocalFrontendString filename(&name);
    gui::string_append_ascii(&filename.value, ".sav");
    save_named_profile(gui::string_ascii(&filename.value));
}

Dword enumerate_page_profiles(const char *pattern, ProfileToggle *toggle) {
    char root[260], saved_directory[260];
    std::sprintf(root, "%s", graphics::startup::configuration_path());
    files::get_directory(saved_directory);
    files::set_directory(root);
    Dword count = 0;
    files::DirectorySearch search;
    search.attributes = 160;
    if (files::find_first(pattern, &search)) {
        do {
            if (search.attributes & 160) {
                ++count;
                if (toggle) {
                    pending::LocalFrontendString filename(search.name), stem("");
                    gui::string_substring(&filename.value, 0, filename.value.length - 4,
                                          &stem.value);
                    family::ListItem item;
                    construct_toggle_item(&item, &stem.value, 0);
                    try {
                        invoke(&toggle->items, 8, &item);
                    } catch (...) {
                        gui::string_destruct(&item.tooltip);
                        gui::string_destruct(&item.text);
                        throw;
                    }
                    gui::string_destruct(&item.tooltip);
                    gui::string_destruct(&item.text);
                    profiles::select_profile_row(toggle, 0);
                }
            }
        } while (files::find_next(&search));
        files::find_close(&search);
    }
    files::set_directory(saved_directory);
    return count;
}

Dword delete_profile_files(const char *pattern) {
    char root[260], saved_directory[260];
    std::sprintf(root, "%s", graphics::startup::configuration_path());
    files::get_directory(saved_directory);
    files::set_directory(root);
    Dword count = 0;
    files::DirectorySearch search;
    search.attributes = 160;
    if (files::find_first(pattern, &search)) {
        do {
            if (search.attributes & 160) {
                ++count;
                files::erase(pattern, 0, 0);
            }
        } while (files::find_next(&search));
        files::find_close(&search);
    }
    files::set_directory(saved_directory);
    return count;
}
} // namespace nfs::application::frontend
