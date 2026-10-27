#pragma once
#include "frontend_simulation.hpp"
#include "frontend_toggle_widget.hpp"
#include "../../gui/widgets/gui_widget_list.hpp"
#include "nfs/gui/lifecycle/gui_modal.hpp"
namespace nfs::gui::engine {
int seconds_to_ticks(float);
}
namespace nfs::gui::input {
const void *language_text(int);
}
namespace nfs::application::pending {
gui::String *construct_integer_text(gui::String *, int);
Dword participant_award(const RaceParticipant *, Dword);
Dword result_has_event_bonus();
Byte save_finished_race_profile(gui::widgets::Widget *);
void *profile_event_record(void *, Dword, Dword);
Byte profile_event_medal(void *, int *);
Byte participant_is_cop(void *, Dword);
}
namespace nfs::profiles {
gui::widgets::family::ListItem *construct_profile_list_item(gui::widgets::family::ListItem *);
}
namespace nfs::gui::widgets::family {
ListItem *construct_text_list_item(ListItem *, const gui::String *);
}
namespace nfs::application::frontend {
struct RaceResults {
    Widget base;
    alignas(4) Byte dialog_state[32];
    Byte reward_shown, points_shown, reserved[2];
    gui::widgets::family::ListBox *results;
    Dword elapsed, timer, statistic;
};
struct CopVsPerpResults {
    Widget base;
    alignas(4) Byte dialog_state[32];
    gui::widgets::family::ListBox *results;
};
static_assert(sizeof(RaceResults) == 308 && offsetof(RaceResults, timer) == 300);
static_assert(sizeof(CopVsPerpResults) == 292 && offsetof(CopVsPerpResults, results) == 288);
extern const std::array<Method, 49> race_results_methods, cop_results_methods;
const char *__cdecl race_results_class_key();
const char *__cdecl cop_results_class_key();
void *create_race_results(void *);
void *create_cop_results(void *);
void __fastcall initialize_race_results(RaceResults *, void *);
void __fastcall initialize_cop_results(CopVsPerpResults *, void *);
void __fastcall race_results_child_event(RaceResults *, void *, Widget *, Dword, Dword, Dword);
Dword __fastcall race_results_event(RaceResults *, void *, Dword, Dword, Dword);
void __fastcall race_results_timer(RaceResults *, void *, Dword);
void __fastcall tick_race_results(RaceResults *, void *, Dword);
void __fastcall draw_race_results(RaceResults *, void *, void *);
Dword __fastcall finish_race_results(RaceResults *, void *, Dword, Dword);
Dword __fastcall race_results_key_up(RaceResults *, void *, Dword, Dword);
Dword __fastcall finish_cop_results(CopVsPerpResults *, void *, Dword, Dword);
Dword __fastcall cop_results_key(CopVsPerpResults *, void *, Dword, Dword);
void __fastcall tick_cop_results(CopVsPerpResults *, void *, Dword);
void __fastcall draw_cop_results(CopVsPerpResults *, void *, void *);
void refresh_result_statistic(RaceResults *);
Byte show_medal_reward(RaceResults *);
Byte show_points_breakdown(RaceResults *);
Dword __cdecl race_results_next_button(void *, Dword, Dword, Dword);
gui::String *participant_display_name(void *, gui::String *, Dword);
void participant_car_label(void *, Dword, gui::String *);
// Inline sequences shared by the saved results methods, without new UI policy.
inline Widget *results_child(Widget *screen, const char *name, const void *type) {
    auto *child = static_cast<Widget *>(pending::find_child(screen, name));
    return child && game::runtime::invoke<Dword>(child, 2, type) ? child : nullptr;
}
inline void results_page(Widget *screen, const char *name, bool enabled) {
    if (auto *page = static_cast<Widget *>(pending::find_child(screen, name))) {
        gui::widgets::set_enabled(page, enabled);
        pending::set_widget_visible(page, enabled);
        if (enabled)
            gui::widgets::activate_first_child(page);
    }
}
inline void result_integer(Widget *widget, Dword value) {
    gui::String text;
    pending::construct_integer_text(&text, int(value));
    try { game::runtime::invoke<void>(widget, 7, &text); }
    catch (...) { gui::string_destruct(&text); throw; }
    gui::string_destruct(&text);
}
struct ResultListItem {
    gui::widgets::family::ListItem value;
    ResultListItem() { profiles::construct_profile_list_item(&value); }
    explicit ResultListItem(const gui::String *text) { gui::widgets::family::construct_text_list_item(&value, text); }
    ~ResultListItem() {
        gui::string_destruct(&value.tooltip);
        gui::string_destruct(&value.text);
        value.methods = reinterpret_cast<const Method *>(gui::base_methods);
    }
};
}
