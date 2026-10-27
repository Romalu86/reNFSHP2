#include "nfs/gui/engine/gui_engine.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include "nfs/input/window/window_input.hpp"
#include "nfs/graphics/configuration/render_settings.hpp"
#include "nfs/graphics/configuration/display_configuration.hpp"
#include "nfs/core/frame/game_step.hpp"
#include "nfs/core/timing/frame_clock.hpp"
#include "nfs/audio/vehicles/vehicle_audio.hpp"
#include "nfs/core/application/application.hpp"
#include "../../math/precision/extended_float.hpp"
namespace nfs::gui {
void reset_timer_deadlines(Dword);
}
namespace nfs::gui::engine::pending {
using game::runtime::at;
using game::runtime::invoke;
using game::runtime::system;
// The two virtual calls bracket GUI drawing, not simulation substitutes.
void __fastcall tick_root(Engine *engine, void *, void *root) {
    if (!engine->context || !root || (at<Dword>(system(96), 8) && at<Dword>(system(96), 36)))
        return;
    if (root != gui::focused_widget() || !at<Dword>(root, 220)) {
        invoke(engine, 296 / 4);
        draw_root(root, engine, 1);
        invoke(engine, 304 / 4);
        finish_window_frame();
        finish_input_frame();
    }
}

void finish_input_frame() {
    if (graphics::startup::fullscreen(&graphics::startup::settings)) {
        if (window_input::hook_get_values && lifecycle::windows_hook) {
            Dword first, second, third, fourth;
            using GetValues = void(__cdecl *)(Dword *, Dword *, Dword *, Dword *);
            reinterpret_cast<GetValues>(window_input::hook_get_values)(&first, &second, &third,
                                                                       &fourth);
        }
        window_input::hook_snapshot_valid = 0;
    }
}

void finish_window_frame() {
    if (graphics::mode != 1) {
        if (graphics::mode < 4 || graphics::mode > 5)
            return;
        graphics::mode = 6;
        gui::reset_timer_deadlines(0);
        const Byte frontend = at<Byte>(system(52), 81296);
        if (!graphics::startup::fullscreen(&graphics::startup::settings)) {
            graphics::startup::cursor_clipped = 0;
            ClipCursor(nullptr);
        } else {
            RECT bounds{0, 0, graphics::startup::width(&graphics::startup::settings, frontend),
                        graphics::startup::height(&graphics::startup::settings, frontend)};
            graphics::startup::cursor_clipped = ClipCursor(&bounds) != FALSE;
        }
        if (!frontend) {
            if (!at<Dword>(system(112), 68) && !at<Dword>(system(108), 44)) {
                frame_clock::elapsed = float(kernel::numeric::Extended::integer(
                                                 int(graphics::unresolved::camera_state),
                                                 kernel::numeric::current_format()) *
                                             0.01666666753590106964111328125f);
                application::synchronize_cursor();
                graphics::mode = 3;
                return;
            }
            if ((!at<Dword>(system(108), 44) || !at<Dword>(system(108), 48)) && system(96))
                at<Dword>(system(96), 36) = 19;
        }
        application::synchronize_cursor();
    }
    graphics::mode = 3;
}
} // namespace nfs::gui::engine::pending
