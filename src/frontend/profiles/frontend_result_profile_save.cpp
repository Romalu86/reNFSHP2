#include "../flow/frontend_race_results.hpp"
#include "../../profiles/storage/player_profile.hpp"
#include "nfs/resources/files/directory_search.hpp"
#include "nfs/network/packets/network_packet_handlers.hpp"
#include <cstdio>
namespace nfs::application::pending {
void release_result_allocation(void *);
// The callers pass a widget; this body does not read it.
Byte save_finished_race_profile(gui::widgets::Widget *) {
    if (!network::packets::rule_value(static_cast<Dword *>(system(72)), 43)) {
        LocalFrontendString text;
        gui::string_reference(&text.value, 1420);
        ScopedMessageBox box(&text.value, 66);
        if (run_message_box(&box.value, nullptr, 0) != 2) return 1;
    }
    gui::String name;
    copy_preference_name(bytes(system(72)) + 2488, &name);
    struct NameOwner {
        gui::String *value;
        ~NameOwner() { gui::string_destruct(value); }
    } name_owner{&name};
    LocalFrontendString filename(&name);
    gui::string_append_ascii(&filename.value, ".sav");
    const char *file = gui::string_ascii(&filename.value);
    char root[332], saved_directory[260];
    std::sprintf(root, "%s", graphics::startup::configuration_path());
    files::get_directory(saved_directory);
    files::set_directory(root);
    auto *profile = static_cast<profiles::SavedProfile *>(lifecycle::allocate(
        50272, "auto_save_profile", lifecycle::ui_pool));
    if (profile) {
        try { profiles::construct_profile(profile); }
        catch (...) { release_result_allocation(profile); throw; }
        profiles::capture_profile(profile, 0);
        const Byte saved = profiles::save_profile(profile, file);
        files::set_directory(saved_directory);
        profiles::destroy_profile(profile);
        release_result_allocation(profile);
        if (saved) {
            LocalFrontendString format;
            gui::string_reference(&format.value, 1398);
            LocalFrontendString text;
            gui::string_format(&text.value, &format.value, gui::string_data(&name));
            MessageBox box;
            construct_message_box(&box, &text.value);
            try { run_message_box(&box, nullptr, 0); }
            catch (...) { destroy_message_box(&box); throw; }
            destroy_message_box(&box);
            return 1;
        }
    }
    LocalFrontendString format;
    gui::string_reference(&format.value, 1407);
    LocalFrontendString text;
    gui::string_format(&text.value, &format.value, gui::string_data(&name));
    ScopedMessageBox box(&text.value, 8449);
    run_message_box(&box.value, nullptr, 0);
    return 0;
}
}
