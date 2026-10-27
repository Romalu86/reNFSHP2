#include "../../frontend/flow/frontend_simulation.hpp"
#include "network_lifecycle_runtime.hpp"
#include "nfs/gui/lifecycle/gui_modal.hpp"
namespace nfs::network::session {
using namespace application::frontend;
using namespace gui::widgets;
using application::pending::LocalFrontendString;
namespace {
Widget *create_dialog(const char *type, const char *name, void *parent) {
    LocalFrontendString widget_type(type), widget_name(name), resource("system\\GDialogs.gui");
    auto *dialog = static_cast<Widget *>(application::pending::create_widget(
        &resource.value, &widget_name.value, &widget_type.value, parent));
    return dialog && method<Dword>(dialog, 2, family::dialog_class_key) ? dialog : nullptr;
}
Widget *typed_child(void *parent, const char *name, TypeName key) {
    auto *child = static_cast<Widget *>(application::pending::find_child(parent, name));
    return child && method<Dword>(child, 2, key) ? child : nullptr;
}
void repost(Dword event) {
    gui::events::propagate(at<void *>(system(96), 40), event, 0, 0, 2);
}
void close_dialog(Widget *dialog) {
    application::pending::destroy_screen(dialog);
}
} // namespace

void show_client_car_dialog(void *) {
    if (!system(96) || game::phase_locked)
        return;
    if (auto *focused = static_cast<Widget *>(gui::focused_widget())) {
        if (method<Dword>(focused, 3, "NetOptions")) {
            family::finish_modal(focused, 2, 0);
            repost(111);
            return;
        }
        if (game::runtime::reset_requested)
            return;
    }
    application::show_cursor(1);
    game::phase_locked = 1;
    game::runtime::reset_requested = 1;
    auto *dialog = create_dialog("ClientCarSelect", "<CARSEL>", at<void *>(system(96), 40));
    int result = 0;
    if (dialog) {
        result = gui::run_modal(dialog, nullptr);
        close_dialog(dialog);
    }
    game::runtime::reset_requested = 0;
    game::phase_locked = 0;
    if (result == 3)
        repost(113);
}

void show_network_statistics(void *owner) {
    if (at<Dword>(owner, 36) || !system(96))
        return;
    void *parent = at<void *>(owner, 92);
    if (!parent)
        parent = at<void *>(system(96), 40);
    if (auto *focused = static_cast<Widget *>(gui::focused_widget())) {
        const bool options = method<Dword>(focused, 3, "NetOptions") != 0;
        if (options || (!at<Dword>(owner, 36) && game::runtime::reset_requested)) {
            family::finish_modal(focused, options ? 2 : 12, 0);
            repost(112);
            return;
        }
    }
    auto *dialog = create_dialog("NetStats", "<STATS>", parent);
    if (dialog) {
        at<Dword>(owner, 36) = 1;
        gui::run_modal(dialog, nullptr);
        close_dialog(dialog);
        at<Dword>(owner, 36) = 0;
    }
}

void show_network_options(void *) {
    if (game::runtime::reset_requested || !system(96))
        return;
    game::runtime::reset_requested = 1;
    auto *dialog = create_dialog("NetOptions", "<OPTIONS>", at<void *>(system(96), 40));
    if (dialog) {
        gui::run_modal(dialog, nullptr);
        close_dialog(dialog);
    }
    game::runtime::reset_requested = 0;
}

void show_server_setup(void *) {
    game::runtime::reset_requested = 1;
    auto *dialog = create_dialog("ServerSetup", "<SERVER>", at<void *>(system(96), 40));
    // These saved type probes intentionally do not install replacement callbacks.
    for (const char *name : {"<ACCEPT>", "<CANCEL>"})
        if (void *button = application::pending::find_child(dialog, name))
            method<Dword>(button, 2, family::button_class_key);
    Widget *dedicated = typed_child(dialog, "<DEDICATED>", family::button_class_key);
    auto *track = reinterpret_cast<ProfileToggle *>(typed_child(dialog, "<TRACK>", profiles::profile_list_class_key));
    auto *direction = reinterpret_cast<ProfileToggle *>(typed_child(dialog, "<DIRECTION>", profiles::profile_list_class_key));
    Widget *laps = typed_child(dialog, "<LAPS>", family::spin_button_class_key);
    auto *maximum_clients = reinterpret_cast<ProfileToggle *>(typed_child(dialog, "<TMAXCLIENTS>", profiles::profile_list_class_key));
    auto *race_type = reinterpret_cast<ProfileToggle *>(typed_child(dialog, "<RACETYPE>", profiles::profile_list_class_key));
    auto *car_class = reinterpret_cast<ProfileToggle *>(typed_child(dialog, "<CLASS>", profiles::profile_list_class_key));
    Widget *server_name = typed_child(dialog, "<SERVERNAME>", family::edit_text_class_key);
    Widget *password = typed_child(dialog, "<PASSWORD>", family::edit_text_class_key);
    if (maximum_clients)
        set_enabled(reinterpret_cast<Widget *>(maximum_clients), 0);
    if (dedicated) {
        set_enabled(dedicated, 0);
        application::pending::set_widget_visible(dedicated, 0);
    }
    void *server = system(108);
    if (race_type)
        select_toggle_value(race_type, at<Dword>(server, 56));
    if (car_class)
        select_toggle_value(car_class, at<Dword>(server, 60));
    if (direction)
        select_toggle_value(direction, at<Dword>(server, 68));
    if (laps)
        family::set_spin_button_value(laps, at<int>(server, 72));
    if (maximum_clients)
        profiles::select_profile_row(maximum_clients,
            Dword(find_toggle_value(maximum_clients, at<Byte>(server, 96))));
    if (server_name) {
        LocalFrontendString text(static_cast<const char *>(server) + 172);
        method<void>(server_name, 7, &text.value);
    }
    if (track)
        select_toggle_value(track, at<Dword>(server, 64));
    if (password) {
        LocalFrontendString text(static_cast<const char *>(server) + 196);
        method<void>(password, 7, &text.value);
    }
    if (dialog) {
        const int result = gui::run_modal(dialog, nullptr);
        close_dialog(dialog);
        game::runtime::reset_requested = 0;
        if (result == 3)
            repost(113);
    }
}

void show_network_audio_options(void *) {
    game::runtime::reset_requested = 1;
    auto *dialog = create_dialog("GDialog", "<AUDIO_OPTIONS>", at<void *>(system(96), 40));
    if (dialog) {
        const int result = gui::run_modal(dialog, nullptr);
        close_dialog(dialog);
        game::audio::restore_music_volume();
        game::runtime::reset_requested = 0;
        if (result != 12)
            repost(113);
    }
}

void show_network_hud_options(void *) {
    game::runtime::reset_requested = 1;
    auto *dialog = create_dialog("GDialog", "<HUD_OPTIONS>", at<void *>(system(96), 40));
    if (dialog) {
        const int result = gui::run_modal(dialog, nullptr);
        close_dialog(dialog);
        game::runtime::reset_requested = 0;
        if (result != 12)
            repost(113);
    }
}
} // namespace nfs::network::session
