#include "../race/race_audio_distance.hpp"
#include "../../math/precision/frame_x87_bridge.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
#include "nfs/audio/vehicles/vehicle_audio.hpp"
#include "nfs/core/frame/game_step.hpp"
namespace audio_layout = nfs::game::audio::state::layout;
namespace nfs::game::audio::vehicle {
using namespace state;
namespace ra = runtime::race_audio;

void update_wind(void *car, int listener) {
    Dword channel = (listener ? 0 : 256) | 8;
    const Dword offset = Dword(listener) * 28;
    int &level = track<int>(11828 + offset), &target = track<int>(11832 + offset),
        &pan = track<int>(11836 + offset), &pan_target = track<int>(11840 + offset);
    for (int i = 0; i < track<int>(audio_layout::sound_point_count); ++i) {
        Byte *point = track_data + 1576 - audio_layout::wind_sections + Dword(i) * 64;
        if (at<int>(point, 12) != -4)
            continue;
        float position[3];
        runtime::invoke<void>(car, 0, position);
        const int distance = ra::pending::rounded_point_distance(
            position, reinterpret_cast<const float *>(point));
        if (distance >= at<int>(point - 16, 0))
            continue;
        track<Dword>(11844 + offset) = at<Dword>(point - 32, 0);
        if (level == target) {
            target = runtime::emitter_random_word() & 127;
            return;
        }
        if (pan == pan_target)
            pan_target = runtime::emitter_random_word() & 511;
        else {
            pan = int(Dword(pan) + (pan < pan_target ? 1u : Dword(-1)));
            pan = clamp_value(pan, 0, 511);
        }
        Dword output_pan = Dword(pan) << 7;
        if (at<int>(runtime::system(72), 176) == 2)
            output_pan = listener ? 16383 : 49152;
        level = int(Dword(level) + (level < target ? 1u : Dword(-1)));
        int volume = clamp_value(level, at<int>(point - 8, 0), at<int>(point - 12, 0));
        ra::pending::play_event(channel, track<Dword>(11844 + offset), level, volume,
                                         output_pan, 0);
        return;
    }
    if (!level) {
        ra::stop_event(channel);
        return;
    }
    level = int(Dword(level) - 1u);
    ra::pending::play_event(channel, track<Dword>(11844 + offset), level, level,
                                     Dword(pan) << 7, 0);
}

void update_track_emitters(void *car) {
    void *camera = runtime::invoke<void *>(game::tick_services[0], 2);
    bool car_camera = at<int>(camera, 72) != 6;
    for (int i = 0; i < track<int>(audio_layout::sound_point_count); ++i) {
        Dword *point = reinterpret_cast<Dword *>(track_data + 1548 - audio_layout::wind_sections + i * 64);
        void *object = at<void *>(car, 7988);
        Byte *base = at<Byte *>(object, 4);
        int offset = at<int>(base, 8);
        void *progress = runtime::invoke<void *>(bytes(object) + 4 + offset, 2);
        Dword condition = 0;
        bool eligible = true;
        if (progress) {
            int lap = at<int>(progress, 568);
            if (lap < 0)
                lap = 0;
            condition = point[10];
            if (condition && condition != Dword(-5) && condition != Dword(-6)) {
                if (condition == Dword(-1))
                    eligible = ((Dword(lap) + 1) & 1) != 0;
                else if (condition == Dword(-2))
                    eligible = ((Dword(lap) + 1) & 1) == 0;
                else
                    eligible = (condition & (Dword(1) << (Byte(lap) & 31))) != 0;
            }
        }
        if (point[10] == Dword(-4))
            eligible = false;
        bool triggered = false, emit = false;
        Dword event = point[-1], flags;
        if (!point[1]) {
            emit = eligible;
            flags = point[10] == Dword(-3) ? 5377 : 4353;
        } else {
            flags = point[10] == Dword(-3) ? 1537 : 513;
            if (car_camera)
                flags |= 4096;
            if (eligible && point[2] < runtime::emitter_random_word() % 1000) {
                triggered = true;
                if (condition != Dword(-5) || !point[13]) {
                    emit = true;
                    if (int(point[0]) >= 2) {
                        do {
                            event = runtime::emitter_random_word() % point[0] + point[-1];
                        } while (event == point[14]);
                    }
                }
            }
        }
        if (emit) {
            Dword parameters[12];
            for (int axis = 0; axis < 3; ++axis) {
                parameters[axis] = point[7 + axis];
                parameters[3 + axis] = 0;
            }
            parameters[6] = point[6];
            parameters[7] = flags;
            parameters[8] = event;
            parameters[9] = point[3];
            parameters[10] = point[4];
            parameters[11] = point[5];
            if (condition == Dword(-6)) {
                const long double road_speed =
                    static_cast<long double>(at<float>(car, 3168)) * 2.2369899749755859375f;
                int speed = int(Dword(runtime::hardware::truncate_integer_native80(&road_speed)));
                if (speed < 40)
                    parameters[10] = parameters[11] = 0;
                else {
                    if (speed > 100)
                        speed = 100;
                    const long double gain = static_cast<long double>(speed) * 0.00999999977648258209228515625f *
                                             static_cast<long double>(int(point[4]));
                    parameters[10] = Dword(runtime::hardware::truncate_integer_native80(&gain));
                }
            }
            ra::pending::update_stream_emitter(parameters);
        }
        if (condition == Dword(-5)) {
            const float *position =
                car_camera ? reinterpret_cast<float *>(bytes(at<void *>(car, 1768)) + 680)
                           : reinterpret_cast<float *>(bytes(camera) + 4);
            const int distance = ra::pending::rounded_point_distance(
                position, reinterpret_cast<const float *>(point + 7));
            if (wrapped_product(distance, distance) <= wrapped_product(point[3], point[3])) {
                if (triggered)
                    point[13] = 1;
            } else
                point[13] = 0;
        }
    }
}
} // namespace nfs::game::audio::vehicle
