#include "nfs/audio/music/music_clients.hpp"
#include "nfs/audio/backend/sound_ownership.hpp"
#include "nfs/input/window/window_input.hpp"
#include "nfs/core/timing/timer.hpp"
namespace audio_layout = nfs::game::audio::state::layout;
namespace nfs::game::audio {
using namespace state;

void initialize_audio_device() {
    int result = sound::backend::load_configuration(audio_settings);
    if (result >= 0 && configuration<Dword>(audio_layout::audio_device_enabled) && !disable_audio) {
        sound::field<Byte>(audio_settings, 64) = sound::field<Byte>(audio_settings, 5) > 2;
        sound::field<Byte>(audio_settings, 72) = sound::field<Byte>(audio_settings, 5);
        sound::field<Byte>(audio_settings, 77) = 3;
        sound::field<std::uint16_t>(audio_settings, 80) = 145;
        sound::field<std::uint16_t>(audio_settings, 82) = 36;
        sound::field<std::uint16_t>(audio_settings, 84) = 146;
        sound::field<Byte>(audio_settings, 49) = sound::field<Byte>(audio_settings, 10);
        if (sound::field<Byte>(audio_settings, 12))
            sound::field<Byte>(audio_settings, 62) = 1;
        sound::field<Byte>(audio_settings, 46) = sound::field<Byte>(audio_settings, 6);
        int device = sound::field<int>(lifecycle::manager->slots[18], 2360);
        sound::field<std::uint16_t>(audio_settings, 40) = 73;
        if (sound::field<Byte>(audio_settings, 10) < 16 || device < 3) {
            sound::field<Byte>(audio_settings, 72) = Byte((device != 2) + 1);
            sound::field<Byte>(audio_settings, 64) = 0;
            sound::field<Byte>(audio_settings, 77) = 1;
            sound::field<std::uint16_t>(audio_settings, 80) = 36;
            sound::field<Byte>(audio_settings, 62) = 0;
        }
        sound::field<std::uint16_t>(audio_settings, 42) =
            sound::field<std::uint16_t>(audio_settings, 2);
        configuration<Dword>(1688) = sound::field<Byte>(audio_settings, 62) != 0;
        sound::backend::apply_configuration(audio_settings);
        sound::backend::register_audio_tasks();
        if (sound::backend::initialize_audio(audio_heap, 409600) >= 0)
            return;
        configuration<Dword>(audio_layout::audio_device_enabled) = 0;
    }
    configuration<Dword>(audio_layout::audio_enabled) = 0;
}
// The saved  getter reads the field before the child HWND.
Dword update_window_audio(Dword, Dword) {
    HWND window = window_input::window_state.handle;
    if (!IsIconic(window) && GetActiveWindow() == window && !configuration<Dword>(audio_layout::music_muted))
        sound::set_master_volume(120);
    else
        sound::set_master_volume(0);
    return 0;
}
} // namespace nfs::game::audio
