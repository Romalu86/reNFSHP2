#include "nfs/graphics/display/graphics_error.hpp"
#include "nfs/graphics/resources/eagl_graphics.hpp"
#include "nfs/graphics/configuration/display_configuration.hpp"
#include "nfs/input/window/window_input.hpp"
#include "nfs/audio/backend/sound_runtime.hpp"
#include "nfs/core/frame/game_step.hpp"
#include "nfs/network/session/network_runtime.hpp"
namespace nfs::graphics {

kernel::Dword refresh_audio_focus(kernel::Dword, kernel::Dword) {
    HWND window = window_input::window_state.handle;
    if (!IsIconic(window) && GetActiveWindow() == window &&
        !pending::audio_focus_disabled) {
        sound::set_master_volume(120);
        return 0;
    }
    sound::set_master_volume(0);
    return 0;
}

void suspend_rendering() {
    switch (mode) {
    case 1:
    case 3: {
        auto *vehicles = reinterpret_cast<std::uint8_t *>(lifecycle::manager->slots[52 / 4]);
        mode = 4;
        startup::cursor_clipped = 0;
        ClipCursor(nullptr);
        if (!vehicles[81296]) {
            auto *client = reinterpret_cast<Dword *>(lifecycle::manager->slots[112 / 4]);
            if (client[68 / 4])
                network::disconnect_client(client, 16);
            auto *network = reinterpret_cast<Dword *>(lifecycle::manager->slots[108 / 4]);
            if (network[44 / 4] && (!network[44 / 4] || !network[48 / 4]))
                nfs::network::leave_game(network);
        }
        if (gamma_saved) {
            HWND desktop = GetDesktopWindow();
            HDC dc = GetWindowDC(desktop);
            SetDeviceGammaRamp(dc, gamma);
            ReleaseDC(desktop, dc);
        }
        refresh_audio_focus(0, 0);
        return;
    }
    case 4: {
        auto *network = reinterpret_cast<Dword *>(lifecycle::manager->slots[108 / 4]);
        mode = 5;
        if (network[44 / 4]) {
            auto *vehicles = reinterpret_cast<std::uint8_t *>(lifecycle::manager->slots[52 / 4]);
            if (!vehicles[81296] && network[44 / 4] && network[48 / 4])
                game::advance();
        }
        break;
    }
    case 5: {
        auto *network = reinterpret_cast<Dword *>(lifecycle::manager->slots[108 / 4]);
        if (network[44 / 4]) {
            auto *vehicles = reinterpret_cast<std::uint8_t *>(lifecycle::manager->slots[52 / 4]);
            if (!vehicles[81296] && network[44 / 4] && network[48 / 4])
                game::advance();
        }
        break;
    }
    default:
        return;
    }
    refresh_audio_focus(0, 0);
}
} // namespace nfs::graphics
