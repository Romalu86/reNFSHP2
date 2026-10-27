#include "nfs/audio/backend/sound_ownership.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
extern "C" __declspec(dllimport) long __stdcall DirectSoundCreate(const void *, void **, void *);
namespace nfs::sound::backend {
extern const double position_scale;

int create_output_device() {
    int failure = -1;
    for (;;) {
        alignas(4) Byte mixer_description[12], description[36]{}, wave[18];
        field<Dword>(mixer_description, 0) = setting<std::uint16_t>(8708898);
        field<Byte>(mixer_description, 4) = setting<Byte>(8708902);
        field<Byte>(mixer_description, 5) = Byte(output_channels);
        field<Byte>(mixer_description, 6) =
            !setting<Byte>(8708920) && Byte(output_channels) > 1 ? 1 : 0;
        field<void(__cdecl *)(int)>(mixer_description, 8) = retire_voice;
        initialize_mixer(mixer_description);
        Dword rate = setting<std::uint16_t>(8708898);
        platform<Dword>(8711412) = platform<Dword>(8711404) = platform<Dword>(8711408) =
            platform<Dword>(8711424) = platform<Dword>(8711396) = platform<Dword>(8711436) = 0;
        platform<Dword>(8711416) = (rate * 20 / 1000) & 1048560;
        platform<Dword>(8711400) = (rate / 50) & 1048560;
        int result = int(DirectSoundCreate(nullptr, &direct_sound, nullptr));
        if (result) {
            direct_sound = nullptr;
            failure = Dword(result) == 2289565706u ? -18 : Dword(result) == 2289565816u ? -10 : -1;
            release_output_device();
            return failure;
        }
        using SetCooperative = int(__stdcall *)(void *, void *, int);
        auto set_cooperative =
            reinterpret_cast<SetCooperative>(field<Dword *>(direct_sound, 0)[24 / 4]);
        void *window = foreground_window();
        set_cooperative(direct_sound, window, 2);
        field<Dword>(description, 0) = 36;
        field<Dword>(description, 4) =
            lifecycle<Byte>(8708815) || setting<Byte>(8708920) ? 17 : 1;
        if (device_call<int>(direct_sound, 12, description, &platform<void *>(8711372),
                             0)) {
            release_output_device();
            return failure;
        }
        field<std::uint16_t>(wave, 0) = 1;
        field<std::uint16_t>(wave, 2) = 2;
        rate = setting<std::uint16_t>(8708898);
        field<Dword>(wave, 4) = rate;
        field<Dword>(wave, 8) = rate * 4;
        field<std::uint16_t>(wave, 12) = 4;
        field<std::uint16_t>(wave, 14) = 16;
        field<std::uint16_t>(wave, 16) = 0;
        device_call<int>(platform<void *>(8711372), 56, wave);
        if (lifecycle<Byte>(8708815)) {
            probe_effect_interface(setting<Byte>(8708918));
            apply_environment(platform_tail<int>(8717644), platform_tail<int>(8717648),
                              platform_tail<int>(8717652));
        }
        zero_memory(description, 36);
        field<Dword>(description, 0) = 36;
        field<Dword>(description, 4) = 98304;
        rate = setting<std::uint16_t>(8708898);
        field<Dword>(wave, 4) = rate;
        Dword bytes = ((rate * 400 / 1000) & 1048560) * 2, ring = bytes >> 1;
        platform<Dword>(8711428) = bytes;
        platform<Dword>(8711452) = ring;
        bool separate = setting<Byte>(8708920) != 0;
        unsigned channels = Byte(output_channels);
        field<std::uint16_t>(wave, 2) = std::uint16_t(separate ? 1 : channels);
        field<std::uint16_t>(wave, 12) = std::uint16_t((separate ? 1 : channels) * 2);
        field<Dword>(wave, 8) = (separate ? 1 : channels) * rate * 2;
        if (separate)
            field<Dword>(description, 4) = 98324;
        else
            bytes *= channels;
        platform<Dword>(8711428) = bytes;
        platform<Dword>(8711432) = ring;
        platform<Dword>(8711420) = ((ring * 8) / 10) & 1048560;
        platform<Dword>(8711444) = platform<Dword>(8711448) = platform<Dword>(8711440) = 0;
        field<Dword>(description, 8) = bytes;
        field<void *>(description, 16) = wave;
        bool fallback = false;
        for (unsigned c = 0; c < (separate ? Byte(output_channels) : 1); ++c) {
            auto &buffer = platform<void *>(8711376 + c * 4);
            if (device_call<int>(direct_sound, 12, description, &buffer, 0)) {
                if (separate) {
                    fallback = true;
                    break;
                }
                release_output_device();
                return failure;
            }
            if (separate) {
                void *spatial;
                if (device_call<int>(buffer, 0, buffer3d_iid, &spatial)) {
                    release_output_device();
                    return failure;
                }
                float position[3];
                voice_pan_vector(setting<std::uint16_t>(
                                     8709000 + (c + unsigned(Byte(output_channels)) * 4) * 2), 0,
                                 position);
                for (auto &value : position)
                    value = float(static_cast<long double>(value) * position_scale);
                if (device_call<int>(spatial, 76, position[0], 0.f, position[2], 0)) {
                    release_output_device();
                    return failure;
                }
                if (use_effect_interface) {
                    void *effect;
                    if (device_call<int>(buffer, 0, effect_iid, &effect))
                        effect = nullptr;
                    Dword value = 0;
                    device_call<int>(effect, 16, effect_guid, 1, 0, 0, &value, 4);
                }
            }
        }
        if (fallback) {
            release_output_device();
            setting<Byte>(8708920) = 0;
            setting<Byte>(8708861) = 2;
            if (Byte(output_channels) > 2)
                setting<Byte>(8708928) = 2;
            continue;
        }
        for (unsigned c = 0; c < (separate ? Byte(output_channels) : 1); ++c) {
            void *memory;
            Dword locked;
            if (!device_call<int>(platform<void *>(8711376 + c * 4), 44, 0,
                                  platform<Dword>(8711428), &memory,
                                  &locked, 0, 0, 0)) {
                zero_memory(memory, platform<Dword>(8711428));
                device_call<int>(platform<void *>(8711376 + c * 4), 76, memory,
                                 platform<Dword>(8711428), 0, 0);
            }
        }
        for (unsigned c = 0; c < (separate ? Byte(output_channels) : 1); ++c)
            if (device_call<int>(platform<void *>(8711376 + c * 4), 48, 0, 0, 1)) {
                release_output_device();
                return failure;
            }
        if (separate)
            for (unsigned c = 0; c < Byte(output_channels); ++c)
                device_call<int>(platform<void *>(8711376 + c * 4), 52, 0);
        return 0;
    }
}
} // namespace nfs::sound::backend
