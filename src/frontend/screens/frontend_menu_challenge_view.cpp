#include "frontend_menu_controls.hpp"
#include "nfs/network/packets/network_packet_handlers.hpp"
namespace nfs::application::pending {
void *create_widget(const gui::String *, const gui::String *, const gui::String *, void *);
}
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::invoke;
using game::runtime::system;

Dword selected_toggle_value(ProfileToggle *widget) {
    const auto *methods = widget->items.methods;
    const Dword row = Dword(family::selected_list_row(&widget->items));
    using Fetch = family::ListItem *(__thiscall *)(void *, Dword, Dword);
    auto *item = reinterpret_cast<Fetch>(methods[9])(&widget->items, row, Dword(0));
    return item ? item->data : ~Dword(0);
}

Dword select_toggle_value(ProfileToggle *widget, Dword value) {
    const int row = find_toggle_value(widget, value);
    return row >= 0 ? profiles::select_profile_row(widget, Dword(row)) : 0;
}
namespace {
struct Choice {
    const char *name;
    Dword ChallengePage::*field;
};
const Choice single_race_choices[] = {
    {"<NUM_PLAYERS>", &ChallengePage::players},
    {"<COPS>", &ChallengePage::cops},
    {"<NUM_OPPONENTS_SINGLE_RACE>", &ChallengePage::opponents},
    {"<OPPONENT_CARS_SINGLE_RACE>", &ChallengePage::opponent_class},
    {"<OPPONENT_SKILL>", &ChallengePage::skill},
    {"<TRAFFIC>", &ChallengePage::traffic}};
const Choice knockout_choices[] = {{"<NUM_PLAYERS>", &ChallengePage::players},
                                   {"<COPS>", &ChallengePage::cops},
                                   {"<NUM_OPPONENTS_LAP_KNOCKOUT>", &ChallengePage::opponents},
                                   {"<OPP_CLASS>", &ChallengePage::opponent_class},
                                   {"<OPPONENT_SKILL>", &ChallengePage::skill},
                                   {"<TRAFFIC>", &ChallengePage::traffic}};
const Choice tournament_choices[] = {{"<TOURNAMENT_LOAD_NEW>", &ChallengePage::load_tournament},
                                     {"<NUM_PLAYERS>", &ChallengePage::players},
                                     {"<COPS>", &ChallengePage::cops},
                                     {"<NUM_OPPONENTS_TOURNAMENT>", &ChallengePage::opponents},
                                     {"<OPPONENT_CARS_TOURNAMENT>", &ChallengePage::opponent_class},
                                     {"<OPPONENT_SKILL>", &ChallengePage::skill},
                                     {"<TRAFFIC>", &ChallengePage::traffic},
                                     {"<NUM_RACES>", &ChallengePage::races}};
const Choice quota_choices[] = {{"<NUM_PLAYERS>", &ChallengePage::players},
                                {"<OPPONENT_SKILL>", &ChallengePage::skill},
                                {"<TRAFFIC>", &ChallengePage::traffic},
                                {"<TIME_LIMITS>", &ChallengePage::time_limit}};
const Choice free_run_choices[] = {{"<NUM_PLAYERS>", &ChallengePage::players},
                                   {"<TRAFFIC>", &ChallengePage::traffic}};
struct ChoiceSet {
    const Choice *choices;
    unsigned count;
};
const ChoiceSet choice_sets[] = {{single_race_choices, 6},
                                 {knockout_choices, 6},
                                 {tournament_choices, 8},
                                 {quota_choices, 4},
                                 {free_run_choices, 2}};
ProfileToggle *choice_widget(ChallengeView *view, const char *name) {
    auto *widget = static_cast<Widget *>(pending::find_child(view, name));
    return widget && invoke<Dword>(widget, 2, profiles::profile_list_class_key)
               ? reinterpret_cast<ProfileToggle *>(widget)
               : nullptr;
}
} // namespace

