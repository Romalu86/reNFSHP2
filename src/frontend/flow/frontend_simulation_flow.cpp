#include "frontend_simulation.hpp"
#include "../../network/session/network_lifecycle_runtime.hpp"
namespace nfs::game::world::actor_resources {
void retire_finished_actors(void *);
}
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::at;
using game::runtime::invoke;
using game::runtime::system;
// Complete body is RET4.
void finish_controller_menu(void *, Dword) {
}
// Complete body is RET4.
void begin_controller_menu(void *, Dword) {
}
// The saved incoming receiver is not used.
static void retire_cinematic_actors(void *) {
    game::world::actor_resources::retire_finished_actors(system(48));
}

int run_pause_overlay(void *frontend, Widget *parent) {
    auto *name = reinterpret_cast<gui::String *>(game::runtime::bytes(frontend) + 16);
    {
        pending::LocalFrontendString pause("<PAUSE>");
        gui::string_copy(name, &pause.value);
    }
    int result = 4;
    while (!gui::string_equal(name, &lifecycle::construction::unresolved::shared_string)) {
        void *screen;
        {
            pending::LocalFrontendString type("HP2Pause"), resource("System\\GInterface.gui");
            screen = pending::create_widget(&resource.value, name, &type.value, parent);
        }
        if (!screen) {
            result = 3;
            break;
        }
        gui::string_copy(name, &lifecycle::construction::unresolved::shared_string);
        result = gui::run_modal(screen, nullptr);
        pending::destroy_screen(screen);
    }
    pending::configure_player_widgets(frontend, 1, parent);
    return result;
}
// Only EAX0 and the pause-byte write are defined by this callback.
Dword __cdecl begin_simulation(void *, Dword, Dword, Dword) {
    frame_clock::paused = 0;
    return 0;
}

void __fastcall initialize_simulation(Widget *screen, void *) {
    gui::widgets::pending::request_interface_layout(screen, nullptr);
    pending::set_widget_callback(screen, 19, begin_simulation);
}
 
void __fastcall release_simulation(Widget *screen, void *) {
    clear_layout_request(screen, nullptr);
}

Dword __fastcall simulation_key_up(Widget *screen, void *, Dword key, Dword modifiers) {
    if (network::session_active() && key == 15)
        game::phase_pending = 0;
    return style_result_34(screen, nullptr, key, modifiers);
}

Dword __fastcall simulation_key_down(Widget *screen, void *, Dword key, Dword modifiers) {
    if (network::session_active()) {
        if (!Byte(modifiers)) {
            switch (key) {
            case 15:
                if (!game::phase_pending)
                    gui::events::propagate(at<void *>(system(96), 40), 112, 0, 0, 2);
                game::phase_pending = 1;
                break;
            case 1:
                gui::events::propagate(at<void *>(system(96), 40), 113, 0, 0, 2);
                break;
            case 14:
                game::phase_reset = game::phase_reset == 0;
                break;
            case 41:
                if (!game::runtime::reset_requested && !game::runtime::reset_complete) {
                    game::runtime::reset_complete = 1;
                    game::phase_reset = 0;
                } else if (game::runtime::reset_complete) {
                    game::runtime::reset_complete = 0;
                }
                break;
            }
        }
    } else {
        const Byte cinematic = game::runtime::ai::lifecycle::state<Byte>(7353316);
        if ((key == 1 || ((key == 28 || key == 57) && cinematic)) && !Byte(modifiers)) {
            if (cinematic) {
                retire_cinematic_actors(game::runtime::ai::pending::path_controller);
                return gui::widgets::pending::interface_key(screen, nullptr, key, modifiers);
            }
            pending::set_menu_camera(1);
            pending::pause_race_for_menu();
            game::audio::mute_game_audio();
            // Signed divide-by2^28 followed by a nibble mask.
            const std::int32_t packed = std::int32_t(modifiers);
            const Dword controller = Dword(packed / 268435456) & 15;
            begin_controller_menu(system(96), controller);
            const int result = run_pause_overlay(system(96), screen);
            finish_controller_menu(system(96), 1);
            if (result != 2 && result != 3)
                family::finish_modal(screen, Dword(result), 0);
            pending::resume_race_from_menu(result != 19);
            pending::set_menu_camera(0);
        }
    }
    return gui::widgets::pending::interface_key(screen, nullptr, key, modifiers);
}

Dword __fastcall simulation_event(Widget *screen, void *, Dword event, Dword first, Dword second) {
    switch (event) {
    case 110:
        network::session::show_connection_error(system(112), at<void *>(system(96), 40), 0);
        if (void *frontend = system(96))
            at<Dword>(frontend, 36) = 19;
        break;
    case 111: network::session::show_client_car_dialog(system(112)); break;
    case 112: network::session::show_network_statistics(system(104)); break;
    case 113: network::session::show_network_options(system(104)); break;
    case 114: network::session::show_server_setup(system(104)); break;
    case 115: network::session::show_network_hud_options(system(104)); break;
    case 116: network::session::show_network_audio_options(system(104)); break;
    }
    return gui::widgets::pending::handle_event(screen, nullptr, event, first, second);
}

void __fastcall tick_simulation(Widget *screen, void *, Dword) {
    void *frontend = system(96);
    if (frontend && at<Byte>(frontend, 104))
        return;
    if (screen->modal_result)
        return;
    // The path dereferences frontend after the initial nullable flag probe.
    if (const Dword result = at<Dword>(frontend, 36)) {
        family::finish_modal(screen, result, 0);
    } else
        game::advance();
}

void __fastcall draw_simulation(Widget *, void *, void *engine) {
    invoke<void>(engine, 56, Dword(6), Dword(0), Dword(0));
    draw_race_frame();
    invoke<void>(engine, 56, Dword(5), Dword(0), Dword(0));
}
} // namespace nfs::application::frontend
