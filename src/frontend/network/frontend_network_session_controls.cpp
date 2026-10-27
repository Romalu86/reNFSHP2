#include "frontend_network_session.hpp"
#include <mmsystem.h>
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::at;
using game::runtime::invoke;
using game::runtime::system;

void refresh_network_ping(NetworkSessionWidget *widget) {
    if (!widget->ping)
        return;
    Dword colour;
    if (at<Dword>(network::server(), 44))
        colour = 4278255360u;
    else {
        auto *client = network::client();
        if (!at<Dword>(client, 68))
            return;
        const auto phase = at<std::uint16_t>(client, 380);
        if (phase == 1 || phase == 5)
            colour = 2147418112;
        else if (at<Dword>(client, 428) > 500)
            colour = 4294901760u;
        else if (at<Dword>(client, 428) > 250)
            colour = 4294967040u;
        else
            colour = 4278255360u;
    }
    widget->ping->colour = colour;
}

void refresh_network_session_controls(NetworkSessionWidget *widget) {
    show_cursor(game::phase_reset || game::runtime::reset_requested ||
                game::runtime::reset_complete);
    const auto phase = at<std::int16_t>(network::network_game(), 8);
    const Dword status_visible =
        phase != 0 && phase != 1 && !(phase == 5 && at<int>(system(72), 204) != -1);
    if (auto *mode = widget->mode) {
        Dword visible = 1;
        if (!(at<Dword>(network::server(), 44) && at<Dword>(network::server(), 48)) &&
            widget->mode_expiry <= timeGetTime())
            visible = status_visible;
        set_enabled(mode, visible);
        pending::set_widget_visible(mode, visible);
    }
    pending::set_letterbox_visible(phase != 5, at<void *>(system(96), 40));
    auto *message_log = widget->message_log;
    if (message_log) {
        if (auto *command = widget->command; command && widget->message_background) {
            const Dword visible = game::runtime::reset_complete;
            if (visible) {
                game::phase_reset = 0;
                widget->chat_was_visible = 0;
            }
            const Dword was_visible = ~command->flags & 1;
            set_enabled(command, visible);
            pending::set_widget_visible(widget->command, visible);
            if (visible != was_visible)
                activate(widget->command);
            set_enabled(&message_log->base.base, visible);
            pending::set_widget_visible(message_log, visible);
            pending::set_widget_visible(widget->message_background, visible);
            if (!visible) {
                const int rows = int(message_log->items.grid.rows.count);
                if (rows > 0)
                    family::ensure_list_box_row_visible(&message_log->base.base, rows - 1);
            } else if (!was_visible) {
                family::move_to_first_draw_position(widget->message_background);
                family::move_to_first_draw_position(&message_log->base.base);
                family::move_to_first_draw_position(widget->command);
            }
        }
    }
    if (auto *chat = widget->chat) {
        const auto visible = game::phase_reset;
        set_enabled(chat, visible);
        pending::set_widget_visible(chat, visible);
        if (game::phase_reset != widget->chat_was_visible) {
            if (game::phase_reset)
                activate(chat);
            widget->chat_was_visible = game::phase_reset;
        }
    }
    if (auto *history = widget->chat_history) {
        set_enabled(&history->base.base, 1);
        pending::set_widget_visible(history, 1);
        family::ensure_list_box_row_visible(&history->base.base, 0);
    }
    if (auto *message = widget->system_message) {
        set_enabled(message, 1);
        pending::set_widget_visible(message, 1);
    }
}

void __fastcall tick_network_session(NetworkSessionWidget *widget, void *, Dword) {
    const Dword enabled = at<Dword>(network::server(), 44) || at<Dword>(network::client(), 68);
    set_enabled(&widget->base.base, enabled);
    pending::set_widget_visible(widget, enabled);
    if (!enabled)
        return;
    refresh_network_session_controls(widget);
    refresh_network_session_messages(widget);
    refresh_network_ping(widget);
    char command[256];
    if (std::uint8_t(network::ui::console_available()) && at<Dword>(network::server(), 44) &&
        network::ui::read_console_input(command, sizeof(command)) &&
        !execute_network_command(widget, command)) {
        network::packets::broadcast_chat(network::server(), command, nullptr);
        pending::LocalFrontendString prefix("server message:  ");
        gui::String line;
        gui::append_ascii_value(&line, &prefix.value, command);
        try {
            network::ui::console_print(gui::string_ascii(&line));
        } catch (...) {
            gui::string_destruct(&line);
            throw;
        }
        gui::string_destruct(&line);
    }
}

