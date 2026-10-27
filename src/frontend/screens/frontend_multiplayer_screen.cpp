#include "../flow/frontend_multiplayer.hpp"
#include "nfs/core/services/service_dependencies.hpp"
#include "nfs/gui/lifecycle/gui_modal.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include "../../network/session/server_runtime.hpp"
#include "../../profiles/storage/player_profile.hpp"
namespace nfs::game::runtime::ai::cars {
gui::String *profile_display_name(void *, gui::String *);
}
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::at;
using game::runtime::invoke;
using game::runtime::system;
MultiplayerScreen *multiplayer_screen{};

MultiplayerScreen *construct_multiplayer(MultiplayerScreen *screen, void *parent) {
    gui::widgets::pending::construct_interface(screen, parent);
    screen->base.methods = multiplayer_methods.data();
    screen->mode = nullptr;
    screen->back = screen->name = screen->server_count = screen->retest = screen->message = nullptr;
    screen->selected_mode = -1;
    screen->page = nullptr;
    multiplayer_screen = screen;
    return screen;
}

void destroy_multiplayer(MultiplayerScreen *screen) {
    screen->base.methods = multiplayer_methods.data();
    multiplayer_screen = nullptr;
    destroy_interface(&screen->base);
}

void __fastcall initialize_multiplayer(MultiplayerScreen *screen, void *) {
    gui::widgets::pending::request_interface_layout(&screen->base, nullptr);
    network::lobby::initialize();
    screen->mode = reinterpret_cast<TypeSpinControl *>(
        network_page_child(screen, "<SPIN>", type_spin_class_key));
    network::lobby::set_mode(int(services::pending::network_kind));
    network::packets::pending::set_game_rule(static_cast<Dword *>(system(72)), 62,
                                                 Dword(network::packets::pending::session_kind));
    screen->retest = network_page_child(screen, "<RETEST>", family::button_class_key);
    if (screen->retest) {
        set_enabled(screen->retest, 0);
        pending::set_widget_visible(screen->retest, 0);
    }
    screen->back = network_page_child(screen, "<BACK>", family::button_class_key);
    screen->name = network_page_child(screen, "<NAME>", family::edit_text_class_key);
    if (screen->name) {
        services::pending::configure_client(system(112), 0);
        bool empty;
        {
            pending::LocalFrontendString current;
            gui::copy_member_string(system(112), &current.value);
            empty = gui::string_equal(&current.value,
                                      &lifecycle::construction::unresolved::shared_string);
        }
        pending::LocalFrontendString name;
        if (!empty)
            gui::copy_member_string(system(112), &name.value);
        else
            game::runtime::ai::cars::profile_display_name(
                pending::player_preferences(system(72), 0), &name.value);
        invoke(screen->name, 7, &name.value);
    }
    screen->server_count = static_cast<Widget *>(pending::find_child(screen, "<NUMSERVERS>"));
    screen->message = network_page_child(screen, "<SYSMSG>", family::text_class_key);
    if (screen->server_count) {
        pending::LocalFrontendString count("0");
        invoke(screen->server_count, 7, &count.value);
        pending::set_widget_visible(screen->server_count, 0);
    }
    if (screen->message)
        invoke(screen->message, 7, &lifecycle::construction::unresolved::shared_string);
    if (void *version = pending::find_child(screen, "<version>")) {
        pending::LocalFrontendString text;
        gui::string_reference(&text.value, 1720);
        gui::string_format(&text.value, &text.value, Dword(242));
        invoke(version, 7, &text.value);
    }
    change_multiplayer_page(screen);
}

BrowserView *current_browser(MultiplayerScreen *screen) {
    Widget *page = screen->page;
    return page && invoke<Dword>(page, 3, "BrowserView") &&
                   invoke<Dword>(page, 2, browser_view_class_key)
               ? reinterpret_cast<BrowserView *>(page)
               : nullptr;
}

