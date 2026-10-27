#include "frontend_menu_pages.hpp"
#include "nfs/network/session/network_rules.hpp"
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::at;
using game::runtime::invoke;
using game::runtime::system;
using network::packets::pending::set_game_rule;
using network::packets::pending::set_rule_variant;
ProfileToggle *find_page_toggle(void *page, const char *name) {
    auto *widget = static_cast<Widget *>(pending::find_child(page, name));
    return widget && invoke<Dword>(widget, 2, profiles::profile_list_class_key)
               ? reinterpret_cast<ProfileToggle *>(widget)
               : nullptr;
}
static void assign_player_slots(void *rules, Dword players) {
    if (players == 1) {
        at<Dword>(rules, 204) = 0;
        at<Dword>(rules, 208) = ~Dword(0);
    } else if (players == 2) {
        at<Dword>(rules, 204) = 0;
        at<Dword>(rules, 208) = 1;
    }
}

void __fastcall initialize_free_run(FreeRunPage *page, void *) {
    gui::widgets::pending::request_general_layout(&page->base, nullptr);
    page->players = find_page_toggle(page, "<NUM_PLAYERS>");
    page->traffic = find_page_toggle(page, "<TRAFFIC>");
}
// Body is RET4.
void __fastcall update_free_run(FreeRunPage *, void *, Dword) {
}

void __fastcall free_run_child_event(FreeRunPage *page, void *, Widget *, Dword event, Dword, Dword) {
    if (event != 24)
        return;
    auto *rules = system(72);
    set_game_rule(rules, 2, 5);
    Dword players = 1;
    auto *player_choice = page->players;
    if (player_choice) {
        set_game_rule(rules, 13, selected_toggle_value(player_choice));
        players = selected_toggle_value(player_choice);
        set_game_rule(rules, 13, players);
    }
    assign_player_slots(rules, players);
    set_game_rule(rules, 14, 0);
    set_game_rule(rules, 7, 100);
    set_game_rule(rules, 15, 0);
    set_game_rule(rules, 16, 0);
    if (page->traffic)
        set_game_rule(rules, 12, selected_toggle_value(page->traffic));
    set_game_rule(rules, 11, 0);
    set_game_rule(rules, 8, 1);
}
template <class RacePage>
static void bind_race_choices(RacePage *page, const char *opponents, const char *cars) {
    gui::widgets::pending::request_general_layout(&page->base, nullptr);
    page->players = find_page_toggle(page, "<NUM_PLAYERS>");
    page->cops = find_page_toggle(page, "<COPS>");
    page->opponents = find_page_toggle(page, opponents);
    page->opponent_cars = find_page_toggle(page, cars);
    page->skill = find_page_toggle(page, "<OPPONENT_SKILL>");
    page->traffic = find_page_toggle(page, "<TRAFFIC>");
}

void __fastcall initialize_single_race(SingleRacePage *page, void *) {
    bind_race_choices(page, "<NUM_OPPONENTS_SINGLE_RACE>", "<OPPONENT_CARS_SINGLE_RACE>");
}
// Body is RET4.
void __fastcall update_single_race(SingleRacePage *, void *, Dword) {
}

void __fastcall single_race_child_event(SingleRacePage *page, void *, Widget *, Dword event, Dword,
                                        Dword) {
    if (event == 19) {
        auto *rules = system(72);
        set_rule_variant(rules, 13, at<Dword>(rules, 176));
        return;
    }
    if (event != 24)
        return;
    auto *rules = system(72);
    set_game_rule(rules, 2, 0);
    const Dword players = page->players ? selected_toggle_value(page->players) : 1;
    set_game_rule(rules, 13, players);
    assign_player_slots(rules, players);
    const Dword cops = page->cops ? selected_toggle_value(page->cops) : 0;
    set_game_rule(rules, 11, cops);
    at<Dword>(rules, 172) = cops;
    const Dword opponents = page->opponents ? selected_toggle_value(page->opponents) : 0;
    if (!opponents)
        set_game_rule(rules, 14, 0);
    else {
        set_game_rule(rules, 14, opponents == 1 ? 1 : 2);
        set_game_rule(rules, 15,
                          page->opponent_cars ? selected_toggle_value(page->opponent_cars) : 0);
        set_game_rule(rules, 16, page->skill ? selected_toggle_value(page->skill) : 0);
    }
    set_game_rule(rules, 12, page->traffic ? selected_toggle_value(page->traffic) : 0);
    set_game_rule(rules, 8, 1);
}

