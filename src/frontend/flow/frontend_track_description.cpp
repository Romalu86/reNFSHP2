#include "frontend_track_selection.hpp"
#include "../../gui/lifecycle/gui_images.hpp"
#include "nfs/gui/resources/gui_format.hpp"
#include "nfs/audio/speech/speech.hpp"
#include <cstring>
namespace audio_layout = nfs::game::audio::state::layout;
namespace nfs::gui::engine::images {
void erase_character(String *, int);

void insert_string(String *text, int index, const String *insertion) {
    application::pending::LocalFrontendString left, right, first, result;
    string_substring(text, 0, index, &left.value);
    string_substring(text, index, text->length, &right.value);
    concatenate(&first.value, &left.value, insertion);
    concatenate(&result.value, &first.value, &right.value);
    string_copy(text, &result.value);
}
} // namespace nfs::gui::engine::images
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::at;
using game::runtime::bytes;
using game::runtime::invoke;
using game::runtime::system;

void stop_track_description(void *owner) {
    sound::streaming::stop_stream(at<Dword>(owner, 16));
}

void play_track_description(void *owner, Dword track) {
    if (!at<Dword>(owner, 20))
        return;
    if (game::audio::state::option<Dword>(audio_layout::speech_stream_active))
        sound::streaming::stop_stream(at<Dword>(owner, 16));
    const Dword masks[] = {4096, 8192, 16384, 64, 128, 256, 4, 2, 1, 512, 1024, 2048};
    speech::event_sink(speech::create_event_code(0, 0, 155), 1, track < 12 ? masks[track] : 0);
    speech::flush_events(0);
}
static void set_reference_text(Widget *widget, Dword reference) {
    pending::LocalFrontendString text;
    gui::string_reference(&text.value, reference);
    invoke<void>(widget, 7, &text.value);
}

