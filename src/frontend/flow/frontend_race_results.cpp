#include "frontend_race_results.hpp"
#include "../screens/frontend_menu_pages.hpp"
#include <cstdio>
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::at;
using game::runtime::invoke;
using game::runtime::system;

void __fastcall initialize_race_results(RaceResults *screen, void *) {
    family::dialog_initialize(&screen->base, nullptr);
    pending::stop_force_feedback();
    at<Dword>(screen, 272) = 0;
    results_page(&screen->base, "00Results", true);
    results_page(&screen->base, "01Reward", false);
    results_page(&screen->base, "02Points", false);
    Widget *list = find_menu_path(&screen->base, 1, "00Results", Dword(1), "<RESULTS>", Dword(0));
    screen->results = list && invoke<Dword>(list, 2, family::list_box_class_key)
        ? reinterpret_cast<family::ListBox *>(list) : nullptr;
    if (screen->results) {
        void *race = at<void *>(system(92), 8);
        ResultListItem item;
        invoke<void>(race, 23);
        Dword slots[24];
        int count;
        invoke<void>(race, 36, slots, &count);
        for (int row = 0; row < count; ++row) {
            char number[32];
            std::sprintf(number, "%d", row + 1);
            {
                pending::LocalFrontendString position(number);
                gui::string_copy(&item.value.text, &position.value);
            }
            invoke<void>(&screen->results->items, 7, &item.value, row, Dword(0));
            gui::String name;
            participant_display_name(race, &name, slots[row]);
            try { gui::string_copy(&item.value.text, &name); }
            catch (...) { gui::string_destruct(&name); throw; }
            gui::string_destruct(&name);
            invoke<void>(&screen->results->items, 7, &item.value, row, Dword(1));
            pending::LocalFrontendString car;
            participant_car_label(race, slots[row], &car.value);
            gui::string_copy(&item.value.text, &car.value);
            invoke<void>(&screen->results->items, 7, &item.value, row, Dword(2));
        }
        refresh_result_statistic(screen);
    }
    Byte eligible = invoke<Byte>(at<void *>(system(92), 8), 35);
    if (at<int>(system(72), 176) > 1) eligible = 0;
    auto *record = pending::participant(at<void *>(system(92), 8), at<Dword>(system(72), 204));
    Widget *points = results_child(&screen->base, "<POINTS>", family::text_class_key);
    Widget *bonus = results_child(&screen->base, "<BONUS>", family::text_class_key);
    Widget *total = results_child(&screen->base, "<TOTAL>", family::text_class_key);
    Widget *total_label = results_child(&screen->base, "t3", base_class_name);
    Widget *bonus_label = results_child(&screen->base, "t2", base_class_name);
    Widget *points_label = results_child(&screen->base, "t1", base_class_name);
    if (!eligible) {
        if (points) pending::set_widget_visible(points, false);
        if (bonus) pending::set_widget_visible(bonus, false);
        if (total) pending::set_widget_visible(total, false);
        if (points_label) pending::set_widget_visible(points_label, false);
        if (bonus_label) pending::set_widget_visible(bonus_label, false);
        if (total_label) pending::set_widget_visible(total_label, false);
        return;
    }
    if (points_label) pending::set_widget_visible(points_label, true);
    if (bonus_label) pending::set_widget_visible(bonus_label, true);
    if (total_label) pending::set_widget_visible(total_label, true);
    if (points) {
        pending::set_widget_visible(points, true);
        Dword sum = 0;
        for (Dword award = 0; award < 5; ++award)
            if (award != 3) sum += pending::participant_award(record, award);
        pending::LocalFrontendString text;
        gui::String number;
        pending::construct_integer_text(&number, int(sum));
        try { gui::string_copy(&text.value, &number); }
        catch (...) { gui::string_destruct(&number); throw; }
        gui::string_destruct(&number);
        invoke<void>(points, 7, &text.value);
    }
    if (total) {
        pending::set_widget_visible(total, true);
        result_integer(total, at<Dword>(system(72), 9728));
    }
    if (!bonus) return;
    pending::set_widget_visible(bonus, true);
    if (Byte(pending::result_has_event_bonus()))
        result_integer(bonus, pending::participant_award(record, 3));
    else {
        pending::set_widget_visible(bonus, false);
        if (auto *label = results_child(&screen->base, "t2", family::text_class_key))
            pending::set_widget_visible(label, false);
    }
}

void refresh_result_statistic(RaceResults *screen) {
    screen->statistic = invoke<Dword>(at<void *>(system(92), 8), 43, screen->statistic);
    auto *heading = find_menu_path(&screen->base, 1, "00Results", Dword(1), "h4", Dword(0));
    if (heading && invoke<Dword>(heading, 2, family::text_class_key)) {
        pending::LocalFrontendString text;
        static const Dword labels[] = {468, 247, 1265, 1431, 465, 1375, 1391, 1390};
        if (screen->statistic < 8) gui::string_reference(&text.value, labels[screen->statistic]);
        else gui::string_assign(&text.value, "HP2_STAT");
        invoke<void>(heading, 7, &text.value);
    }
    if (screen->results) {
        void *race = at<void *>(system(92), 8);
        ResultListItem item;
        invoke<void>(race, 23);
        Dword slots[24];
        int count;
        invoke<void>(race, 36, slots, &count);
        for (int row = 0; row < count; ++row) {
            invoke<void>(race, 44, screen->statistic, slots[row], &item.value);
            invoke<void>(&screen->results->items, 7, &item.value, row, Dword(3));
        }
    }
}

Dword __fastcall race_results_event(RaceResults *screen, void *, Dword event, Dword first, Dword second) {
    return family::dialog_event(&screen->base, nullptr, event, first, second);
}

void __fastcall race_results_timer(RaceResults *screen, void *, Dword timer) {
    if (timer == screen->timer) refresh_result_statistic(screen);
}

Dword __fastcall race_results_key_up(RaceResults *, void *, Dword, Dword) { return 1; }

void __fastcall tick_race_results(RaceResults *screen, void *, Dword ticks) {
    family::dialog_tick(&screen->base, nullptr, ticks);
    pending::stop_force_feedback();
    screen->elapsed += ticks;
    if (!screen->base.modal_result) {
        if (at<Dword>(system(96), 36) == 23) {
            game::audio::stop_race_channels();
            family::finish_modal(&screen->base, at<Dword>(system(96), 36), 0);
        } else game::advance();
    }
}

void __fastcall draw_race_results(RaceResults *, void *, void *engine) {
    invoke<void>(engine, 56, Dword(6), Dword(0), Dword(0));
    draw_race_frame();
    invoke<void>(engine, 56, Dword(5), Dword(0), Dword(0));
}
}
