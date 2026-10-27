#include "frontend_race_results.hpp"
#include <cstdio>
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::at;
using game::runtime::invoke;
using game::runtime::system;

void __fastcall initialize_cop_results(CopVsPerpResults *screen, void *) {
    family::dialog_initialize(&screen->base, nullptr);
    pending::stop_force_feedback();
    at<Dword>(screen, 272) = 0;
    screen->results = reinterpret_cast<family::ListBox *>(
        results_child(&screen->base, "<RESULTS>", family::list_box_class_key));
    if (screen->results) {
        void *race = at<void *>(system(92), 8);
        ResultListItem item;
        invoke<void>(race, 23);
        Dword slots[24];
        int count;
        invoke<void>(race, 38, slots, &count);
        for (int row = 0; row < count; ++row) {
            const Dword slot = slots[row];
            char number[32];
            std::sprintf(number, "%d", row + 1);
            {
                pending::LocalFrontendString position(number);
                gui::string_copy(&item.value.text, &position.value);
            }
            invoke<void>(&screen->results->items, 7, &item.value, row, Dword(0));
            gui::String name;
            participant_display_name(race, &name, slot);
            try { gui::string_copy(&item.value.text, &name); }
            catch (...) { gui::string_destruct(&name); throw; }
            gui::string_destruct(&name);
            invoke<void>(&screen->results->items, 7, &item.value, row, Dword(1));
            pending::LocalFrontendString car;
            participant_car_label(race, slot, &car.value);
            gui::string_copy(&item.value.text, &car.value);
            invoke<void>(&screen->results->items, 7, &item.value, row, Dword(2));
            pending::LocalFrontendString score;
            gui::string_copy(&score.value, &lifecycle::construction::unresolved::shared_string);
            if (game::runtime::ai::lifecycle::pending::participant_enabled(race, slot)) {
                auto *record = pending::participant(race, slot);
                const Dword captures = pending::participant_is_cop(race, slot) ? record->takedowns : record->losses;
                gui::string_format_ascii(&score.value, "%d of %d", captures, at<Dword>(system(72), 232));
            }
            gui::string_copy(&item.value.text, &score.value);
            invoke<void>(&screen->results->items, 7, &item.value, row, Dword(3));
        }
    }
    auto *record = pending::participant(at<void *>(system(92), 8), at<Dword>(system(72), 204));
    if (auto *points = results_child(&screen->base, "<POINTS>", family::text_class_key)) {
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
    if (auto *total = results_child(&screen->base, "<TOTAL>", family::text_class_key))
        result_integer(total, at<Dword>(system(72), 9728));
    if (auto *bonus = results_child(&screen->base, "<BONUS>", family::text_class_key)) {
        if (Byte(pending::result_has_event_bonus()))
            result_integer(bonus, pending::participant_award(record, 3));
        else {
            pending::set_widget_visible(bonus, false);
            if (auto *label = results_child(&screen->base, "t2", family::text_class_key))
                pending::set_widget_visible(label, false);
        }
    }
}
}