void __fastcall tick_multiplayer(MultiplayerScreen *screen, void *, Dword) {
    if (screen->selected_mode !=
        int(network::packets::rule_value(static_cast<Dword *>(system(72)), 62)))
        change_multiplayer_page(screen);
    if (screen->page && screen->server_count) {
        if (invoke<Dword>(screen->page, 3, "BrowserView")) {
            BrowserView *page = invoke<Dword>(screen->page, 2, browser_view_class_key)
                                    ? reinterpret_cast<BrowserView *>(screen->page)
                                    : nullptr;
            pending::LocalFrontendString count(
                &lifecycle::construction::unresolved::shared_string);
            gui::string_format_ascii(&count.value, "%d",
                                     page->servers ? page->servers->items.grid.rows.count : 0);
            invoke(screen->server_count, 7, &count.value);
            pending::set_widget_visible(screen->server_count, 1);
        } else
            pending::set_widget_visible(screen->server_count, 0);
    }
    if (screen->retest) {
        const Dword enabled = network::packets::pending::session_kind == 1 ||
                             network::packets::pending::session_kind == 2;
        set_enabled(screen->retest, enabled);
        pending::set_widget_visible(screen->retest, enabled);
    }
    if (!screen->message)
        return;
    pending::LocalFrontendString text(&lifecycle::construction::unresolved::shared_string);
    gui::string_format_ascii(&text.value, "");
    const int mode = network::packets::pending::session_kind;
    if (mode == 1 || mode == 2) {
        if (pending::server_password_enabled) {
            pending::LocalFrontendString format;
            gui::string_reference(&format.value, 1605);
            gui::string_format(&text.value, &format.value, pending::server_password_enabled);
        } else if (network::bandwidth::download()) {
            pending::LocalFrontendString format;
            gui::string_reference(&format.value, 1606);
            const Dword upload = network::bandwidth::upload(),
                       download = network::bandwidth::download();
            gui::string_format(&text.value, &format.value, download, upload);
        } else if (network::bandwidth::probe_phase == 2)
            gui::string_copy(&text.value,
                             &lifecycle::construction::unresolved::shared_string);
        else {
            pending::LocalFrontendString format;
            gui::string_reference(&format.value, 1607);
            gui::string_format(&text.value, &format.value);
        }
    }
    invoke(screen->message, 7, &text.value);
}

void __fastcall leave_multiplayer(MultiplayerScreen *screen, void *) {
    if (screen->name) {
        pending::LocalFrontendString name(invoke<const gui::String *>(screen->name, 8));
        network::lobby::apply_client_name(network::lobby::client_owner(), name.release());
    }
    if (!pending::screen_queue_empty(system(96))) {
        pending::LocalFrontendString profile;
        pending::copy_preference_name(pending::player_preferences(system(72), 0), &profile.value);
        void *rules = system(72);
        pending::LocalFrontendString narrow(&profile.value);
        if (gui::string_make_narrow(&narrow.value) &&
            gui::engine::pending::character_at(&narrow.value, 0)) {
            std::strncpy(reinterpret_cast<char *>(game::runtime::bytes(rules) + 2514),
                         gui::string_ascii(&narrow.value), 25);
            at<std::uint8_t>(rules, 2513) = 0;
        } else
            at<std::uint8_t>(rules, 2514) = 0;
    }
    network::lobby::destroy();
    gui::widgets::clear_layout_request(&screen->base, nullptr);
}

Dword __fastcall close_multiplayer(MultiplayerScreen *screen, void *, Dword result, Dword) {
    screen->base.modal_result = result;
    return 1;
}

void __fastcall multiplayer_child(MultiplayerScreen *screen, void *, Widget *sender, Dword event,
                                  Dword, Dword) {
    if (event == 43) {
        Widget *button =
            sender && invoke<Dword>(sender, 2, family::button_class_key) ? sender : nullptr;
        if (button == screen->back) {
            family::finish_modal(&screen->base, 19, 0);
            return;
        }
        if (button == screen->retest && !pending::server_password_enabled)
            network::bandwidth::retest();
    } else if (event == 64) {
        Widget *edit =
            sender && invoke<Dword>(sender, 2, family::edit_text_class_key) ? sender : nullptr;
        if (edit == screen->name) {
            pending::LocalFrontendString text(invoke<const gui::String *>(edit, 8));
            pending::LocalFrontendString copy(&text.value);
            network::lobby::apply_client_name(network::lobby::client_owner(), copy.release());
        }
    }
}
} // namespace nfs::application::frontend
