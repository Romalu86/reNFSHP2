#include "frontend_race_results.hpp"
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::at;
using game::runtime::invoke;
using game::runtime::system;

void __fastcall race_results_child_event(RaceResults *screen, void *, Widget *sender,
                                         Dword event, Dword, Dword modifiers) {
    if (event != 43) return;
    auto *button = results_child(&screen->base, "CONTINUE", family::button_class_key);
    if (sender != button || !contains_modal(&screen->base) || Byte(modifiers) ||
        int(screen->elapsed) < gui::engine::seconds_to_ticks(.5f)) return;
    if (!screen->points_shown && show_points_breakdown(screen)) return;
    if (!screen->reward_shown && show_medal_reward(screen)) return;
    family::finish_modal(&screen->base, 22, 0);
}

Dword __cdecl race_results_next_button(void *sender, Dword event, Dword, Dword modifiers) {
    auto *widget = static_cast<Widget *>(sender);
    if (event == 18) {
        pending::set_widget_callback(widget, 43, race_results_next_button);
        pending::set_widget_callback(widget, 6, race_results_next_button);
        return 0;
    }
    if (event != 43 && (event != 6 || Byte(modifiers))) return 0;
    if (enabled_in_ancestors(widget)) {
        RaceResults *screen;
        {
            pending::LocalFrontendString type("HP2RaceResults");
            auto *parent = find_menu_ancestor(widget, &type.value,
                &lifecycle::construction::unresolved::shared_string, 0);
            screen = parent && invoke<Dword>(parent, 2, race_results_class_key)
                ? reinterpret_cast<RaceResults *>(parent) : nullptr;
        }
        if (screen && !screen->base.modal_result && contains_modal(&screen->base) &&
            int(screen->elapsed) >= gui::engine::seconds_to_ticks(.5f)) {
            if (!screen->points_shown && show_points_breakdown(screen)) return 1;
            if (!screen->reward_shown && show_medal_reward(screen)) return 1;
            family::finish_modal(&screen->base, 22, 0);
        }
    }
    return 1;
}

Dword __fastcall finish_race_results(RaceResults *screen, void *, Dword result, Dword) {
    if (result == 22 && contains_modal(&screen->base)) {
        results_page(&screen->base, "00Results", false);
        results_page(&screen->base, "01Reward", false);
        results_page(&screen->base, "02Points", false);
        pending::pause_race_for_menu();
        game::audio::stop_race_channels();
        const Byte saved = pending::save_finished_race_profile(&screen->base);
        pending::resume_race_from_menu(1);
        if (!saved) return 0;
    }
    screen->base.modal_result = result;
    return 1;
}

Dword __fastcall finish_cop_results(CopVsPerpResults *screen, void *, Dword result, Dword) {
    if (result == 22 && contains_modal(&screen->base)) {
        pending::pause_race_for_menu();
        game::audio::stop_race_channels();
        const Byte saved = pending::save_finished_race_profile(&screen->base);
        pending::resume_race_from_menu(1);
        if (!saved) return 0;
    }
    screen->base.modal_result = result;
    return 1;
}

Dword __fastcall cop_results_key(CopVsPerpResults *screen, void *, Dword key, Dword modifiers) {
    if (!Byte(modifiers) && key == 28) family::finish_modal(&screen->base, 22, 0);
    return 1;
}

void __fastcall tick_cop_results(CopVsPerpResults *screen, void *, Dword ticks) {
    family::dialog_tick(&screen->base, nullptr, ticks);
    pending::stop_force_feedback();
    if (!screen->base.modal_result) {
        if (at<Dword>(system(96), 36) == 23) {
            game::audio::stop_race_channels();
            family::finish_modal(&screen->base, at<Dword>(system(96), 36), 0);
        } else game::advance();
    }
}

void __fastcall draw_cop_results(CopVsPerpResults *, void *, void *engine) {
    invoke<void>(engine, 56, Dword(6), Dword(0), Dword(0));
    draw_race_frame();
    invoke<void>(engine, 56, Dword(5), Dword(0), Dword(0));
}
}
