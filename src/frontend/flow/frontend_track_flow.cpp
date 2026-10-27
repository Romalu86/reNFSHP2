#include "frontend_track_selection.hpp"
#include "../../race/session/race_session.hpp"
namespace audio_layout = nfs::game::audio::state::layout;
namespace nfs::application::pending {
void write_event_track(void *, Dword, const void *);
}
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::at;
using game::runtime::bytes;
using game::runtime::invoke;
using game::runtime::system;

void track_purchase_error() {
    game::audio::play_frontend_event(66, 63);
    pending::MessageBox message;
    {
        pending::LocalFrontendString text;
        gui::string_reference(&text.value, 1425);
        pending::construct_message_box(&message, &text.value);
    }
    try {
        pending::run_message_box(&message, nullptr, 0);
    } catch (...) {
        pending::destroy_message_box(&message);
        throw;
    }
    pending::destroy_message_box(&message);
}

Dword purchase_selected_track(TrackSelection *screen) {
    pending::LocalFrontendString format, text;
    gui::string_reference(&format.value, 1427);
    gui::string_format(&text.value, &format.value, at<int>(screen->preferences, 7240),
                       selected_track_cost(screen));
    pending::ScopedMessageBox message(&text.value, 18);
    if (pending::run_message_box(&message.value, nullptr, 0) != 2)
        return 0;
    const Dword track = Dword(screen->track), direction = Dword(screen->direction);
    void *preferences = screen->preferences;
    at<Dword>(preferences, 7240) -= selected_track_cost(screen);
    auto *flag = bytes(preferences) + 7462 + track * 4 + direction;
    if (flag < bytes(preferences) + 7510)
        *flag = 1;
    ++at<Dword>(preferences, 7324);
    refresh_track_selection(screen);
    if (auto *images = static_cast<Widget *>(pending::find_child(screen, "<TRK_IL>")))
        if (invoke<Dword>(images, 2, menu_image_list_class_key))
            set_menu_image_enabled(
                family::image_cell_id(images, int(Dword(screen->direction) + Dword(screen->track) * 4)),
                1);
    return 1;
}
static Dword track_lap_choice(TrackSelection *screen) {
    if (!screen->laps)
        return 0;
    auto *items = &screen->laps->items;
    const auto *methods = items->methods;
    const Dword row = Dword(family::selected_list_row(items));
    using Cell = family::ListItem *(__thiscall *)(void *, Dword, Dword);
    auto *item = reinterpret_cast<Cell>(methods[9])(items, row, Dword(0));
    return item ? item->data : 0;
}
static void enqueue_selection_screen(const char *name, const char *type) {
    pending::LocalFrontendString screen_name(name), screen_type(type);
    pending::enqueue_screen(system(96), screen_name.release(), screen_type.release());
}

Dword __fastcall finish_track_selection(TrackSelection *screen, void *, Dword result, Dword) {
    if (result != 4) {
        if (result == 5) {
            if (screen->event_progress) {
                at<std::uint8_t>(system(72), 4) = 0;
                auto *playlist = screen->playlist;
                if (!track_playlist_empty(playlist)) {
                    remove_last_track_playlist(playlist);
                    refresh_track_selection(screen);
                    return 1;
                }
            }
            pop_frontend_screen(system(96));
        }
        stop_track_description(system(84));
        screen->base.modal_result = result;
        return 1;
    }
    void *event = screen->event_progress;
    if (!event || !screen->playlist || !track_playlist_full(screen->playlist)) {
        if (!at<std::uint8_t>(screen->preferences,
                              7462 + Dword(screen->track) * 4 + Dword(screen->direction))) {
            if (at<int>(screen->preferences, 7240) < int(selected_track_cost(screen)))
                track_purchase_error();
            else
                purchase_selected_track(screen);
            return 0;
        }
    }
    if (event) {
        auto *playlist = screen->playlist;
        if (!track_playlist_full(playlist)) {
            Dword laps = 1;
            if (at<std::uint8_t>(selected_track_record(screen), 24)) {
                laps = track_lap_choice(screen);
                if (!laps) {
                    refresh_track_selection(screen);
                    return 0;
                }
            }
            append_track_playlist(playlist, &screen->track_name, laps, Dword(screen->track),
                                  screen->direction);
            refresh_track_selection(screen);
            return 0;
        }
        at<std::uint8_t>(system(72), 4) = 1;
        for (Dword stage = 0; stage < (at<Dword>(event, 0) >> 1 & 7) + 2; ++stage) {
            Dword choice[7]{};
            read_track_playlist_row(playlist, stage, &choice[0], &choice[1], &choice[2]);
            pending::write_event_track(event, stage, choice);
        }
    }
    void *rules = system(72);
    if (at<Dword>(rules, 148) == 1) {
        Dword laps = 1;
        if (!at<Dword>(rules, 152)) {
            if (at<std::uint8_t>(selected_track_record(screen), 24)) {
                laps = track_lap_choice(screen);
                if (!laps)
                    return 0;
            }
        } else if (at<Dword>(rules, 152) != 5 ||
                   at<std::uint8_t>(selected_track_record(screen), 24))
            goto next_screen;
        network::packets::pending::set_game_rule(system(72), 7, laps);
    }
next_screen:
    stop_track_description(system(84));
    if (at<Dword>(system(72), 152) == 2) {
        pending::clear_event_progress(pending::active_event_progress(system(72)));
        enqueue_selection_screen("TourneyStatus", "TourneyStatus");
    } else
        enqueue_selection_screen("CarSelect", "CarSelectScreen");
    screen->base.modal_result = result;
    return 1;
}

Dword __cdecl track_next_pressed(void *sender, Dword event, Dword, Dword) {
    if (event == 43)
        if (auto *root = pending::widget_root(static_cast<Widget *>(sender)))
            if (invoke<Dword>(root, 2, interface_class_name))
                family::finish_modal(root, 4, 0);
    return 0;
}

Dword __cdecl track_previous_pressed(void *sender, Dword event, Dword, Dword) {
    if (event == 43)
        if (auto *root = pending::widget_root(static_cast<Widget *>(sender)))
            if (invoke<Dword>(root, 2, interface_class_name))
                family::finish_modal(root, 5, 0);
    return 0;
}

Dword __cdecl toggle_track_description(void *sender, Dword event, Dword, Dword modifiers) {
    if (event == 43 && !std::uint8_t(modifiers))
        if (auto *root = pending::widget_root(static_cast<Widget *>(sender)))
            if (invoke<Dword>(root, 2, track_selection_class_key)) {
                if (!game::audio::state::option<Dword>(audio_layout::speech_stream_active))
                    play_track_description(system(84),
                                           network::packets::rule_value(system(72), 5));
                else
                    stop_track_description(system(84));
            }
    return 0;
}
} // namespace nfs::application::frontend
