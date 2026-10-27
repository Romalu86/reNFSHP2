#include "frontend_simulation.hpp"
#include "nfs/graphics/resources/eagl_graphics.hpp"
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::at;
using game::runtime::invoke;
using game::runtime::system;
void stop_track_description(void *);
struct ReplaySpeed {
    const char *name;
    float multiplier;
};
static constexpr ReplaySpeed replay_speeds[] = {
    {"1/8x", .125f}, {"1/4x", .25f}, {"1/2x", .5f}, {"1x", 1.f}, {"2x", 2.f}, {"4x", 4.f}};
namespace {
void set_replay_label(Widget *widget, Dword reference) {
    pending::LocalFrontendString text;
    gui::string_reference(&text.value, reference);
    invoke<void>(widget, 7, &text.value);
}
void eject_replay(ReplayControls *screen) {
    game::audio::stop_race_channels();
    stop_track_description(system(84));
    family::finish_modal(&screen->base, 22, 0);
}
void toggle_replay_pause(ReplayControls *screen) {
    if (!frame_clock::paused) {
        pending::pause_race_for_menu();
        game::audio::mute_game_audio();
        if (screen->pause_play)
            set_replay_label(screen->pause_play, 1434);
    } else {
        pending::resume_race_from_menu(1);
        if (screen->pause_play)
            set_replay_label(screen->pause_play, 1433);
    }
}
} // namespace

void __fastcall initialize_replay_controls(ReplayControls *screen, void *) {
    family::dialog_initialize(&screen->base, nullptr);
    at<Dword>(screen, 272) = 0;
    screen->speed = 3;
    frame_clock::step = 1;
    screen->pause_play = static_cast<Widget *>(pending::find_child(screen, "<PAUSE_PLAY>"));
    if (screen->pause_play)
        set_replay_label(screen->pause_play, 1433);
    screen->speed_label = static_cast<Widget *>(pending::find_child(screen, "<SPEED>"));
    if (screen->speed_label) {
        pending::LocalFrontendString text(replay_speeds[screen->speed].name);
        invoke<void>(screen->speed_label, 7, &text.value);
    }
    screen->eject = static_cast<Widget *>(pending::find_child(screen, "<EJECT>"));
    screen->faster = static_cast<Widget *>(pending::find_child(screen, "<FASTER>"));
    screen->slower = static_cast<Widget *>(pending::find_child(screen, "<SLOWER>"));
    show_cursor(1);
}
 
void __fastcall release_replay_controls(ReplayControls *screen, void *) {
    clear_layout_request(&screen->base, nullptr);
}
// Also inlined in saved child/key handlers.
static void change_replay_speed(ReplayControls *screen, int delta) {
    const int previous = screen->speed;
    int speed = std::int32_t(Dword(previous) + Dword(delta));
    if (speed < 0)
        speed = 0;
    else if (speed > 5)
        speed = 5;
    frame_clock::step = replay_speeds[speed].multiplier;
    screen->speed = speed;
    if (frame_clock::step <= 0)
        frame_clock::step = 0;
    if (screen->speed_label) {
        pending::LocalFrontendString text(replay_speeds[speed].name);
        invoke<void>(screen->speed_label, 7, &text.value);
    }
    if (delta < 0 && screen->speed != previous)
        frame_clock::elapsed = static_cast<float>(
            static_cast<long double>(std::int32_t(graphics::unresolved::camera_state)) *
            0.01666666753590106964111328125f);
}

void __fastcall replay_controls_child_event(ReplayControls *screen, void *, Widget *sender,
                                           Dword event, Dword, Dword) {
    if (event != 43)
        return;
    if (sender == screen->eject)
        eject_replay(screen);
    else if (sender == screen->faster)
        change_replay_speed(screen, 1);
    else if (sender == screen->slower)
        change_replay_speed(screen, -1);
    else if (sender == screen->pause_play)
        toggle_replay_pause(screen);
}

Dword __fastcall replay_controls_key(ReplayControls *screen, void *, Dword key, Dword modifiers) {
    switch (key) {
    case 1: eject_replay(screen); break;
    case 14:
    case 205: change_replay_speed(screen, 1); break;
    case 28:
    case 57: toggle_replay_pause(screen); break;
    case 203:
    case 211: change_replay_speed(screen, -1); break;
    default: return family::dialog_key_down(&screen->base, nullptr, key, modifiers);
    }
    return 1;
}

void __fastcall tick_replay_controls(ReplayControls *screen, void *, Dword ticks) {
    family::dialog_tick(&screen->base, nullptr, ticks);
    change_replay_speed(screen, 0);
    if (!screen->base.modal_result) {
        if (at<Dword>(system(96), 36) == 23) {
            game::audio::stop_race_channels();
            family::finish_modal(&screen->base, at<Dword>(system(96), 36), 0);
        } else {
            game::advance();
        }
    }
}

void __fastcall draw_replay_controls(ReplayControls *, void *, void *engine) {
    invoke<void>(engine, 56, Dword(6), Dword(0), Dword(0));
    draw_race_frame();
    invoke<void>(engine, 56, Dword(5), Dword(0), Dword(0));
}
} // namespace nfs::application::frontend