static void save_challenge_choices(ChallengeView *widget, Dword selection) {
    if (selection > 4)
        return;
    auto &page = challenge_pages[selection];
    const auto &set = choice_sets[selection];
    for (unsigned index = 0; index < set.count; ++index)
        if (auto *choice = choice_widget(widget, set.choices[index].name))
            page.*(set.choices[index].field) = selected_toggle_value(choice);
}

static void restore_challenge_choices(ChallengeView *widget, Dword selection) {
    if (selection > 4)
        return;
    auto &page = challenge_pages[selection];
    const auto &set = choice_sets[selection];
    for (unsigned index = 0; index < set.count; ++index) {
        const auto &choice = set.choices[index];
        const Dword value = page.*(choice.field);
        if (value != ~Dword(0))
            if (auto *control = choice_widget(widget, choice.name))
                select_toggle_value(control, page.*(choice.field));
    }
}

static void reload_challenge_view(ChallengeView *widget) {
    bool refocus = false;
    if (gui::events::find_widget(gui::context_state, active_widget_id)) {
        auto *active =
            static_cast<Widget *>(gui::events::find_widget(gui::context_state, active_widget_id));
        refocus = descendant(&widget->base.base, active->id) != nullptr;
    }
    const int selection = game::runtime::at<int>(system(72), 2484);
    widget->selected = selection;
    if (widget->content) {
        save_challenge_choices(widget, last_challenge_selection);
        pending::destroy_screen(widget->content);
        widget->content = nullptr;
    }
    {
        pending::LocalFrontendString type(challenge_pages[selection].type),
            name(challenge_pages[widget->selected].name), file("System\\GInterface.gui");
        widget->content = static_cast<Widget *>(
            pending::create_widget(&file.value, &name.value, &type.value, widget));
    }
    restore_challenge_choices(widget, Dword(selection));
    if (refocus)
        activate_first_child(pending::widget_root(&widget->base.base));
}

void __fastcall release_challenge_view(ChallengeView *widget, void *) {
    save_challenge_choices(widget, last_challenge_selection);
    clear_layout_request(&widget->base.base, nullptr);
}

Dword __fastcall challenge_view_event(ChallengeView *widget, void *, Dword event, Dword first,
                                     Dword second) {
    if (event == 19) {
        reload_challenge_view(widget);
        network::packets::pending::set_game_rule(
            system(72), 15, challenge_pages[widget->selected].opponent_class);
    }
    return gui::widgets::pending::general_event(&widget->base, nullptr, event, first, second);
}

void __fastcall initialize_challenge_view(ChallengeView *widget, void *) {
    gui::widgets::pending::request_general_layout(&widget->base, nullptr);
    auto *parent = widget->base.base.parent;
    widget->selected = game::runtime::at<int>(system(72), 2484);
    if (invoke<Dword>(parent, 3, "SingleChallengeScreen") &&
        invoke<Dword>(parent, 2, challenge_screen_class_key)) {
        set_challenge_heading(parent, challenge_pages[widget->selected].title);
        update_challenge_heading(widget, 0, 0, 0);
    }
}

void __fastcall update_challenge_view(ChallengeView *widget, void *, Dword) {
    const int selection = game::runtime::at<int>(system(72), 2484);
    last_challenge_selection = Dword(widget->selected);
    if (widget->selected == selection)
        return;
    reload_challenge_view(widget);
    auto *parent = widget->base.base.parent;
    if (invoke<Dword>(parent, 3, "SingleChallengeScreen")) {
        if (invoke<Dword>(parent, 2, challenge_screen_class_key)) {
            set_challenge_heading(parent, challenge_pages[selection].title);
            update_challenge_heading(widget, 0, 0, 0);
        } else
            parent = nullptr;
        auto *selector = static_cast<Widget *>(pending::find_child(parent, "<OPTTAB>"));
        if (selector && invoke<Dword>(selector, 2, type_spin_class_key))
            update_challenge_heading(selector, 0, 0, 0);
    }
}
} // namespace nfs::application::frontend
