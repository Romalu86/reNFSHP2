#include "../screens/frontend_menu_pages.hpp"
namespace nfs::application::pending {
Byte event_humans(void *);
Dword event_racers(void *);
Byte event_car_class(void *);
Dword event_cops(void *);
Dword event_difficulty(void *);
Dword event_helicopters(void *);
} // namespace nfs::application::pending
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::at;
using game::runtime::invoke;
using game::runtime::system;
using network::packets::pending::set_game_rule;
using network::packets::pending::set_rule_variant;

void synchronize_page_toggle(ProfileToggle *toggle) {
    if (toggle->binding) {
        const Dword value = network::packets::rule_value(system(72), int(toggle->binding));
        const int row = find_toggle_value(toggle, value);
        if (row >= 0) {
            profiles::select_profile_row(toggle, Dword(row));
            return;
        }
    }
    const int row = find_toggle_value(toggle, toggle->default_value);
    if (row >= 0) {
        profiles::select_profile_row(toggle, Dword(row));
        return;
    }
    profiles::select_profile_row(toggle, 0);
    set_rule_variant(system(72), int(toggle->binding), selected_toggle_value(toggle));
}

void __fastcall initialize_tournament(TournamentPage *page, void *) {
    gui::widgets::pending::request_general_layout(&page->base, nullptr);
    page->load_new = find_page_toggle(page, "<TOURNAMENT_LOAD_NEW>");
    page->players = find_page_toggle(page, "<NUM_PLAYERS>");
    page->cops = find_page_toggle(page, "<COPS>");
    page->opponents = find_page_toggle(page, "<NUM_OPPONENTS_TOURNAMENT>");
    page->opponent_cars = find_page_toggle(page, "<OPPONENT_CARS_TOURNAMENT>");
    page->skill = find_page_toggle(page, "<OPPONENT_SKILL>");
    page->traffic = find_page_toggle(page, "<TRAFFIC>");
    page->races = find_page_toggle(page, "<NUM_RACES>");
}
// Body is RET4.
void __fastcall update_tournament(TournamentPage *, void *, Dword) {
}
static void append_tournament_choice(TournamentPage *page, const gui::String *text, Dword data) {
    family::ListItem item;
    construct_toggle_item(&item, text, data);
    try {
        invoke(&page->load_new->items, 8, &item);
    } catch (...) {
        gui::string_destruct(&item.tooltip);
        gui::string_destruct(&item.text);
        throw;
    }
    gui::string_destruct(&item.tooltip);
    gui::string_destruct(&item.text);
}

static void load_saved_tournament_choices(TournamentPage *page) {
    auto *progress = pending::active_event_progress(system(72));
    set_rule_variant(system(72), 13, pending::event_humans(progress));
    set_rule_variant(system(72), 14, pending::event_racers(progress));
    set_rule_variant(system(72), 15, pending::event_car_class(progress));
    set_rule_variant(system(72), 11, pending::event_cops(progress) != 0);
    set_rule_variant(system(72), 16, pending::event_difficulty(progress));
    set_rule_variant(system(72), 12, pending::event_helicopters(progress));
    set_rule_variant(system(72), 8, ((at<Dword>(progress, 0) >> 1) & 7) + 2);
    synchronize_page_toggle(page->players);
    synchronize_page_toggle(page->cops);
    synchronize_page_toggle(page->opponents);
    synchronize_page_toggle(page->opponent_cars);
    synchronize_page_toggle(page->skill);
    synchronize_page_toggle(page->traffic);
    synchronize_page_toggle(page->races);
}

static void update_tournament_choices(TournamentPage *page) {
    if (selected_toggle_value(page->load_new)) {
        auto *players = page->players;
        set_enabled(reinterpret_cast<Widget *>(players), 1);
        auto *cops = page->cops;
        set_enabled(reinterpret_cast<Widget *>(cops), 1);
        auto *opponents = page->opponents;
        set_enabled(reinterpret_cast<Widget *>(opponents), 1);
        auto *cars = page->opponent_cars;
        set_enabled(reinterpret_cast<Widget *>(cars), 1);
        auto *skill = page->skill;
        set_enabled(reinterpret_cast<Widget *>(skill), 1);
        auto *traffic = page->traffic;
        set_enabled(reinterpret_cast<Widget *>(traffic), 1);
        auto *races = page->races;
        set_enabled(reinterpret_cast<Widget *>(races), 1);
        for (auto *toggle : {players, cops, opponents, cars, skill, traffic, races})
            synchronize_page_toggle(toggle);
    } else {
        load_saved_tournament_choices(page);
        set_enabled(reinterpret_cast<Widget *>(page->players), 0);
        set_enabled(reinterpret_cast<Widget *>(page->cops), 0);
        set_enabled(reinterpret_cast<Widget *>(page->opponents), 0);
        set_enabled(reinterpret_cast<Widget *>(page->opponent_cars), 0);
        set_enabled(reinterpret_cast<Widget *>(page->skill), 0);
        set_enabled(reinterpret_cast<Widget *>(page->traffic), 0);
        set_enabled(reinterpret_cast<Widget *>(page->races), 0);
    }
}

