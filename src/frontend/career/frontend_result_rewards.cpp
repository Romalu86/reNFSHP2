#include "../flow/frontend_race_results.hpp"
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::at;
using game::runtime::bytes;
using game::runtime::invoke;
using game::runtime::system;

Byte show_medal_reward(RaceResults *screen) {
    void *rules = system(72);
    screen->reward_shown = 1;
    if (!Byte(services::special_race_mode(rules))) return 0;
    void *progress = pending::active_event_progress(rules);
    if (at<Dword>(rules, 152) == 2 && !pending::event_complete_progress(progress)) return 0;
    results_page(&screen->base, "00Results", false);
    results_page(&screen->base, "02Points", false);
    auto *page = static_cast<Widget *>(pending::find_child(&screen->base, "01Reward"));
    if (!page) return 0;
    set_enabled(page, true);
    pending::set_widget_visible(page, true);
    activate_first_child(page);
    const Dword player = at<Dword>(rules, 204);
    if (auto *text = results_child(page, "<TEXT>", family::text_class_key)) {
        int previous_medal;
        const Byte has_medal = pending::profile_event_medal(pending::profile_event_record(
            bytes(rules) + 2488, at<Dword>(rules, 148), at<Dword>(rules, 102920)), &previous_medal);
        const int medal = pending::participant(at<void *>(system(92), 8), player)->awarded_medal;
        Dword reference;
        if (medal == 0) reference = 1240;
        else if (medal == 1) reference = 1239;
        else if (medal == 2) reference = 1238;
        else reference = has_medal ? 1508 : 1241;
        {
            pending::LocalFrontendString label;
            gui::string_reference(&label.value, reference);
            invoke<void>(text, 7, &label.value);
        }
        if (medal == 0 || medal == 1 || medal == 2)
            pending::announce_race_results(system(84), 3 - medal, 99);
    }
    return 1;
}

Byte show_points_breakdown(RaceResults *screen) {
    void *race = at<void *>(system(92), 8);
    screen->points_shown = 1;
    if (!invoke<Byte>(race, 35) || game::runtime::physics::split_screen()) return 0;
    results_page(&screen->base, "00Results", false);
    results_page(&screen->base, "01Reward", false);
    auto *page = static_cast<Widget *>(pending::find_child(&screen->base, "02Points"));
    if (!page) return 0;
    set_enabled(page, true);
    pending::set_widget_visible(page, true);
    activate_first_child(page);
    auto *list = reinterpret_cast<family::ListBox *>(results_child(page, "<LIST>", family::list_box_class_key));
    auto *heading = static_cast<Widget *>(pending::find_child(page, "<TEXT>"));
    const Dword player = at<Dword>(system(72), 204);
    auto *record = pending::participant(race, player);
    const Dword collision = pending::participant_award(record, 1);
    const Dword position = pending::participant_award(record, 0);
    const Dword clean_laps = pending::participant_award(record, 2);
    const Dword bonus = pending::participant_award(record, 3);
    const Dword pursuit = pending::participant_award(record, 4);
    const Dword total = pursuit + bonus + clean_laps + position + collision;
    pending::LocalFrontendString text, format;
    const Byte special = Byte(services::special_race_mode(system(72)));
    if (heading) {
        if (!special) {
            const Dword difficulty_labels[] = {42, 44, 45};
            gui::string_reference(&format.value, 1502);
            gui::string_format(&text.value, &format.value,
                gui::input::language_text(difficulty_labels[at<Dword>(system(72), 192)]));
            invoke<void>(heading, 7, &text.value);
        } else {
            set_enabled(heading, false);
            pending::set_widget_visible(heading, false);
        }
    }
    if (!list) return 1;
    // List operations append each eligible award, then address rows 5/6 explicitly for the total and
    // account balance.
    auto write_cell = [&](int row, Dword column) {
        ResultListItem item(&text.value);
        invoke<void>(&list->items, 7, &item.value, row, column);
    };
    auto append_value = [&](Dword value) {
        {
            ResultListItem item(&text.value);
            invoke<void>(&list->items, 8, &item.value);
        }
        gui::string_assign(&format.value, "%d");
        gui::string_format(&text.value, &format.value, value);
        const int count = at<int>(list, 280);
        write_cell(count - 1, 1);
    };
    if (!invoke<Byte>(race, 9)) {
        if (int(position) > 0 || at<int>(system(72), 180) > 0) {
            int place = at<int>(race, 34296 + player * 4);
            const Dword labels[] = {1493, 1494, 1495, 1496, 1497, 1498, 1499, 1500};
            if ((place < 0 || place > 7) && at<Dword>(system(72), 152) == 1)
                place = at<int>(race, 34392 + player * 4);
            gui::string_reference(&text.value, place < 0 || place > 7 ? 1488 : labels[place]);
            append_value(position);
        }
        if (int(collision) > 0 || at<int>(system(72), 180) > 0) {
            gui::string_reference(&format.value, 1468);
            gui::string_format(&text.value, &format.value, at<Dword>(race, player * 988 + 1556));
            append_value(collision);
        }
        if (int(clean_laps) > 0 || at<int>(system(72), 172) > 0) {
            gui::string_reference(&format.value, 1469);
            gui::string_format(&text.value, &format.value, at<Dword>(race, player * 988 + 1560));
            append_value(clean_laps);
        }
    } else {
        gui::string_reference(&format.value, 1492);
        gui::string_format(&text.value, &format.value, at<Dword>(race, player * 988 + 1568));
        append_value(pursuit);
    }
    if ((int(bonus) > 0 || special) && (at<Dword>(system(72), 152) != 2 ||
        pending::event_complete_progress(pending::active_event_progress(system(72))))) {
        gui::string_reference(&text.value, 1470);
        append_value(bonus);
    }
    gui::string_reference(&text.value, 1472);
    write_cell(5, 0);
    gui::string_assign(&format.value, "%d");
    gui::string_format(&text.value, &format.value, total);
    write_cell(5, 1);
    gui::string_reference(&text.value, 1490);
    write_cell(6, 0);
    const Dword balance = at<Dword>(system(72), 9728);
    gui::string_assign(&format.value, "%d");
    gui::string_format(&text.value, &format.value, balance);
    write_cell(6, 1);
    return 1;
}
}
