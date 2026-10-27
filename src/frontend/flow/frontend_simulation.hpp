#pragma once
#include "../screens/frontend_race_dialogs.hpp"
#include "../../gui/widgets/gui_widget_shared_methods.hpp"
#include "nfs/ai/lifecycle/ai_lifecycle.hpp"
#include "nfs/audio/music/music_clients.hpp"
namespace nfs::gui {
bool string_equal(const String *, const String *);
}
namespace nfs::application::pending {
void *create_widget(const gui::String *, const gui::String *, const gui::String *, void *);
void configure_player_widgets(void *, Dword, void *);
void set_menu_camera(Byte);
void pause_race_for_menu();
void resume_race_from_menu(Byte);
}
namespace nfs::network::session {
void show_client_car_dialog(void *);
void show_network_statistics(void *);
void show_network_options(void *);
void show_server_setup(void *);
void show_network_hud_options(void *);
void show_network_audio_options(void *);
}
namespace nfs::application::frontend {
struct PauseDialog {
    Widget base;
    alignas(4) Byte dialog_state[32];
    gui::String parent_menu;
    Dword saved_settings[43];
};
struct CountdownWidget {
    GeneralWidget base;
    Widget *text;
    int phase;
};
struct ReplayControls {
    Widget base;
    alignas(4) Byte dialog_state[32];
    Widget *pause_play, *speed_label, *eject, *faster, *slower;
    int speed;
};
static_assert(sizeof(PauseDialog) == 480 && offsetof(PauseDialog, saved_settings) == 308);
static_assert(sizeof(CountdownWidget) == 272 && offsetof(CountdownWidget, text) == 264);
static_assert(sizeof(ReplayControls) == 312 && offsetof(ReplayControls, speed) == 308);
extern const std::array<Method, 42> simulation_methods;
extern const std::array<Method, 49> pause_dialog_methods;
extern const std::array<Method, 49> end_game_methods;
extern const std::array<Method, 49> replay_controls_methods;
extern const std::array<Method, 47> countdown_methods;
const char *__cdecl simulation_class_key();
const char *__cdecl pause_dialog_class_key();
const char *__cdecl countdown_class_key();
const char *__cdecl end_game_class_key();
const char *__cdecl replay_controls_class_key();
void *create_simulation_screen(void *);
void *create_pause_dialog(void *);
void *create_countdown_widget(void *);
void *create_end_game_dialog(void *);
void *create_replay_controls(void *);
void __fastcall initialize_replay_controls(ReplayControls *, void *);
void __fastcall release_replay_controls(ReplayControls *, void *);
void __fastcall replay_controls_child_event(ReplayControls *, void *, Widget *, Dword, Dword, Dword);
Dword __fastcall replay_controls_key(ReplayControls *, void *, Dword, Dword);
void __fastcall tick_replay_controls(ReplayControls *, void *, Dword);
void __fastcall draw_replay_controls(ReplayControls *, void *, void *);
void __fastcall initialize_end_game(Widget *, void *);
void __fastcall release_end_game(Widget *, void *);
void __fastcall tick_end_game(Widget *, void *, Dword);
void __fastcall draw_end_game(Widget *, void *, void *);
void __fastcall initialize_pause_dialog(PauseDialog *, void *);
void __fastcall release_pause_dialog(PauseDialog *, void *);
Dword __fastcall finish_pause_dialog(PauseDialog *, void *, Dword, Dword);
Dword __fastcall pause_dialog_load(PauseDialog *, void *, void *);
Dword __fastcall pause_dialog_save(PauseDialog *, void *, void *);
Dword __fastcall pause_dialog_key_down(PauseDialog *, void *, Dword, Dword);
void __fastcall tick_pause_dialog(PauseDialog *, void *, Dword);
void __fastcall initialize_countdown(CountdownWidget *, void *);
void __fastcall tick_countdown(CountdownWidget *, void *, Dword);
int run_pause_overlay(void *, Widget *);
void begin_controller_menu(void *, Dword);
void finish_controller_menu(void *, Dword);
}
