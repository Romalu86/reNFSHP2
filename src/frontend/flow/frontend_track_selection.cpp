#include "nfs/core/lifecycle/scope_cleanup.hpp"
#include "frontend_track_selection.hpp"
#include "../../race/session/race_session.hpp"
#include "../../profiles/storage/player_profile.hpp"
namespace nfs::profiles {
gui::widgets::family::ListItem *construct_profile_list_item(gui::widgets::family::ListItem *);
}
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::at;
using game::runtime::bytes;
using game::runtime::invoke;
using game::runtime::system;

TrackSelection *construct_track_selection(TrackSelection *screen, void *parent) {
    gui::widgets::pending::construct_interface(screen, parent);
    kernel::lifecycle::Cleanup base_cleanup([=] { destroy_interface(&screen->base); });
    screen->track = -1;
    screen->direction = 0;
    gui::string_construct(&screen->track_name);
    kernel::lifecycle::Cleanup track_name_cleanup([=] { gui::string_destruct(&screen->track_name); });
    screen->preview_dirty = 1;
    screen->lock = screen->balance = screen->cost = nullptr;
    screen->laps = nullptr;
    screen->playlist = nullptr;
    screen->preview = nullptr;
    screen->description = nullptr;
    screen->next = nullptr;
    screen->tracks = system(40);
    screen->preferences = bytes(system(72)) + 2488;
    screen->base.methods = track_selection_methods.data();
    gui::string_reference(&screen->track_name, at<Dword>(at<void *>(screen->tracks, 12), 12));
    track_name_cleanup.dismiss();
    base_cleanup.dismiss();
    return screen;
}

void destroy_track_selection(TrackSelection *screen) {
    screen->base.methods = track_selection_methods.data();
    kernel::lifecycle::Cleanup base_cleanup([=] { destroy_interface(&screen->base); });
    gui::string_destruct(&screen->track_name);
}

void __fastcall initialize_track_selection(TrackSelection *screen, void *) {
    gui::widgets::pending::request_interface_layout(&screen->base, nullptr);
    clear_menu_input_queue();
    screen->event_progress = nullptr;
    screen->playlist = nullptr;
    Widget *tournament = static_cast<Widget *>(pending::find_child(screen, "TournTrax"));
    if (tournament && invoke<Dword>(tournament, 2, general_class_name)) {
        set_enabled(tournament, 0);
        pending::set_widget_visible(tournament, 0);
        screen->playlist = reinterpret_cast<family::ListBox *>(
            track_child(tournament, "list", family::list_box_class_key));
        if (screen->playlist && at<Dword>(system(72), 152) == 2) {
            set_enabled(tournament, 1);
            pending::set_widget_visible(tournament, 1);
            screen->event_progress = pending::active_event_progress(system(72));
            clear_track_playlist(screen->playlist,
                                 int((at<Dword>(screen->event_progress, 0) >> 1 & 7) + 2));
            if (at<std::uint8_t>(system(72), 4)) {
                for (Dword stage = 0; stage < (at<Dword>(screen->event_progress, 0) >> 1 & 7) + 2;
                     ++stage) {
                    Dword choice[7]{};
                    pending::read_event_track(screen->event_progress, stage, choice);
                    pending::LocalFrontendString name;
                    gui::string_reference(
                        &name.value,
                        at<Dword>(at<void *>(screen->tracks, 12 + choice[0] * 4), 12));
                    append_track_playlist(screen->playlist, &name.value, choice[2] ? choice[2] : 2,
                                          choice[0], int(choice[1]));
                }
            }
        }
    }
    screen->cost = track_child(screen, "<Cost>", family::text_class_key);
    screen->balance = track_child(screen, "<BNK2>", family::text_class_key);
    screen->lock = track_child(screen, "<lock>", family::static_image_class_key);
    if (screen->lock) {
        set_enabled(screen->lock, 0);
        pending::set_widget_visible(screen->lock, 0);
    }
    screen->preview =
        reinterpret_cast<ActorPreview *>(track_child(screen, "TRCK", actor_preview_class_key));
    screen->track = at<int>(screen->preferences, 7512) / 4;
    screen->track_images = track_child(screen, "<tracklist>", family::anim_image_class_key);
    family::select_sequence_frame(
        reinterpret_cast<family::ImageSequence *>(bytes(screen->track_images) + 328),
        screen->track);
    gui::string_reference(&screen->track_name, at<Dword>(selected_track_record(screen), 12));
    screen->track_selector = reinterpret_cast<TypeSpinControl *>(
        track_child(screen, "<trackselect>", type_spin_class_key));
    select_type_spin(screen->track_selector, screen->track);
    screen->direction_wheel =
        reinterpret_cast<ImageWheel *>(track_child(screen, "<TrackDir>", image_wheel_class_key));
    screen->direction_wheel->selection = at<int>(screen->preferences, 7512) % 4;
    screen->direction = screen->direction_wheel->selection;
    screen->description =
        reinterpret_cast<Ticker *>(track_child(screen, "<TDS>", ticker_class_key));
    screen->next = track_child(screen, "<t2>", family::text_class_key);
    populate_track_laps(screen);
}

