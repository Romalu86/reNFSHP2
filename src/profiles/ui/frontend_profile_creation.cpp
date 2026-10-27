#include "../storage/player_profile.hpp"
#include "frontend_profile_controls.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
#include <cstdio>
#include <cstring>
namespace nfs::gui {
int string_find_ascii_folded(const String *, const char *);
}
namespace nfs::profiles {

void assign_player_preferences(void *rules, const void *preferences, Dword player) {
    if (player < 2)
        copy_profile_dwords(bytes(rules) + 2488 + player * preferences_size, preferences,
                            preferences_size / sizeof(Dword));
}
namespace pending {

Byte create_profile(void *parent, gui::String *output) {
    gui::String current_name;
    copy_preference_name(bytes(system(72)) + 2488, &current_name);
    // copy_preference_name constructs the output, unlike string_copy.
    struct NameOwner {
        gui::String *value;
        ~NameOwner() {
            gui::string_destruct(value);
        }
    } name_owner{&current_name};
    LocalFrontendString filename(&current_name);
    for (;;) {
        {
            LocalFrontendString initial;
            gui::string_reference(&initial.value, 2064);
            LocalFrontendString name("CreateProfile"),
                title(&lifecycle::construction::unresolved::shared_string),
                text(&lifecycle::construction::unresolved::shared_string);
            if (prompt_text_entry(widget_root(static_cast<Widget *>(parent)), text.release(),
                                  title.release(), &current_name, &initial.value, 12,
                                  name.release(), 16384, nullptr, 0, 0) != 2)
                return 0;
        }
        if (!gui::string_make_narrow(&current_name))
            return 0;
        {
            gui::String with_extension;
            gui::append_ascii_value(&with_extension, &current_name, ".sav");
            gui::string_copy(&filename.value, &with_extension);
            gui::string_destruct(&with_extension);
        }
        const bool invalid =
            gui::string_equal_folded(&filename.value, "*.sav") || !current_name.length;
        const int slash = invalid ? -1 : gui::string_find_ascii_folded(&filename.value, "/");
        if (invalid || slash >= 0) {
            LocalFrontendString format, message;
            gui::string_reference(&format.value, invalid ? 1808 : 1744);
            gui::string_format(&message.value, &format.value, gui::string_data(&current_name));
            ScopedMessageBox information(&message.value, 8449);
            run_message_box(&information.value, nullptr, 0);
            continue;
        }
        const char *path = gui::string_ascii(&filename.value);
        char directory[408], saved_directory[260];
        std::sprintf(directory, "%s", graphics::startup::configuration_path());
        files::get_directory(saved_directory);
        files::set_directory(directory);
        void *preferences = bytes(system(72)) + 2488;
        set_preference_name(preferences, &current_name);
        set_preference_network_name(preferences, &current_name);
        auto *profile = static_cast<SavedProfile *>(
            lifecycle::allocate(file_size, "auto save new profile", lifecycle::ui_pool));
        if (profile) {
            try {
                profile = construct_profile(profile);
            } catch (...) {
                game::runtime::track::pending::release_named(
                    profile, "auto save new profile", lifecycle::ui_pool);
                throw;
            }
            set_preference_name(profile, &current_name);
            capture_profile(profile, 0);
            set_preference_name(profile, &current_name);
            set_preference_network_name(profile, &current_name);
            assign_player_preferences(system(72), profile, 0);
            const Byte saved = save_profile(profile, path);
            files::set_directory(saved_directory);
            delete_profile(profile, 1);
            if (saved) {
                LocalFrontendString format, message;
                gui::string_reference(&format.value, 1398);
                gui::string_format(&message.value, &format.value, gui::string_data(&current_name));
                ScopedMessageBox information(&message.value, 257);
                run_message_box(&information.value, nullptr, 0);
                gui::string_copy(output, &current_name);
                LocalFrontendString remembered(&current_name);
                application::frontend::remember_profile(remembered.release());
                return 1;
            }
        }
        // In the saved allocation-failure branch the directory is not restored.
        LocalFrontendString format, message;
        gui::string_reference(&format.value, 1407);
        gui::string_format(&message.value, &format.value, gui::string_data(&current_name));
        ScopedMessageBox information(&message.value, 8449);
        run_message_box(&information.value, nullptr, 0);
        return 0;
    }
}
} // namespace pending
} // namespace nfs::profiles
