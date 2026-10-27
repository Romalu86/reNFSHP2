#include "nfs/physics/frame/dynamics_runtime.hpp"
namespace nfs::game::runtime::race_audio::pending {
// The callers do not consume the incidental EAX value.
void play_event(Dword channel, Dword event, Dword pitch, Dword requested_volume, Dword pan,
                         Dword force_volume) {
    if (!network::graphics_enabled)
        return;
    sound::lock();
    int pitch_value = static_cast<int>(pitch);
    if (pitch_value < 0)
        pitch_value = 0;
    else if (pitch_value > 127)
        pitch_value = 127;
    const Dword slot = (((channel >> 8) & 255) * 124 + (channel & 255)) * 5;
    Dword *state = race_audio_event_channels + slot;
    const int bank = at<int>(bank_bindings, Dword(at<Byte>(state, 8)) * 16);
    int volume = group_volume(state[3], state[4], Byte(force_volume) ? 127 : requested_volume);
    if (race_audio_muted)
        volume = 0;
    auto create = [&](Dword selected, Byte gain, Byte voice_pitch) -> Dword {
        if (!network::graphics_enabled)
            return Dword(-1);
        sound::VoiceParameters parameters;
        sound::initialize_parameters(&parameters);
        if (bank == -1)
            return 0;
        at<Byte>(&parameters, 0) = gain;
        at<std::uint16_t>(&parameters, 8) = std::uint16_t(pan);
        at<Byte>(&parameters, 1) = voice_pitch;
        sound::lock();
        Dword handle = sound::play_bank(bank, static_cast<int>(selected), &parameters);
        sound::set_voice_effect(handle, 0, 127);
        sound::unlock();
        return handle;
    };
    const Dword id = channel & 255;
    if (id >= 112) {
        state[0] =
            create(event, Byte(volume), (id == 118 || id == 119) ? Byte(pitch_value) : 64);
        state[1] = event;
        sound::unlock();
        return;
    }
    if (state[1] != event) {
        if (state[0] != Dword(-1))
            sound::stop(state[0]);
        if (id == 110) {
            Dword handle = create(event, 0, Byte(pitch_value));
            sound::fade_voice_volume(handle, 300, volume);
            state[0] = handle;
        } else
            state[0] = create(event, (id == 3 || id == 4) ? 0 : Byte(volume), Byte(pitch_value));
        state[1] = event;
        sound::unlock();
        return;
    }
    if (sound::voice_missing(state[0])) {
        const int variations[16] = {0, 1, 2, 3, 4, 1, 3, 0, 2, 4, 3, 2, 1, 0, 2, 4};
        Dword selected = event;
        if ((id == 3 || id == 4 || id == 5 || id == 6) && state[1] == 22 && event == 22) {
            selected = Dword(variations[variation_index]) + 22;
            int next =
                static_cast<int>(variation_index + 1) & static_cast<int>(2147483663u);
            if (next < 0)
                next = static_cast<int>((Dword(next) - 1) | 4294967280u) + 1;
            variation_index = Dword(next);
        }
        state[0] = create(selected, Byte(volume), Byte(pitch_value));
        state[1] = event;
    }
    sound::set_voice_volume(state[0], volume);
    sound::set_voice_pitch(state[0], pitch_value);
    sound::set_voice_pan(state[0], at<Dword>(system(72), 2360) == 2 ? 0 : std::int16_t(pan), 0);
    sound::unlock();
}
} // namespace nfs::game::runtime::race_audio::pending