Dword __fastcall track_selection_event(TrackSelection *screen, void *, Dword event, Dword first,
                                      Dword second) {
    if (event == 19) {
        restore_menu_rule_bindings(system(72));
        select_type_spin(screen->track_selector, screen->track);
        const Dword kind = at<Dword>(system(72), 152);
        if (kind == 1 || kind == 10 || kind == 6)
            for (int row = 0; row < int(type_spin_count(screen->track_selector)); ++row) {
                const Dword track = type_spin_value(screen->track_selector, row);
                if (!at<std::uint8_t>(at<void *>(screen->tracks, 12 + track * 4), 24))
                    enable_type_spin_value(screen->track_selector, int(row), 0);
            }
        refresh_track_selection(screen);
    }
    return gui::widgets::pending::handle_event(&screen->base, nullptr, event, first, second);
}

Dword __fastcall track_selection_key(TrackSelection *screen, void *, Dword key, Dword modifiers) {
    switch (key) {
    case 200:
        if (screen->direction_wheel)
            dispatch_target(screen->direction_wheel->base.base.id, 107, 0, 0);
        return 1;
    case 203:
        if (screen->track_selector) {
            dispatch_target(screen->track_selector->base.base.id, 107, 0, 0);
            return 1;
        }
        break;
    case 205:
        if (screen->track_selector) {
            dispatch_target(screen->track_selector->base.base.id, 106, 0, 0);
            return 1;
        }
        break;
    case 208:
        if (screen->direction_wheel)
            dispatch_target(screen->direction_wheel->base.base.id, 106, 0, 0);
        return 1;
    }
    return reinterpret_cast<Dword(__thiscall *)(Widget *, Dword, Dword)>(interface_methods[32])(
        &screen->base, key, modifiers);
}

void __fastcall update_track_selection(TrackSelection *screen, void *, Dword) {
    if (screen->track != int(network::packets::rule_value(system(72), 5)) ||
        screen->direction_wheel->selection != int(network::packets::rule_value(system(72), 6)))
        refresh_track_selection(screen);
}

void populate_track_laps(TrackSelection *screen) {
    screen->laps = reinterpret_cast<ProfileToggle *>(
        track_child(screen, "<LAPS>", family::toggle_button_class_key));
    if (!screen->laps)
        return;
    auto *widget = reinterpret_cast<Widget *>(screen->laps);
    set_enabled(widget, 0);
    pending::set_widget_visible(widget, 0);
    invoke<void>(&screen->laps->items, 1);
    void *rules = system(72);
    int count = 0, selected = 0;
    if (at<Dword>(rules, 148) == 1) {
        if (!at<Dword>(rules, 152)) {
            count = 8;
            selected = at<int>(rules, 228);
            if (selected < 1)
                selected = 2;
            else if (selected > 8)
                selected = 8;
        } else if (at<Dword>(rules, 152) == 5 &&
                   !at<std::uint8_t>(selected_track_record(screen), 24)) {
            count = 1;
            selected = 1;
        }
    }
    if (screen->event_progress) {
        count = 8;
        selected = at<int>(rules, 228);
        if (selected < 1)
            selected = 2;
        else if (selected > 8)
            selected = 8;
    }
    if (!at<std::uint8_t>(selected_track_record(screen), 24)) {
        count = 1;
        selected = 1;
    } else if (!count)
        return;
    int selection = 0;
    for (int lap = 1; lap <= count; ++lap) {
        const Dword row = invoke<Dword>(&screen->laps->items, 3);
        family::ListItem item;
        profiles::construct_profile_list_item(&item);
        try {
            pending::LocalFrontendString number;
            gui::string_format_ascii(&number.value, "%d", lap);
            gui::string_copy(&item.text, &number.value);
            item.flags &= ~Dword(2);
            if (lap == selected)
                selection = lap - 1;
            item.data = Dword(lap);
            invoke<void>(&screen->laps->items, 7, &item, row, Dword(0));
        } catch (...) {
            gui::string_destruct(&item.tooltip);
            gui::string_destruct(&item.text);
            throw;
        }
        gui::string_destruct(&item.tooltip);
        gui::string_destruct(&item.text);
    }
    if (screen->laps->items.grid.rows.count > 1) {
        set_enabled(widget, 1);
        pending::set_widget_visible(widget, 1);
    }
    invoke<void>(&screen->laps->items, 10, Dword(selection), Dword(1));
}
} // namespace nfs::application::frontend
