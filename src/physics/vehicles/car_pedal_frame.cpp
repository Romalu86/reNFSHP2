#include "car_physics_frame.hpp"
#include "car_drive_frame_x87.hpp"
namespace nfs::game::runtime::physics::car_frame {

void update_car_throttle_and_brakes(void *car) {
    if (at<Byte>(car, 776))
        select_reverse_braking(car);
    if (!at<Byte>(car, 1592) && audio::vehicle::car_audio_suppressed(car)) {
        const Byte previous = at<Byte>(car, 1583);
        const int difference = int(at<Byte>(car, 1576)) - previous;
        if (difference > 31)
            at<Byte>(car, 1583) = Byte(previous + (difference > 16 ? 16 : difference));
        else if (difference > 1)
            at<Byte>(car, 1583) = Byte(previous + (difference / 2 > 8 ? 8 : difference / 2));
        else if (difference == 1)
            at<Byte>(car, 1583) = Byte(previous + 1);
        else if (difference < -31)
            at<Byte>(car, 1583) = Byte(previous - (-difference > 31 ? 32 : -difference));
        else if (difference < -1)
            at<Byte>(car, 1583) = Byte(previous - (-difference / 2 > 15 ? 16 : -difference / 2));
        else if (difference == -1)
            at<Byte>(car, 1583) = Byte(previous - 1);
    } else {
        at<Byte>(car, 1583) = at<Byte>(car, 1576);
    }
    if (!at<Byte>(car, 1593) && audio::vehicle::car_audio_suppressed(car)) {
        const int requested = at<Byte>(car, 1581);
        const int step = at<Byte>(car, 1580) < requested ? at<Byte>(car, 1580) + 10 : requested;
        at<Byte>(car, 1580) = Byte(audio::vehicle::clamp_value(step, 0, requested));
    } else {
        at<Byte>(car, 1580) = at<Byte>(car, 1581);
    }
    const float requested = at<float>(car, 1600), previous = at<float>(car, 1604);
    const bool crossing = (requested >= 0 && previous < 0) || (requested <= 0 && previous > 0);
    const float rate = at<float>(car, crossing ? 596 : 592);
    const float difference = float(drive::retained(requested) - previous);
    float result;
    if (difference < 0) {
        const float amount = -difference;
        result = float(drive::retained(previous) - (rate < amount ? rate : amount));
    } else {
        result = float(drive::retained(previous) + (rate >= difference ? difference : rate));
    }
    at<float>(car, 1604) = result;
    at<float>(car, 1604) = float(drive::transmission_clamp(at<float>(car, 1604), -1, 1));
    if (at<Byte>(car, 776))
        update_automatic_gear_selection(car);
    else
        update_manual_gear_selection(car);
    if (!race_started)
        at<Byte>(car, 1580) = 255;
    float throttle = float(drive::integer(at<Byte>(car, 1583)) * 0.003921569f);
    if (throttle < 0)
        throttle = 0;
    else if (throttle > 1)
        throttle = 1;
    if (throttle < at<float>(car, 1612))
        at<Dword>(car, 8) |= 4;
    else
        at<Dword>(car, 8) &= ~4u;
    at<float>(car, 1612) = throttle;
    at<float>(car, 1608) = float(drive::throttle_curve(car));
}
} // namespace nfs::game::runtime::physics::car_frame
