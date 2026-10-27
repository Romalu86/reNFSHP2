#include "nfs/core/application/application.hpp"
#include "nfs/gui/engine/gui_engine.hpp"
#include "nfs/audio/vehicles/vehicle_audio.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include "nfs/physics/frame/simulation_runtime.hpp"
#include "nfs/audio/music/music_clients.hpp"
#include "nfs/core/timing/frame_clock.hpp"
namespace audio_layout = nfs::game::audio::state::layout;
namespace nfs::application::pending {
using game::runtime::at;
using game::runtime::invoke;
using game::runtime::system;
void *create_widget(const gui::String *, const gui::String *, const gui::String *,
                    void *);
void *find_child(void *, const char *);
void set_widget_visible(void *, Dword);
void bring_widget_to_front(void *);
void set_menu_camera(Byte);
void prepare_simulation_widgets(void *, void *);
void configure_player_widgets(void *, Dword, void *);
void configure_race_widgets(void *, Dword, void *);
Dword capture_replay_length(void *);
namespace {
struct LocalString {
    gui::String value;
    explicit LocalString(const char *text) {
        gui::string_construct_ascii(&value, text);
    }
    ~LocalString() {
        gui::string_destruct(&value);
    }
    LocalString(const LocalString &) = delete;
    LocalString &operator=(const LocalString &) = delete;
};
void *create_screen(const char *type, const char *name, void *parent = nullptr) {
    LocalString widget_type(type), widget_name(name), resource("System\\GInterface.gui");
    return create_widget(&resource.value, &widget_name.value, &widget_type.value, parent);
}
void add_chyron(void *screen) {
    if (!find_child(screen, "<CHYRON>"))
        create_screen("ChyronCtl", "<CHYRON>", screen);
}
void configure_letterbox(void *screen, Dword visible) {
    void *letterbox = find_child(screen, "<LETTERBOX>");
    if (!letterbox)
        letterbox = create_screen("HP2LetterBox", "<LETTERBOX>", screen);
    if (letterbox) {
        set_widget_visible(letterbox, visible);
        bring_widget_to_front(letterbox);
    }
}
void finish_screen(void *frontend, void *screen) {
    destroy_screen(screen);
    at<void *>(frontend, 40) = nullptr;
}
} // namespace

void pause_race_for_menu() {
    stop_force_feedback();
    frame_clock::paused = 1;
}

void resume_race_from_menu(Byte enabled) {
    game::audio::state::configuration<Dword>(audio_layout::music_muted) = enabled == 0;
    game::audio::restore_game_volume();
    frame_clock::paused = 0;
}

int run_starting_grid(void *frontend) {
    at<Dword>(system(72), 140) = 0;
    reset_network_race();
    void *screen = create_screen("HP2StartingGrid", "<STARTGRID>");
    if (!screen)
        return 0;
    invoke(at<void *>(system(92), 4), 68 / 4);
    at<void *>(frontend, 40) = screen;
    configure_letterbox(screen, 1);
    add_chyron(screen);
    set_menu_camera(1);
    const int result = gui::run_modal(screen, nullptr);
    set_menu_camera(0);
    finish_screen(frontend, screen);
    return result;
}

int run_simulation_screen(void *frontend) {
    void *screen = create_screen("SimGUIInterface", "SimGUI");
    if (!screen)
        return 0;
    at<Dword>(system(72), 140) = 0;
    at<void *>(frontend, 40) = screen;
    prepare_simulation_widgets(frontend, screen);
    configure_player_widgets(frontend, 1, screen);
    configure_race_widgets(frontend, 1, screen);
    configure_letterbox(screen, 0);
    add_chyron(screen);
    const int result = gui::run_modal(screen, nullptr);
    capture_replay_length(system(132));
    at<Dword>(system(104), 92) = 0;
    finish_screen(frontend, screen);
    return result;
}
namespace {
int run_results(void *frontend, const char *type, const char *name) {
    void *screen = create_screen(type, name);
    if (!screen)
        return 0;
    at<void *>(frontend, 40) = screen;
    add_chyron(screen);
    at<Dword>(frontend, 36) = 0;
    configure_letterbox(screen, 1);
    set_menu_camera(1);
    pause_race_for_menu();
    game::audio::stop_race_channels();
    const int result = gui::run_modal(screen, nullptr);
    set_menu_camera(0);
    resume_race_from_menu(1);
    finish_screen(frontend, screen);
    return result;
}
} // namespace

int run_race_results(void *frontend) {
    return run_results(frontend, "HP2RaceResults", "<RACERESULTS>");
}

int run_pursuit_results(void *frontend) {
    return run_results(frontend, "HP2CopVsPerpResults", "<COPVSPERPRESULTS>");
}

int run_end_game_menu(void *frontend) {
    void *screen = create_screen("HP2EndGame", "<ENDGAMEMENU>");
    if (!screen)
        return 0;
    at<void *>(frontend, 40) = screen;
    add_chyron(screen);
    set_menu_camera(1);
    pause_race_for_menu();
    game::audio::stop_race_channels();
    at<Dword>(frontend, 36) = 0;
    configure_letterbox(screen, 1);
    const int result = gui::run_modal(screen, nullptr);
    set_menu_camera(0);
    resume_race_from_menu(1);
    finish_screen(frontend, screen);
    return result;
}

int run_replay_controls(void *frontend) {
    void *screen = create_screen("HP2ReplayControls", "<REPLAYCONTROLS>");
    if (!screen)
        return 0;
    at<void *>(frontend, 40) = screen;
    add_chyron(screen);
    at<Dword>(system(72), 140) = 1;
    reset_network_race();
    at<Dword>(frontend, 36) = 0;
    Dword visible = 1;
    if (game::runtime::physics::split_screen() &&
        network::packets::rule_value(system(72), 38) == 1)
        visible = 0;
    configure_letterbox(screen, visible);
    gui::run_modal(screen, nullptr);
    finish_screen(frontend, screen);
    return 22;
}

void create_loading_screen(void *frontend) {
    show_cursor(0);
    void *owner = system(96);
    if (!at<Dword>(owner, 8)) {
        clear_frontend_views(owner);
        lifecycle::ui_pool = at<Dword>(owner, 52);
        create_frontend_view(owner, 0);
    }
    void *screen = create_screen("QuickRaceLoadingScreen", "QuickRaceLoad");
    at<void *>(frontend, 40) = screen;
    void *background = create_screen("GWidget",
                                     at<Dword>(system(72), 2312) == 1 ? "RaceLoadingBackgroundF"
                                                                        : "RaceLoadingBackground",
                                     screen);
    if (background)
        bring_widget_to_front(background);
    gui::tick_root(at<void *>(frontend, 40));
}
} // namespace nfs::application::pending
