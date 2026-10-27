#include "../../math/precision/frame_x87_bridge.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
#include "nfs/audio/vehicles/vehicle_audio.hpp"
#include "nfs/graphics/resources/eagl_graphics.hpp"
namespace nfs::game::runtime::race_audio::pending {
namespace v = game::audio::vehicle;
namespace state = game::audio::state;

void update_car_sounds() {
    void *car = at<void *>(listener_state, 2792 + listener_index * 2760);
    if (!car)
        return;
    void *physics;
    int pitch;
    Dword pan = 0;
    if (at<Dword>(system(72), 140)) {
        physics = at<void *>(car, 1768);
        auto *position = reinterpret_cast<float *>(bytes(physics) + 680);
        pitch = doppler(position, reinterpret_cast<float *>(bytes(physics) + 8),
                        reinterpret_cast<const float *>(listener_configuration));
        pan = spatial_pan(listener_state, position);
    } else
        pitch = hardware::current_integer_f32(65536.f);
    if (race_audio_listener_count == 2)
        pan = listener_index ? 16383 : 49152;
    Dword listener_bits = listener_index ? 256 : 0;
    physics = at<void *>(car, 1768);
    auto *position = reinterpret_cast<float *>(bytes(physics) + 680);
    float separation[3];
    for (unsigned axis = 0; axis < 3; ++axis)
        at<volatile float>(separation, axis * 4) =
            float(static_cast<long double>(position[axis]) - at<float>(listener_state, axis * 4));
    const volatile float measured_distance = float(hardware::vector_length(separation));
    int distance = hardware::current_integer_f32(measured_distance);
    int direction = 0;
    if (distance < 0 || distance > 10000)
        distance = 10000;
    if (distance) {
        float relative[3];
        for (int axis = 0; axis < 3; ++axis)
            at<volatile float>(relative, axis * 4) = float(static_cast<long double>(position[axis]) -
                                   at<float>(listener_state, axis * 4));
        const volatile float direction_sample =
            float(hardware::dot_product(relative, reinterpret_cast<float *>(bytes(physics) + 716)) *
                  65536.f);
        direction = hardware::current_integer_f32(direction_sample) / distance;
    }
    int squared = v::wrapped_product(distance, distance);
    v::engine_rpm(car);
    v::update_player_engine(car, squared, pitch, pan, direction, listener_index);
    physics = at<void *>(car, 1768);
    volatile float speed = float(static_cast<long double>(at<float>(car, 3168)) * 2.2369899749755859375f);
    Dword engine_pan =
        race_audio_listener_count == 2 ? (listener_index ? 16383 : 49152) : 0;
    const Dword engine_listener_bits = listener_index ? 256 : 0;
    volatile int &level = state::world<volatile int>(6892);
    if (at<Byte>(physics, 584) == 4) {
        level = int(Dword(level) + 4);
        if (level > 127)
            level = 127;
        const long double scaled_gain = static_cast<long double>(speed) * 0.011111111380159854888916015625f * 90.f;
        int gain = v::clamp_value(int(Dword(hardware::truncate_integer_native80(&scaled_gain))), 0, 90);
        play_event(engine_listener_bits, 0, 64, v::wrapped_product(gain, level) >> 7, engine_pan,
                            0);
        event_flags[listener_index * 124] = 1;
    } else if (at<Byte>(physics, 584) == 6 && speed > 45) {
        speed = float(static_cast<long double>(speed) * 0.011111111380159854888916015625f);
        const long double scaled_step = static_cast<long double>(speed) * 16.f;
        int step = v::clamp_value(int(Dword(hardware::truncate_integer_native80(&scaled_step))), 0, 16);
        const long double scaled_gain = static_cast<long double>(speed) * 90.f;
        int gain = v::clamp_value(int(Dword(hardware::truncate_integer_native80(&scaled_gain))), 0, 90);
        if (v::siren_rising) {
            level = int(Dword(level) + Dword(step));
            if (level > 127) {
                v::siren_rising = 0;
                level = 127;
            }
        } else {
            level = int(Dword(level) - Dword(step));
            if (level < 35) {
                v::siren_rising = 1;
                level = 35;
            }
        }
        const int event_pitch = v::clamp_value(level, 44, 84);
        const int event_volume = v::wrapped_product(gain, level) >> 7;
        play_event(engine_listener_bits, 0, event_pitch, event_volume, engine_pan, 0);
        event_flags[listener_index * 124] = 1;
    } else {
        level = 0;
        v::siren_rising = 1;
    }
    const int event_count = at<int>(car, 3208);
    for (int i = 0; i < event_count; ++i) {
        Dword *event = reinterpret_cast<Dword *>(bytes(car) + 3212 + i * 20);
        Dword channel = event[0];
        if (event[2] == 260) {
            int gain = v::wrapped_product(event[3], 127) / 655360;
            if (gain > 126)
                gain = 127;
            gain *= 2;
            int period = 8 - gain / 16;
            if (period < 3)
                period = 3;
            gain = v::attenuate(squared, v::cutoff(), gain);
            if (gain > 100)
                gain = 100;
            int &counter = state::world<int>(6896);
            counter = int(Dword(counter) + 1) % period;
            if (!counter && !at<std::int16_t>(physics, 596))
                play_event(116, 28, 64, gain, 0, 0);
        } else {
            v::select_surface_sound(channel | listener_bits, event[1], event[2], event[3], squared,
                                    pan);
            if (int(channel) < 112)
                event_flags[channel + listener_index * 124] = 1;
        }
    }
    int listener = listener_index;
    Byte siren = at<Byte>(car, 1579);
    if (!siren)
        stop_engine(4, listener);
    else {
        int volume = v::wrapped_product(int(Dword(v::cutoff()) - Dword(squared)), 127) / v::cutoff();
        if (volume < 0)
            volume = 0;
        if (!engine_active(4, listener))
            start_engine(4, 0, listener_index);
        else
            advance_engine(4, volume, pitch, pan, listener);
    }
    Dword &horn_active = at<Dword>(listener_state, 2796 + listener_index * 2760);
    if (at<Byte>(car, 1577) && !siren) {
        v::submit_distant_engine(squared, v::fixed_pitch(pitch, 64), pan, listener_index);
        event_flags[1 + listener_index * 124] = 1;
        horn_active = 1;
        return;
    }
    if (horn_active) {
        v::stop_distant_engine();
        horn_active = 0;
    }
}
} // namespace nfs::game::runtime::race_audio::pending
