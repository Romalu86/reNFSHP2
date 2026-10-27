#include "../screens/frontend_menu_roots.hpp"
#include "../../gui/widgets/gui_widget_list.hpp"
#include "nfs/network/session/network_gui.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include <cstdlib>
namespace nfs::profiles {
gui::widgets::family::ListItem *construct_profile_list_item(gui::widgets::family::ListItem *);
}
namespace nfs::application::pending {
int event_participant_count(void *);
int __cdecl compare_event_points(const void *, const void *);
} // namespace nfs::application::pending
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::at;
using game::runtime::invoke;
using game::runtime::system;
namespace {
struct ResultItem {
    family::ListItem value;
    ResultItem() {
        profiles::construct_profile_list_item(&value);
    }
    ~ResultItem() {
        gui::string_destruct(&value.tooltip);
        gui::string_destruct(&value.text);
        value.methods = reinterpret_cast<const Method *>(gui::base_methods);
    }
};
void set_reference(Widget *widget, Dword reference) {
    pending::LocalFrontendString text;
    gui::string_reference(&text.value, reference);
    invoke<void>(widget, 7, &text.value);
}
Dword track_reference(void *progress, Dword stage) {
    void *tracks = system(40);
    const Dword track = pending::event_track(progress, stage);
    return at<Dword>(at<void *>(tracks, 12 + track * 4), 12);
}
} // namespace

