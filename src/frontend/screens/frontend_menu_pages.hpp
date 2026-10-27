#pragma once
#include "frontend_menu_controls.hpp"
#include "frontend_dialogs.hpp"
#include "../../gui/widgets/gui_widget_interaction.hpp"
namespace nfs::application::pending {
void *active_event_progress(void *);
Dword event_active(void *);
bool event_complete_progress(void *);
void clear_event_progress(void *);
void set_round_rule_cache(int, Dword);
} // namespace nfs::application::pending
namespace nfs::application::frontend {
void pop_frontend_screen(void *);
struct FreeRunPage {
    GeneralWidget base;
    ProfileToggle *players, *traffic;
};
struct SingleRacePage {
    GeneralWidget base;
    ProfileToggle *players, *cops, *opponents, *opponent_cars, *skill, *traffic;
};
struct KnockoutPage {
    GeneralWidget base;
    ProfileToggle *players, *cops, *opponents, *opponent_cars, *skill, *traffic;
    int previous_cops, previous_opponents;
};
struct QuotaPage {
    GeneralWidget base;
    ProfileToggle *players, *skill, *traffic, *time_limit;
};
struct TournamentPage {
    GeneralWidget base;
    ProfileToggle *load_new, *players, *cops, *opponents, *opponent_cars, *skill, *traffic, *races;
};
struct AudioPage {
    GeneralWidget base;
    gui::widgets::family::SliderBar *menu_music, *race_music, *engine, *voice, *effects;
    Dword mode_widget;
    ProfileToggle *music_type;
    Dword initial_levels[5];
    int mode, playlist;
    std::uint8_t suppress_release_fade, padding[3];
};
struct GraphicsPage {
    GeneralWidget base;
    ProfileToggle *resolution, *colour_depth;
    Dword reserved_110;
    gui::widgets::family::SliderBar *gamma, *car_detail, *track_detail, *effects;
};
static_assert(sizeof(GraphicsPage) == 292);
const char *__cdecl graphics_page_class_key();
void __fastcall initialize_graphics_page(GraphicsPage *, void *);
void __fastcall refresh_graphics_page(GraphicsPage *, void *);
void __fastcall graphics_page_child_event(GraphicsPage *, void *, Widget *, Dword, Dword, Dword);
Dword __fastcall finish_graphics_page(GraphicsPage *, void *, Dword, Dword);
static_assert(sizeof(AudioPage) == 324 && offsetof(AudioPage, music_type) == 288);
const char *__cdecl audio_page_class_key();
void __fastcall initialize_audio_page(AudioPage *, void *);
void __fastcall update_audio_page(AudioPage *, void *, Dword);
void __fastcall release_audio_page(AudioPage *, void *);
Dword __fastcall audio_page_key(AudioPage *, void *, Dword, Dword);
static_assert(sizeof(FreeRunPage) == 272 && sizeof(SingleRacePage) == 288 &&
              sizeof(QuotaPage) == 280 && sizeof(TournamentPage) == 296);
static_assert(sizeof(KnockoutPage) == 296 && offsetof(KnockoutPage, previous_cops) == 288 &&
              offsetof(KnockoutPage, previous_opponents) == 292);
ProfileToggle *find_page_toggle(void *, const char *);
void synchronize_page_toggle(ProfileToggle *);
bool can_leave_challenge_view(ChallengeView *);
Widget *find_menu_path(Widget *, Dword, const char *, ...);
void queue_options_notice(const gui::String *);
const char *__cdecl free_run_class_key();
const char *__cdecl single_race_class_key();
const char *__cdecl knockout_class_key();
const char *__cdecl quota_class_key();
const char *__cdecl tournament_class_key();
void __fastcall initialize_free_run(FreeRunPage *, void *);
void __fastcall update_free_run(FreeRunPage *, void *, Dword);
void __fastcall free_run_child_event(FreeRunPage *, void *, Widget *, Dword, Dword, Dword);
void __fastcall initialize_single_race(SingleRacePage *, void *);
void __fastcall update_single_race(SingleRacePage *, void *, Dword);
void __fastcall single_race_child_event(SingleRacePage *, void *, Widget *, Dword, Dword, Dword);
void __fastcall initialize_knockout(KnockoutPage *, void *);
void refresh_knockout_choices(KnockoutPage *);
void __fastcall update_knockout(KnockoutPage *, void *, Dword);
void __fastcall knockout_child_event(KnockoutPage *, void *, Widget *, Dword, Dword, Dword);
void __fastcall initialize_quota(QuotaPage *, void *);
void __fastcall update_quota(QuotaPage *, void *, Dword);
void __fastcall quota_child_event(QuotaPage *, void *, Widget *, Dword, Dword, Dword);
void __fastcall initialize_tournament(TournamentPage *, void *);
void __fastcall update_tournament(TournamentPage *, void *, Dword);
Dword __fastcall tournament_event(TournamentPage *, void *, Dword, Dword, Dword);
void __fastcall tournament_child_event(TournamentPage *, void *, Widget *, Dword, Dword, Dword);
Dword __fastcall finish_tournament(TournamentPage *, void *, Dword, Dword);
} // namespace nfs::application::frontend
