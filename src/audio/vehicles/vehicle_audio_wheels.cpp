#include "nfs/audio/vehicles/vehicle_audio.hpp"
#include "../../math/precision/frame_x87_bridge.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
#include "nfs/graphics/resources/eagl_graphics.hpp"
#include "nfs/network/session/network_runtime.hpp"
#include "nfs/core/frame/game_step.hpp"
namespace nfs::game::runtime::race_audio::pending {
namespace v = game::audio::vehicle;

void update_wheel(void *record) {
    auto *wheel = static_cast<Dword *>(record);
    void *car = reinterpret_cast<void *>(wheel[0]);
    if (!car)
        return;
    auto *physics = at<Byte *>(car, 1768);
    auto *position = reinterpret_cast<float *>(physics + 680);
    int pitch = doppler(position, reinterpret_cast<float *>(physics + 8),
                        reinterpret_cast<const float *>(listener_configuration));
    if (pitch < 0)
        return;
    Dword pan = spatial_pan(listener_state, position);
    if (race_audio_listener_count == 2)
        pan = listener_index ? 16383 : 49152;
    Dword listener_bits = listener_index ? 256 : 0;
    long double retained_speed = static_cast<long double>(at<float>(physics, 16));
    if (reinterpret_cast<const float *>(listener_configuration)) {
        void *local = at<void *>(listener_state, 2792 + listener_index * 2760);
        retained_speed = retained_speed - at<float>(at<void *>(local, 1768), 16);
    }
    if (retained_speed < 0.f)
        retained_speed = -retained_speed;
    const volatile float relative_speed = float(retained_speed);
    float relative[3];
    for (int axis = 0; axis < 3; ++axis)
        at<volatile float>(relative, axis * 4) = float(static_cast<long double>(position[axis]) -
                               at<float>(listener_state, axis * 4));
    const volatile float direction_sample =
        float(hardware::dot_product(relative, reinterpret_cast<float *>(physics + 716)) * 65536.f);
    int direction = hardware::current_integer_f32(direction_sample);
    int rpm = v::engine_rpm(car), scale;
    const bool active_session = network::session_active() != 0;
    const bool waiting_for_session = active_session &&
        at<std::int16_t>(system(104), 8) != 5 && at<std::int16_t>(system(104), 8) != 6;
    if (waiting_for_session ||
        int(game::start_deadline - graphics::unresolved::camera_state) > 0) {
        scale = 5;
        wheel[5] = 5;
    } else {
        const int offsets[4] = {-5, 0, 5, 10};
        int target = v::clamp_value(
            int(Dword(v::wrapped_product(rpm, 127) / at<int>(car, 660)) +
                Dword(offsets[wheel[3]])), 0, 127);
        if (!wheel[5])
            wheel[5] = target;
        int smoothed = int(wheel[5]);
        if (target < smoothed)
            smoothed = int(Dword(smoothed) - 2);
        else if (smoothed < target)
            smoothed = int(Dword(smoothed) + 2);
        scale = v::clamp_value(smoothed, 0, 127);
        wheel[5] = scale;
    }
    int squared = wheel[2];
    auto mark = [&](Dword channel) {
        event_flags[channel + listener_index * 124] = 1;
    };
    if (wheel[6] == 4) {
        Dword channel = wheel[3] + 14;
        const volatile float speed_sample =
            float(static_cast<long double>(relative_speed) * 2.2369899749755859375f);
        int speed = hardware::current_integer_f32(speed_sample);
        v::submit_wheel_sound(channel | listener_bits, 1, 64, pitch, squared, pan, speed);
        mark(channel);
    } else {
        Dword index = at<Dword>(car, 1772);
        if (int(game::start_deadline - graphics::unresolved::camera_state) <= 0) {
            v::submit_tyre_sound((index + 22) | listener_bits, 1, scale, pitch, squared, pan,
                                 direction);
            mark(index + 22);
            v::submit_tyre_sound((index + 38) | listener_bits, 0, scale, pitch, squared, pan,
                                 direction);
            mark(index + 38);
        } else {
            v::submit_tyre_sound((index + 38) | listener_bits, 3, scale, pitch, squared, pan,
                                 direction);
            mark(index + 38);
            if (emitter_random_word() % 1000 > 980)
                play_event((at<Dword>(car, 1772) + 22) | listener_bits, 2, 64, 127, pan,
                                    1);
        }
    }
    for (int i = int(at<Dword>(car, 3208) - 1); i >= 0; --i) {
        Dword *event = reinterpret_cast<Dword *>(bytes(car) + 3212 + i * 20);
        if (int(event[0]) < 112 && event[0] != 7) {
            Dword channel = wheel[3] + 54;
            mark(channel);
            v::submit_surface_sound(channel | listener_bits, event[1], event[2], event[3], squared,
                                    pan);
        }
    }
    int listener = listener_index, slot = wheel[3];
    if (wheel[6] != 3) {
        if (engine_active(slot, listener))
            stop_engine(slot, listener_index);
        if (!at<Byte>(car, 1577)) {
            if (wheel[4])
                wheel[4] = 0;
            return;
        }
        Dword channel = at<Dword>(reinterpret_cast<void *>(wheel[0]), 1772) + 62;
        int horn_pitch = v::clamp_value(v::fixed_pitch(pitch, 64), 0, 127);
        play_event(
            channel | (listener_index ? 256 : 0), 0, horn_pitch,
            v::wrapped_product(int(Dword(v::cutoff()) - Dword(squared)), 127) / v::cutoff(), pan, 0);
        wheel[4] = 1;
        mark(channel);
        return;
    }
    if (!at<Byte>(car, 1579)) {
        stop_engine(slot, listener);
        return;
    }
    int volume = v::wrapped_product(int(Dword(v::cutoff()) - Dword(squared)), 127) / v::cutoff();
    if (volume < 0)
        volume = 0;
    if (!engine_active(slot, listener)) {
        start_engine(slot, 0, listener_index);
        return;
    }
    advance_engine(slot, volume, pitch, pan, listener);
}
} // namespace nfs::game::runtime::race_audio::pending
