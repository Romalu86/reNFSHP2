#pragma once
#include "../network/frontend_network_pages.hpp"
#include "../flow/frontend_pick_list.hpp"
#include "../../gui/widgets/gui_widget_combo.hpp"
namespace nfs::application::frontend {
struct LobbyScreen {
    Widget base;
    gui::widgets::family::ListBox *players, *messages;
    Widget *chat;
};
struct ReplayScreen {
    Widget base;
    Widget *view, *remove, *replays;
};
static_assert(sizeof(LobbyScreen) == 268 && sizeof(ReplayScreen) == 268);
extern const std::array<Method, 42> lobby_screen_methods, replay_screen_methods;
const char *__cdecl lobby_screen_class_key();
const char *__cdecl replay_screen_class_key();
void __fastcall initialize_lobby_screen(LobbyScreen *, void *);
void __fastcall lobby_screen_child(LobbyScreen *, void *, Widget *, Dword, Dword, Dword);
void __fastcall initialize_replay_screen(ReplayScreen *, void *);
void __fastcall replay_screen_child(ReplayScreen *, void *, Widget *, Dword, Dword, Dword);
} // namespace nfs::application::frontend