void __fastcall initialize_tournament_status(TournamentStatus *screen, void *) {
    gui::widgets::pending::request_interface_layout(&screen->base, nullptr);
    void *progress = pending::active_event_progress(system(72));
    const int count = pending::event_participant_count(progress);
    const Dword stage = pending::event_stage(progress);
    pending::LocalFrontendString text;
    auto *heading = static_cast<Widget *>(pending::find_child(screen, "tx01b"));
    if (heading) {
        pending::set_widget_visible(heading, 1);
        if (tournament_page_state < 4) {
            if (tournament_page_state == 3)
                pending::set_widget_visible(heading, 0);
            else
                set_reference(heading, tournament_page_state == 0   ? 461
                                       : tournament_page_state == 1 ? 1371
                                                                    : 473);
        }
    }
    if (auto *race = static_cast<Widget *>(pending::find_child(screen, "<RACE>"))) {
        pending::set_widget_visible(race, 1);
        Dword round = Dword(count);
        switch (tournament_page_state) {
        case 0:
        case 1:
            round = stage;
            break;
        case 2:
            round = stage + 1;
            break;
        case 3:
            pending::set_widget_visible(race, 0);
            break;
        }
        const Dword races = (at<Dword>(progress, 0) >> 1 & 7) + 2;
        if (!round)
            round = 1;
        else if (races < round)
            round = races;
        {
            pending::LocalFrontendString format;
            gui::string_reference(&format.value, 1368);
            gui::string_format(&text.value, &format.value, round,
                               (at<Dword>(progress, 0) >> 1 & 7) + 2);
        }
        invoke<void>(race, 7, &text.value);
    }
    auto *track = static_cast<Widget *>(pending::find_child(screen, "<TRACK>"));
    if (track) {
        if (tournament_page_state == 1) {
            gui::string_copy(&text.value,
                             &lifecycle::construction::unresolved::shared_string);
            if (pending::event_active(progress) && stage)
                gui::string_append_reference(&text.value, track_reference(progress, stage - 1));
            invoke<void>(track, 7, &text.value);
            pending::set_widget_visible(track, 1);
        } else
            pending::set_widget_visible(track, 0);
    }
    Widget *reader;
    {
        pending::LocalFrontendString reader_name("<READER>");
        reader = family::named_sibling(track, &reader_name.value);
        if (reader && !invoke<Dword>(reader, 2, ticker_class_key))
            reader = nullptr;
    }
    if (reader) {
        pending::LocalFrontendString laps;
        Dword choice[7]{};
        switch (tournament_page_state) {
        case 0:
        case 2: {
            gui::string_reference(&text.value, 1369);
            gui::string_append_reference(&text.value, track_reference(progress, stage));
            gui::string_append_ascii(&text.value, "   ");
            gui::string_append_reference(&text.value, 2);
            pending::read_event_track(progress, stage, choice);
            gui::string_format_ascii(&laps.value, " %d", choice[2]);
            gui::string_append(&text.value, &laps.value);
            gui::string_append_ascii(&text.value, "   ");
            invoke<void>(reader, 7, &text.value);
            break;
        }
        case 1:
            gui::string_reference(&text.value, 472);
            invoke<void>(reader, 7, &text.value);
            break;
        case 3:
            set_reference(reader, 1370);
            break;
        }
        auto *ticker = reinterpret_cast<Ticker *>(reader);
        clear_ticker_messages(ticker);
        for (unsigned i = 0; i < 6; ++i)
            queue_ticker_message(ticker, invoke<const gui::String *>(ticker, 8), 1,
                                 &ticker->base.base.colour, 0, 100.0f, 0);
    }
    auto *list = reinterpret_cast<family::ListBox *>(
        track_child(screen, "<LIST>", family::list_box_class_key));
    if (!list)
        return;
    Dword slots[24];
    for (auto &slot : slots)
        slot = Dword(-1);
    network::ui::CarSettings car;
    network::ui::initialize_car_settings(&car);
    unsigned selected = 0;
    for (int slot = 0; slot < count; ++slot) {
        pending::read_event_participant(progress, Dword(slot), &car);
        if (!at<Dword>(&car, 8) && !at<Dword>(&car, 16))
            slots[selected++] = Dword(slot);
    }
    if (tournament_page_state == 1)
        tournament_sort_round = stage - 1;
    if (selected > 1) {
        if (tournament_page_state == 0)
            std::qsort(slots, selected, 4, compare_grid_order);
        else if (tournament_page_state == 1)
            std::qsort(slots, selected, 4, compare_tournament_round);
        else if (tournament_page_state == 2 || tournament_page_state == 3)
            std::qsort(slots, selected, 4, pending::compare_event_points);
    }
    if (auto *next = track_child(screen, "Next", family::button_class_key))
        if (tournament_page_state == 1 || tournament_page_state == 3)
            set_reference(next, 565);
    if (auto *back = track_child(screen, "Back", family::button_class_key))
        if (int(tournament_page_state) > 1 && int(tournament_page_state) < 4)
            set_reference(back, 566);
    for (Dword row = 0; row < selected; ++row) {
        ResultItem item;
        const Dword slot = slots[row];
        pending::read_event_participant(progress, slot, &car);
        auto cell = [&](Dword column) { invoke<void>(&list->items, 7, &item.value, row, column); };
        gui::string_format_ascii(&text.value, "%d", row + 1);
        gui::string_copy(&item.value.text, &text.value);
        cell(0);
        if (!at<Dword>(&car, 0))
            gui::string_assign(&item.value.text, tournament_driver_name(at<Dword>(&car, 52)));
        else {
            gui::String name;
            pending::copy_preference_name(pending::player_preferences(system(72), slot), &name);
            try {
                gui::string_copy(&item.value.text, &name);
            } catch (...) {
                gui::string_destruct(&name);
                throw;
            }
            gui::string_destruct(&name);
        }
        cell(1);
        if (tournament_page_state == 1) {
            char time[24];
            if (event_round_failed(progress, slot, stage - 1))
                gui::string_reference(&item.value.text, 1249);
            else {
                format_tournament_time(event_stage_time(progress, slot), time);
                gui::string_assign(&item.value.text, time);
            }
            cell(2);
            if (tournament_page_state == 1) {
                const auto lap = event_best_lap_time(progress, slot);
                if (lap < 1)
                    gui::string_reference(&item.value.text, 1316);
                else {
                    format_tournament_time(lap, time);
                    gui::string_assign(&item.value.text, time);
                }
                cell(3);
            }
        }
        if (tournament_page_state == 1) {
            gui::string_format_ascii(&text.value, "%d",
                                     stage ? event_round_points(progress, slot, stage - 1) : 0);
            gui::string_copy(&item.value.text, &text.value);
            cell(4);
        }
        gui::string_format_ascii(&text.value, "%d", pending::event_points(progress, slot));
        gui::string_copy(&item.value.text, &text.value);
        cell(5);
    }
}
} // namespace nfs::application::frontend
