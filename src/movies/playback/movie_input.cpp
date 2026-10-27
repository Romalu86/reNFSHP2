#include "movie_runtime.hpp"
#include "nfs/input/directinput/directinput.hpp"
#include "nfs/core/frame/game_step.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include "nfs/input/window/window_input.hpp"
namespace nfs::movies {
// Keep the primary window handle and the input-state word separate.
HWND foreground_owner() {
    return window_input::window_state.handle;
}

Byte any_input() {
    if (graphics::device) {
        game::keyboard_snapshot = reinterpret_cast<Dword>(input::keyboard_get(6));
        void *manager = game::runtime::system(100);
        Dword count = reinterpret_cast<Dword>(input::device_get(0, 1));
        for (Dword index = 0; index < count; ++index)
            if (index < 32 && field<Byte>(manager, 1012 + index * 8))
                game::set_device_snapshot(manager, index,
                                          reinterpret_cast<Dword>(input::device_get(int(index), 6)));
    }
    Byte pressed = 0;
    auto *keys = reinterpret_cast<const Byte *>(game::keyboard_snapshot);
    for (int index = 0; index < 256; ++index)
        if (keys[index]) {
            pressed = 1;
            break;
        }
    void *mouse = input::mouse_get(6);
    if (!mouse)
        pressed = 0;
    // The saved null-mouse branch still reaches these four byte reads.
    for (int index = 0; index < 4; ++index)
        if (field<Byte>(mouse, 12 + index))
            pressed = 1;
    return pressed;
}
} // namespace nfs::movies