void refresh_track_selection(TrackSelection *screen) {
    if (!screen->track_selector)
        return;
    if (screen->track != selected_type_spin_row(screen->track_selector)) {
        screen->track = selected_type_spin_row(screen->track_selector);
        if (at<Dword>(system(72), 152) != 1)
            populate_track_laps(screen);
    }
    gui::string_reference(&screen->track_name, at<Dword>(selected_track_record(screen), 12));
    family::select_sequence_frame(
        reinterpret_cast<family::ImageSequence *>(bytes(screen->track_images) + 328),
        screen->track);
    screen->direction = screen->direction_wheel->selection;
    if (game::audio::state::option<Dword>(audio_layout::speech_stream_active))
        stop_track_description(system(84));
    const int prior_track = at<int>(screen->preferences, 7512) / 4;
    if (screen->preview && (screen->preview_dirty || screen->track != prior_track)) {
        screen->preview_dirty = 0;
        pending::LocalFrontendString resource;
        gui::string_format_ascii(&resource.value, screen->track < 10 ? "tr0%d" : "tr%d",
                                 screen->track);
        release_preview_actor(screen->preview);
        pending::LocalFrontendString owned(&resource.value);
        set_preview_actor(screen->preview, owned.release());
    }
    network::packets::pending::set_game_rule(system(72), 5, Dword(screen->track));
    network::packets::pending::set_game_rule(system(72), 6, Dword(screen->direction));
    at<int>(screen->preferences, 7512) = int(Dword(screen->direction) + Dword(screen->track) * 4);
    network::packets::pending::select_track(static_cast<Dword *>(screen->tracks),
                                                   Dword(screen->track));
    const bool unlocked =
        at<std::uint8_t>(screen->preferences,
                         7462 + at<Dword>(selected_track_record(screen), 20) * 4 +
                             Dword(screen->direction)) != 0;
    if (!unlocked) {
        if (screen->lock) {
            set_enabled(screen->lock, 1);
            pending::set_widget_visible(screen->lock, 1);
            screen->lock->flags = 1;
        }
        if (screen->next)
            set_reference_text(screen->next, 1131);
    } else {
        if (screen->lock) {
            set_enabled(screen->lock, 0);
            pending::set_widget_visible(screen->lock, 0);
        }
        if (screen->next)
            set_reference_text(screen->next, 736);
    }
    if (screen->balance) {
        pending::LocalFrontendString text, number;
        gui::string_reference(&text.value, 722);
        gui::string_append_reference(&text.value, 733);
        gui::string_append_reference(&text.value, 733);
        gui::string_format_ascii(&number.value, "%d", at<int>(screen->preferences, 7240));
        gui::string_append(&text.value, &number.value);
        invoke<void>(screen->balance, 7, &text.value);
    }
    if (screen->cost) {
        if (!unlocked) {
            set_enabled(screen->cost, 1);
            pending::set_widget_visible(screen->cost, 1);
            pending::LocalFrontendString text, number;
            gui::string_reference(&text.value, 1231);
            gui::string_append_reference(&text.value, 722);
            gui::string_append_reference(&text.value, 733);
            gui::string_append_reference(&text.value, 733);
            gui::string_format_ascii(&number.value, "%d", selected_track_cost(screen));
            gui::string_append(&text.value, &number.value);
            invoke<void>(screen->cost, 7, &text.value);
        } else {
            set_enabled(screen->cost, 0);
            pending::set_widget_visible(screen->cost, 0);
        }
    }
    auto *next = static_cast<Widget *>(pending::find_child(screen, "<NEXT>"));
    if (next) {
        if (screen->event_progress && screen->playlist && !track_playlist_full(screen->playlist) &&
            unlocked) {
            set_reference_text(next, 1165);
            gui::string_copy(&next->tooltip,
                             &lifecycle::construction::unresolved::shared_string);
        } else {
            const bool complete =
                screen->event_progress && screen->playlist && track_playlist_full(screen->playlist);
            set_reference_text(next, unlocked || complete ? 565 : 1131);
            pending::LocalFrontendString tip;
            gui::string_reference(&tip.value, unlocked || complete ? 1577 : 1466);
            gui::string_copy(&next->tooltip, &tip.value);
        }
    }
    pending::LocalFrontendString text(&screen->track_name);
    gui::string_append_reference(&text.value, 722);
    gui::string_append_reference(&text.value, 733);
    gui::string_append_reference(&text.value, 733);
    auto *direction = static_cast<Widget *>(pending::find_child(screen, "<tdir>"));
    if (screen->direction >= 0 && screen->direction <= 3) {
        const Dword reference = 692 + Dword(screen->direction);
        gui::string_append_reference(&text.value, reference);
        if (direction)
            set_reference_text(direction, reference);
    }
    for (unsigned i = 0; i < 5; ++i)
        gui::string_append_reference(&text.value, 733);
    gui::string_append_reference(&text.value, 1335);
    gui::string_append_reference(&text.value, 722);
    for (unsigned i = 0; i < 2; ++i)
        gui::string_append_reference(&text.value, 733);
    gui::string_append_reference(&text.value, at<Dword>(selected_track_record(screen), 16));
    for (unsigned i = 0; i < 5; ++i)
        gui::string_append_reference(&text.value, 733);
    gui::string_append_reference(&text.value, 1476);
    gui::string_append_reference(&text.value, 722);
    for (unsigned i = 0; i < 2; ++i)
        gui::string_append_reference(&text.value, 733);
    pending::LocalFrontendString length(
        &lifecycle::construction::unresolved::shared_string);
    const Dword language = network::packets::rule_value(system(72), 17),
               metric = network::packets::rule_value(system(72), 44);
    const float distance = at<float>(selected_track_record(screen), 48);
    const auto format = kernel::numeric::current_format();
    const auto display = metric ? kernel::numeric::Extended(distance, format)
                                : kernel::numeric::Extended(distance, format) * 0.621504008769989013671875f;
    gui::string_format_ascii(&length.value, "%0.1f ", double(display));
    if (language >= 1 && language <= 5) {
        pending::LocalFrontendString separator;
        gui::string_reference(&separator.value, 720);
        gui::engine::images::erase_character(&length.value, length.value.length - 3);
        gui::engine::images::insert_string(&length.value, length.value.length - 2,
                                           &separator.value);
    }
    if (metric) {
        gui::string_append_reference(&length.value, 1477);
        gui::string_append_reference(&length.value, 733);
    }
    gui::string_append_reference(&length.value, metric ? 733 : 1478);
    gui::string_append(&text.value, &length.value);
    gui::string_append_reference(&text.value, 733);
    gui::string_append_reference(&text.value, 733);
    clear_ticker_messages(screen->description);
    for (unsigned i = 0; i < 4; ++i)
        queue_ticker_message(screen->description, &text.value, 1,
                             &screen->description->base.base.colour, 0, 100.f, 0);
}
} // namespace nfs::application::frontend
