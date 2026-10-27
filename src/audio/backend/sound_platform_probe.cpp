#include "nfs/audio/backend/sound_ownership.hpp"
extern "C" __declspec(dllimport) long __stdcall DirectSoundCreate(const void *, void **, void *);
namespace nfs::sound::backend {
extern const Byte effect_capabilities_guid[16];

void *foreground_window() {
    HWND window = GetForegroundWindow();
    return window ? window : GetDesktopWindow();
}

void release_effect_interface() {
    for (unsigned va : {8717636u, 8717640u}) {
        auto &object = platform_tail<void *>(va);
        if (object) {
            device_call<Dword>(object, 8);
            object = nullptr;
        }
    }
    use_effect_interface = 0;
}

void probe_effect_interface(int enabled) {
    release_effect_interface();
    alignas(4) Byte description[36], wave[18];
    zero_memory(description, 36);
    field<std::uint16_t>(wave, 0) = 1;
    field<std::uint16_t>(wave, 2) = 1;
    field<Dword>(wave, 4) = 11025;
    field<Dword>(wave, 8) = 22050;
    field<std::uint16_t>(wave, 12) = 2;
    field<std::uint16_t>(wave, 14) = 16;
    field<std::uint16_t>(wave, 16) = 0;
    field<Dword>(description, 0) = 36;
    field<Dword>(description, 4) = 32788;
    field<Dword>(description, 8) = 128;
    field<void *>(description, 16) = wave;
    auto &buffer = platform_tail<void *>(8717640), &effect = platform_tail<void *>(8717636);
    if (device_call<int>(direct_sound, 12, description, &buffer, 0))
        return;
    if (device_call<int>(buffer, 0, effect_iid, &effect)) {
        device_call<Dword>(buffer, 8);
        buffer = nullptr;
        return;
    }
    if (enabled) {
        Dword capabilities;
        if (!device_call<int>(effect, 20, effect_capabilities_guid, 0, &capabilities) &&
            (capabilities & 3) == 3) {
            use_effect_interface = 1;
            return;
        }
    }
    if (!use_effect_interface)
        release_effect_interface();
}

int probe_device(Dword *channels, int *effects) {
    *effects = 0;
    int result = int(DirectSoundCreate(nullptr, &direct_sound, nullptr));
    if (result) {
        direct_sound = nullptr;
        return Dword(result) == 2289565706u ? -18 : Dword(result) == 2289565816u ? -10 : -1;
    }
    using SetCooperative = int(__stdcall *)(void *, void *, int);
    auto set_cooperative =
        reinterpret_cast<SetCooperative>(field<Dword *>(direct_sound, 0)[24 / 4]);
    void *window = foreground_window();
    set_cooperative(direct_sound, window, 2);
    alignas(4) Byte caps[96], description[36];
    field<Dword>(caps, 0) = 96;
    if (device_call<int>(direct_sound, 16, caps))
        result = -1;
    else {
        field<Dword>(channels, 0) = field<Dword>(caps, 44);
        platform<Byte>(8711392) = (field<Byte>(caps, 4) >> 5) & 1;
        zero_memory(description, 36);
        field<Dword>(description, 0) = 36;
        field<Dword>(description, 4) = 17;
        if (!device_call<int>(direct_sound, 12, description, &platform<void *>(8711372),
                              0)) {
            probe_effect_interface(1);
            *effects = std::int8_t(use_effect_interface);
            release_effect_interface();
        }
    }
    if (direct_sound)
        device_call<Dword>(direct_sound, 8);
    direct_sound = nullptr;
    return result;
}

int release_output_device() {
    release_effect_interface();
    if (direct_sound) {
        device_call<Dword>(direct_sound, 8);
        direct_sound = nullptr;
    }
    shutdown_mixer();
    return 0;
}
} // namespace nfs::sound::backend
