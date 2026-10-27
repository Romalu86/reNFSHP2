#include "nfs/audio/vehicles/vehicle_audio.hpp"
#include "nfs/core/frame/game_step.hpp"
namespace nfs::game::audio::vehicle {

int engine_rpm(void *car) {
    return static_cast<int>(static_cast<long double>(at<float>(car, 4)) * 9.54929637908935546875f + 0.5f);
}

int clamp_value(int value, int minimum, int maximum) {
    return value < minimum ? minimum : value > maximum ? maximum : value;
}

int selected_camera_mode(void *camera) {
    return at<int>(camera, 312 + at<int>(camera, 356) * 4);
}

int normalized_rpm(void *car) {
    int idle = at<int>(car, 656), maximum = at<int>(car, 660);
    int rpm = clamp_value(
        wrapped_product(int(Dword(engine_rpm(car)) - Dword(idle)), 127) / (maximum - idle), 0, 127);
    if (!table(7218880, 0))
        return 0;
    runtime::race_audio::debug_sound(4, 1, "TrueRPM: %5d", rpm);
    runtime::race_audio::debug_sound(5, 1, "FakeRPM: %5d", rpm);
    return rpm;
}

void set_engine_parameters(int volume, std::uint16_t rpm, int throttle, int distance, int pitch,
                           std::uint16_t pan, int direction, void *camera, int listener) {
    auto *engine = owner(listener);
    if (!engine)
        return;
    at<std::uint16_t>(engine, 22) = rpm;
    at<std::int16_t>(engine, 12) = std::int16_t(pitch >> 4);
    throttle = int(Dword(throttle) + Dword(throttle >> 5));
    if (throttle > 127)
        throttle = 127;
    at<int>(engine, 8) = throttle < 4 ? 0 : throttle;
    if (!at<Dword>(runtime::system(72), 140))
        at<Byte>(engine, 18) = at<Byte>(engine, selected_camera_mode(camera) == 3 ? 36 : 40);
    else if (!distance) {
        at<Byte>(engine, 18) = 44;
        pan = 0;
    } else
        at<Byte>(engine, 18) = Byte((direction >> (direction < 0 ? 11 : 10)) + 44);
    at<std::uint16_t>(engine, 14) = pan;
    at<std::uint16_t>(engine, 16) = 16383;
    if (at<Byte>(engine, 18) > 126)
        at<Byte>(engine, 18) = 127;
    at<int>(engine, 4) = clamp_value(volume, 0, 127);
}
} // namespace nfs::game::audio::vehicle
