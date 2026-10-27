#pragma once
#include "frontend_network_pages.hpp"
#include "../../network/session/network_chat_submission.hpp"
#include "nfs/core/frame/game_step.hpp"

namespace nfs::application::pending {
const char *__cdecl netgame_frontend_class_key();
void set_letterbox_visible(Dword, void *);
} // namespace nfs::application::pending
namespace nfs::application::frontend {
struct NetworkSessionWidget {
    GeneralWidget base;
    Widget *mode, *system_message, *chat, *ping, *command;
    gui::widgets::family::ListBox *chat_history, *message_log;
    Widget *message_background;
    std::uint8_t chat_was_visible;
    std::uint8_t padding[3];
    int consumed_messages;
    Dword next_message, chat_expiry, mode_expiry;
};
static_assert(sizeof(NetworkSessionWidget) == 316 &&
              offsetof(NetworkSessionWidget, mode) == 264 &&
              offsetof(NetworkSessionWidget, consumed_messages) == 300);
extern const std::array<Method, 47> network_session_methods;
void __fastcall initialize_network_session(NetworkSessionWidget *, void *);
void __fastcall tick_network_session(NetworkSessionWidget *, void *, Dword);
Dword __fastcall network_session_key(NetworkSessionWidget *, void *, Dword, Dword);
void refresh_network_session_controls(NetworkSessionWidget *);
void refresh_network_session_messages(NetworkSessionWidget *);
void refresh_network_ping(NetworkSessionWidget *);
void submit_network_session_text(NetworkSessionWidget *);
Dword execute_network_command(NetworkSessionWidget *, const char *);
void execute_server_setting(NetworkSessionWidget *, char *, char *);
char *parse_network_argument(char *, int *, const char *, const char *, const char *);
void format_network_mode(gui::String *);
Dword __cdecl network_edit_event(void *, Dword, Dword, Dword);
Dword __cdecl filter_network_console_key(void *, Dword, Dword, Dword);
inline void session_notice(NetworkSessionWidget *widget, const gui::String *text, int sender = -1,
                           int mute_log = 0, int mute_chat = 0) {
    network::ui::notice(reinterpret_cast<Dword *>(widget), nullptr, text, std::int16_t(sender), 0,
                        mute_log, mute_chat);
}
} // namespace nfs::application::frontend
namespace nfs::network {
Dword request_client_car(Dword *, const ui::CarSettings *);
void send_client_vote(Dword *, std::uint8_t, Dword);
void send_client_whisper(Dword *, const char *, std::uint16_t);
void send_client_rename(Dword *, const char *);
namespace packets::pending {
extern Dword command_name_comparer;
}
} // namespace nfs::network
