#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "frontend_race_results.hpp"
#include "nfs/core/configuration/launch_options.hpp"
#include "nfs/gui/lifecycle/gui_modal.hpp"
#include "frontend_transition_math.hpp"
#include "nfs/core/lifecycle/array_construction.hpp"
#include <cstdlib>
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::invoke;
using game::runtime::system;
void stop_track_description(void *);
namespace {
void stop_race_playback() {
    game::audio::stop_race_channels();
    stop_track_description(system(84));
}
Dword confirm_race_transition(Widget *sender, Dword title_reference, Dword text_reference, Dword result) {
    Widget *parent = sender->parent;
    pending::MessageBox message;
    {
        pending::LocalFrontendString title, text;
        gui::string_reference(&title.value, title_reference);
        gui::string_reference(&text.value, text_reference);
        pending::construct_message_box(&message, &text.value, &title.value, 81986);
    }
    try {
        if (parent && pending::run_message_box(&message, nullptr, 0) == 2) {
            stop_race_playback();
            family::finish_modal(parent, result, 0);
        }
    } catch (...) {
        pending::destroy_message_box(&message);
        throw;
    }
    pending::destroy_message_box(&message);
    return 0;
}
} // namespace

static Dword __cdecl resume_race_button(void *sender, Dword, Dword, Dword) {
    if (Widget *parent = static_cast<Widget *>(sender)->parent)
        family::finish_modal(parent, 2, 0);
    return 0;
}

static Dword __cdecl quit_race_button(void *sender, Dword, Dword, Dword) {
    return confirm_race_transition(static_cast<Widget *>(sender), 437, 458, 19);
}

static Dword __cdecl confirm_restart_button(void *sender, Dword, Dword, Dword) {
    return confirm_race_transition(static_cast<Widget *>(sender), 436, 457, 17);
}

static Dword __cdecl confirm_replay_button(void *sender, Dword, Dword, Dword) {
    return confirm_race_transition(static_cast<Widget *>(sender), 1230, 458, 18);
}

static Dword __cdecl enable_replay_button(void *sender, Dword event, Dword, Dword) {
    if (event == 18 || event == 21) {
        set_enabled(static_cast<Widget *>(sender), transition_math::replay_available(game::race_ticks));
    }
    return 0;
}

static Dword __cdecl quit_to_system_button(void *sender, Dword, Dword, Dword) {
    if (Widget *parent = static_cast<Widget *>(sender)->parent)
        family::finish_modal(parent, 6, 0);
    return 0;
}

static Dword __cdecl restart_race_button(void *sender, Dword, Dword, Dword) {
    if (Widget *parent = static_cast<Widget *>(sender)->parent) {
        stop_race_playback();
        family::finish_modal(parent, 17, 0);
    }
    return 0;
}

static Dword __cdecl view_replay_button(void *sender, Dword, Dword, Dword) {
    if (Widget *parent = static_cast<Widget *>(sender)->parent) {
        stop_race_playback();
        family::finish_modal(parent, 18, 0);
    }
    return 0;
}

static Dword __cdecl continue_race_button(void *sender, Dword, Dword, Dword) {
    if (Widget *parent = static_cast<Widget *>(sender)->parent) {
        stop_race_playback();
        family::finish_modal(parent, 19, 0);
    }
    return 0;
}

static Dword __cdecl enable_rear_view_button(void *sender, Dword, Dword, Dword) {
    set_enabled(static_cast<Widget *>(sender), !game::runtime::physics::split_screen() &&
                 game::runtime::at<Byte>(system(32), 4) != 0);
    return 0;
}

static Dword __cdecl label_player_vibration(void *sender, Dword, Dword, Dword) {
    pending::LocalFrontendString text;
    gui::string_reference(&text.value, game::runtime::physics::split_screen() ? 455 : 338);
    invoke<void>(sender, 7, &text.value);
    return 0;
}

static Dword __cdecl exit_race_to_windows(void *sender, Dword, Dword, Dword) {
    if (!sender || !invoke<Dword>(sender, 2, family::button_class_key))
        return 0;
    auto *focused = static_cast<Widget *>(gui::focused_widget());
    if (!focused)
        return 0;
    if (!launch::field<Dword>(8627776)) {
        pending::MessageBox message;
        {
            pending::LocalFrontendString title, text;
            gui::string_reference(&title.value, 559);
            gui::string_reference(&text.value, 743);
            pending::construct_message_box(&message, &text.value, &title.value, 81986);
        }
        int result;
        try {
            result = pending::run_message_box(&message, nullptr, 0);
        } catch (...) {
            pending::destroy_message_box(&message);
            throw;
        }
        pending::destroy_message_box(&message);
        if (result != 2)
            return 0;
    }
    family::finish_modal(focused, 6, 0);
    return 0;
}
struct RaceButtonDefinition {
    const char *name;
    Dword event;
    const char *event_name;
    Callback callback;
};
static const RaceButtonDefinition race_button_definitions[] = {
    {"Pause_ExitToWindows", 43, "BEVENT_PRESSED", exit_race_to_windows},
    {"Pause_LabelP1Vibration", 18, "GEVENT_BEGIN", label_player_vibration},
    {"Pause_ResumeRace", 43, "BEVENT_PRESSED", resume_race_button},
    {"Pause_QuitRace", 43, "BEVENT_PRESSED", quit_race_button},
    {"Pause_Restart", 43, "BEVENT_PRESSED", confirm_restart_button},
    {"Pause_QuitToReplay", 43, "BEVENT_PRESSED", confirm_replay_button},
    {"Pause_Enable_QuitToReplay", 21, "GEVENT_IDLE", enable_replay_button},
    {"Pause_Enable_QuitToReplay", 18, "GEVENT_BEGIN", enable_replay_button},
    {"Pause_EnableRearView", 18, "GEVENT_BEGIN", enable_rear_view_button},
    {"Pause_QuitToSystem", 43, "BEVENT_PRESSED", quit_to_system_button},
    {"EndGame_RestartRace", 43, "BEVENT_PRESSED", restart_race_button},
    {"EndGame_ViewReplay", 43, "BEVENT_PRESSED", view_replay_button},
    {"EndGame_Continue", 43, "BEVENT_PRESSED", continue_race_button},
    {"HP2StartingGrid::RaceButton", 18, "GEVENT_BEGIN", starting_grid_race_button},
    {"HP2RaceResults::NextButton", 18, "GEVENT_BEGIN", race_results_next_button}};
static CallbackRecord race_button_records[15];

static void unregister_race_buttons() {
    lifecycle::construction::objects::destroy_array(race_button_records, 36, 15,
                                                     unregister_callback);
}

static void register_race_buttons() {
    for (unsigned i = 0; i < 15; ++i) {
        const auto &definition = race_button_definitions[i];
        register_callback(&race_button_records[i], family::button_class_key,
            definition.event_name, definition.event, definition.name, definition.callback, 0);
    }
    std::atexit(unregister_race_buttons);
}
namespace {
NFS_KERNEL_INITIALIZER(startup_register_race_buttons, ".CRT$XCU_NFS_006900FC",
                       register_race_buttons);
}
} // namespace nfs::application::frontend