Dword __fastcall network_session_key(NetworkSessionWidget *widget, void *, Dword key,
                                    Dword modifiers) {
    if (!std::uint8_t(modifiers) && key == 15) {
        if (!game::phase_pending)
            gui::events::propagate(widget, 112, 0, 0, 2);
        game::phase_pending = 1;
    }
    return gui::widgets::pending::general_key(&widget->base, nullptr, key, modifiers);
}

Dword __cdecl filter_network_console_key(void *, Dword, Dword key, Dword) {
    return key == 126 || key == 96;
}

Dword __cdecl network_edit_event(void *owner, Dword event, Dword, Dword) {
    auto *edit = static_cast<Widget *>(owner);
    auto *session = reinterpret_cast<NetworkSessionWidget *>(edit);
    if (edit->parent)
        session = invoke<Dword>(edit->parent, 2, pending::netgame_frontend_class_key)
                      ? reinterpret_cast<NetworkSessionWidget *>(edit->parent)
                      : nullptr;
    switch (event) {
    case 15:
        if (!game::phase_reset)
            invoke(edit, 7, &lifecycle::construction::unresolved::shared_string);
        break;
    case 18:
        for (Dword code : {14u, 15u, 40u, 41u})
            pending::set_widget_callback(edit, code, network_edit_event);
        break;
    case 40:
        if (session)
            submit_network_session_text(session);
        break;
    case 41:
        invoke(edit, 7, &lifecycle::construction::unresolved::shared_string);
        break;
    }
    return 0;
}

void submit_network_session_text(NetworkSessionWidget *widget) {
    pending::LocalFrontendString text;
    int mute_log = 0;
    if (!game::runtime::reset_complete) {
        gui::string_copy(&text.value, invoke<const gui::String *>(widget->chat, 8));
        if (text.value.wide)
            for (int index = 0; index < text.value.length; ++index)
                if (network::packets::string_character(&text.value, index) & 65280) {
                    mute_log = 1;
                    break;
                }
    } else
        gui::string_copy(&text.value, invoke<const gui::String *>(widget->command, 8));
    pending::LocalFrontendString narrow(&text.value);
    gui::string_make_narrow(&narrow.value);
    const char *message = gui::string_ascii(&narrow.value);
    if (text.value.wide) {
        const int sender =
            at<Dword>(network::client(), 68) ? at<std::int16_t>(network::client(), 376) : -1;
        session_notice(widget, &text.value, sender, mute_log);
        if (at<Dword>(network::client(), 68))
            network::send_client_wide_chat(
                network::client(),
                static_cast<const wchar_t *>(text.value.wide ? gui::string_data(&text.value)
                                                             : nullptr),
                text.value.length);
    } else if (!execute_network_command(widget, message)) {
        const int sender =
            at<Dword>(network::client(), 68) ? at<std::int16_t>(network::client(), 376) : -1;
        session_notice(widget, &text.value, sender, mute_log);
        if (at<Dword>(network::client(), 68) && message)
            network::send_client_chat(network::client(), message);
        else if (at<Dword>(network::server(), 44) && message)
            network::packets::broadcast_chat(network::server(), message, nullptr);
    }
    auto *empty = &lifecycle::construction::unresolved::shared_string;
    if (!game::runtime::reset_complete) {
        invoke(widget->chat, 7, empty);
        game::phase_reset = 0;
    } else {
        invoke(widget->command, 7, empty);
        if (widget->message_log && int(widget->message_log->items.grid.rows.count) > 0)
            family::ensure_list_box_row_visible(&widget->message_log->base.base,
                                                int(widget->message_log->items.grid.rows.count) -
                                                    1);
        activate(widget->command);
    }
}
} // namespace nfs::application::frontend