void __fastcall initialize_knockout(KnockoutPage *page, void *) {
    bind_race_choices(page, "<NUM_OPPONENTS_LAP_KNOCKOUT>", "<OPP_CLASS>");
}

void refresh_knockout_choices(KnockoutPage *page) {
    auto *cops = page->cops;
    if (!cops)
        return;
    auto *opponents = page->opponents;
    if (!opponents)
        return;
    if (page->previous_cops == static_cast<int>(selected_toggle_value(cops)) &&
        page->previous_opponents == static_cast<int>(selected_toggle_value(opponents)))
        return;
    page->previous_cops = static_cast<int>(selected_toggle_value(cops));
    const int selected = static_cast<int>(selected_toggle_value(opponents));
    page->previous_opponents = selected;
    const int maximum = selected_toggle_value(cops) == 1 ? 5 : 7;
    Dword choices[24];
    int count = 0;
    for (; count < maximum; ++count)
        choices[count] = Dword(count + 1);
    network::rules::choice_values(static_cast<Dword *>(system(96)), 14, Dword(count), choices);
    if (selected > maximum) {
        page->previous_opponents = maximum;
        select_toggle_value(opponents, Dword(maximum));
    }
}
// The call precedes RET4.
void __fastcall update_knockout(KnockoutPage *page, void *, Dword) {
    refresh_knockout_choices(page);
}

void __fastcall knockout_child_event(KnockoutPage *page, void *, Widget *, Dword event, Dword, Dword) {
    if (event == 19) {
        auto *rules = system(72);
        set_rule_variant(rules, 13, at<Dword>(rules, 176));
        return;
    }
    if (event != 24)
        return;
    auto *rules = system(72);
    set_game_rule(rules, 2, 1);
    auto *player_choice = page->players;
    set_game_rule(rules, 13, selected_toggle_value(player_choice));
    const Dword players = selected_toggle_value(player_choice);
    set_game_rule(rules, 13, players);
    assign_player_slots(rules, players);
    set_game_rule(rules, 11, selected_toggle_value(page->cops));
    at<Dword>(rules, 172) = selected_toggle_value(page->cops);
    const Dword opponents = selected_toggle_value(page->opponents);
    set_game_rule(rules, 14, opponents);
    set_game_rule(rules, 7, opponents - 1 + players);
    set_game_rule(rules, 15, selected_toggle_value(page->opponent_cars));
    set_game_rule(rules, 16, selected_toggle_value(page->skill));
    set_game_rule(rules, 12, selected_toggle_value(page->traffic));
    set_game_rule(rules, 8, 1);
}

void __fastcall initialize_quota(QuotaPage *page, void *) {
    gui::widgets::pending::request_general_layout(&page->base, nullptr);
    page->players = find_page_toggle(page, "<NUM_PLAYERS>");
    page->skill = find_page_toggle(page, "<OPPONENT_SKILL>");
    page->traffic = find_page_toggle(page, "<TRAFFIC>");
    page->time_limit = find_page_toggle(page, "<TIME_LIMITS>");
}
// Body is RET4.
void __fastcall update_quota(QuotaPage *, void *, Dword) {
}

void __fastcall quota_child_event(QuotaPage *page, void *, Widget *, Dword event, Dword, Dword) {
    if (event != 24)
        return;
    auto *rules = system(72);
    set_game_rule(rules, 2, 10);
    auto *player_choice = page->players;
    set_game_rule(rules, 13, selected_toggle_value(player_choice));
    const Dword players = selected_toggle_value(player_choice);
    set_game_rule(rules, 13, players);
    assign_player_slots(rules, players);
    set_game_rule(rules, 11, 1);
    set_game_rule(rules, 14, 7);
    set_game_rule(rules, 7, 7);
    set_game_rule(rules, 15, 43);
    set_game_rule(rules, 16, selected_toggle_value(page->skill));
    set_game_rule(rules, 12, selected_toggle_value(page->traffic));
    set_game_rule(rules, 8, 1);
    const Dword cached = at<Dword>(system(72), 2308);
    pending::set_round_rule_cache(8, cached);
    pending::set_round_rule_cache(7, cached);
    pending::set_round_rule_cache(3, cached);
    const Dword selection = selected_toggle_value(page->time_limit);
    set_game_rule(rules, 3, selection == 1 ? 18000 : selection == 2 ? 25200 : 10800);
}
} // namespace nfs::application::frontend