Dword __fastcall tournament_event(TournamentPage *page, void *, Dword event, Dword first,
                                 Dword second) {
    pending::LocalFrontendString text;
    if (event == 19) {
        gui::string_reference(&text.value, 596);
        append_tournament_choice(page, &text.value, 1);
        bool resumable = false;
        if (pending::event_active(pending::active_event_progress(system(72))))
            resumable =
                !pending::event_complete_progress(pending::active_event_progress(system(72)));
        if (resumable) {
            gui::string_reference(&text.value, 372);
            append_tournament_choice(page, &text.value, 0);
            invoke<Dword>(&page->load_new->items, 10, Dword(1), Dword(1));
            update_tournament_choices(page);
        } else {
            auto *toggle = page->load_new;
            set_enabled(reinterpret_cast<Widget *>(toggle), 0);
            invoke<Dword>(&toggle->items, 10, Dword(0), Dword(1));
            update_tournament_choices(page);
            set_rule_variant(system(72), 13, at<Dword>(system(72), 176));
            set_rule_variant(system(72), 14,
                                      network::packets::rule_value(system(72), 14));
            set_rule_variant(system(72), 11,
                                      network::packets::rule_value(system(72), 11));
        }
    }
    return gui::widgets::pending::general_event(&page->base, nullptr, event, first, second);
}

void __fastcall tournament_child_event(TournamentPage *page, void *, Widget *sender, Dword event,
                                       Dword, Dword) {
    if ((event == 68 || event == 70) && sender == reinterpret_cast<Widget *>(page->load_new))
        update_tournament_choices(page);
}

static bool confirm_tournament_replacement(TournamentPage *page) {
    if (!pending::event_active(pending::active_event_progress(system(72))) ||
        !selected_toggle_value(page->load_new))
        return true;
    pending::MessageBox message;
    {
        pending::LocalFrontendString title, text;
        gui::string_reference(&title.value, 1329);
        gui::string_reference(&text.value, 1330);
        pending::construct_message_box(&message, &text.value, &title.value, 18);
    }
    int result;
    try {
        result = pending::run_message_box(&message, nullptr, 0);
    } catch (...) {
        pending::destroy_message_box(&message);
        throw;
    }
    pending::destroy_message_box(&message);
    return result == 2;
}

bool can_leave_challenge_view(ChallengeView *view) {
    auto *content = view->content;
    return !content || !invoke<Dword>(content, 2, tournament_class_key) ||
           confirm_tournament_replacement(reinterpret_cast<TournamentPage *>(content));
}

Dword __fastcall finish_tournament(TournamentPage *page, void *, Dword result, Dword) {
    auto *root = pending::widget_root(&page->base.base);
    if (!root || root->modal_result != 4)
        return 1;
    auto *rules = system(72);
    set_game_rule(rules, 2, 2);
    at<std::uint8_t>(rules, 4) = 0;
    if (!selected_toggle_value(page->load_new)) {
        pop_frontend_screen(system(96));
        pending::LocalFrontendString type("TourneyStatus"), name("TourneyStatus");
        pending::enqueue_screen(system(96), name.release(), type.release());
        page->base.base.modal_result = result;
        return 1;
    }
    pending::clear_event_progress(pending::active_event_progress(rules));
    auto *players_toggle = page->players;
    set_game_rule(rules, 13, selected_toggle_value(players_toggle));
    const Dword players = selected_toggle_value(players_toggle);
    set_game_rule(rules, 13, players);
    if (players == 1) {
        at<Dword>(rules, 204) = 0;
        at<Dword>(rules, 208) = ~Dword(0);
    } else if (players == 2) {
        at<Dword>(rules, 204) = 0;
        at<Dword>(rules, 208) = 1;
    }
    auto *cops = page->cops;
    set_game_rule(rules, 11, selected_toggle_value(cops));
    at<Dword>(rules, 172) = selected_toggle_value(cops);
    set_game_rule(rules, 14, selected_toggle_value(page->opponents));
    set_game_rule(rules, 15, selected_toggle_value(page->opponent_cars));
    set_game_rule(rules, 16, selected_toggle_value(page->skill));
    set_game_rule(rules, 12, selected_toggle_value(page->traffic));
    set_game_rule(rules, 8, selected_toggle_value(page->races));
    const Dword races = at<Dword>(rules, 232);
    auto *progress = pending::active_event_progress(rules);
    at<Dword>(progress, 0) ^= ((races * 2 - 3) ^ at<Dword>(progress, 0)) & 14;
    page->base.base.modal_result = result;
    return 1;
}
} // namespace nfs::application::frontend
