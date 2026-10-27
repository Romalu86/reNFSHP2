#include "nfs/audio/music/music_clients.hpp"
#include "nfs/audio/backend/sound_ownership.hpp"
namespace audio_layout = nfs::game::audio::state::layout;
namespace nfs::game::audio {
using namespace state;

Byte car_environment(void *car) {
    configuration<Dword>(1700) = configuration<Dword>(1692);
    configuration<Dword>(1704) = configuration<Dword>(1696);
    return sound::field<Byte>(sound::field<void *>(car, 1768), 584);
}

void apply_reverb_preset(int preset) {
    Byte &initialized = world<Byte>(6912);
    int &hardware_level = world<int>(6908), &software_level = world<int>(6904);
    if (!(initialized & 1)) {
        initialized |= 1;
        hardware_level = configuration<int>(1700);
    }
    if (!(initialized & 2)) {
        initialized |= 2;
        software_level = configuration<int>(1704);
    }
    if (!configuration<Dword>(audio_layout::audio_enabled))
        return;
    int &current = configuration<int>(audio_layout::reverb_preset);
    auto hardware = [&](int index) -> Dword {
        return configuration<Dword>(1708 + Dword(index) * 4);
    };
    auto software = [&](int index) -> Dword {
        return configuration<Dword>(1740 + Dword(index) * 4);
    };
    Dword &hardware_active = sound::field<Dword>(effects_storage, 56),
         &software_active = sound::field<Dword>(effects_storage, 60);
    if (current != preset) {
        bool first = current == -1;
        if (first)
            current = preset;
        if (!software_active || software_level < 1) {
            Dword effect = hardware(preset);
            if ((hardware(current) != effect || first) && configuration<Dword>(1688)) {
                hardware_level = 0;
                if (!effect)
                    sound::backend::configure_effect_groups(128, 0, 0, Dword(-1), Dword(-1));
                else {
                    sound::backend::refresh_effect_group(128, 0);
                    sound::backend::configure_effect_groups(128, 0, hardware(preset), Dword(-1),
                                                           Dword(-1));
                }
                hardware_active = effect != 0;
                sound::backend::refresh_effect_group(128, Dword(hardware_level));
            }
            if (software(current) != software(preset) || first) {
                software_level = 0;
                if (!software(preset)) {
                    sound::backend::configure_effect_groups(16, 0, 0, Dword(-1), Dword(-1));
                    software_active = 0;
                } else if (sound::field<Dword>(effects_storage, 4 + Dword(preset) * 4)) {
                    sound::backend::refresh_effect_group(16, 0);
                    sound::backend::configure_effect_groups(16, 0, 0, Dword(-1), Dword(-1));
                    sound::backend::configure_effect_groups(
                        16, 0, 1, sound::field<Dword>(effects_storage, 4 + Dword(preset) * 4),
                        Dword(-1));
                    software_active = 1;
                }
                sound::backend::refresh_effect_group(16, Dword(software_level));
            }
            current = preset;
        } else {
            software_level = int(Dword(software_level) - Dword(configuration<int>(1704) >> 4));
            if (software_level < 0)
                software_level = 0;
            sound::backend::refresh_effect_group(16, Dword(software_level));
            return;
        }
    }
    if (hardware_level < configuration<int>(1700) && hardware_active &&
        configuration<Dword>(1688)) {
        hardware_level = int(Dword(hardware_level) + Dword(configuration<int>(1700) >> 2));
        if (configuration<int>(1700) < hardware_level)
            hardware_level = configuration<int>(1700);
        sound::backend::refresh_effect_group(128, Dword(hardware_level));
    }
    if (configuration<int>(1704) <= software_level || !software_active)
        return;
    software_level = int(Dword(software_level) + Dword(configuration<int>(1704) >> 4));
    if (configuration<int>(1704) < software_level)
        software_level = configuration<int>(1704);
    sound::backend::refresh_effect_group(16, Dword(software_level));
}
} // namespace nfs::game::audio
