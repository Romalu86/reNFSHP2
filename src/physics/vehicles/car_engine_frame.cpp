#include "car_physics_frame.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
#include "../../math/precision/frame_x87_bridge.hpp"
namespace nfs::game::runtime::physics::car_frame {
namespace {
long double N(float value) { return static_cast<long double>(value); }
long double I(std::int64_t value) { return static_cast<long double>(value); }
long double A(long double value) { return value < 0.0L ? -value : value; }
}
namespace {
int rpm_from_speed(float speed) {
    return physics::legacy::speed_to_rpm(speed);
}
bool speed_limited(void *car) {
    const float limit = at<float>(car, 572), speed = at<float>(car, 3168);
    return limit != 0 && (limit <= speed);
}
} // namespace

void integrate_engine_speed(void *car) {
    const float load = at<float>(car, 24);
    const float unloaded = float(N(1) - load);
    const float throttle = at<float>(car, 1612);
    const auto inverse_inertia = N(1) / N(at<float>(car, 16));
    const float coupling = float((N(at<float>(car, 4)) * inverse_inertia -
                                  N(at<float>(car, 84)) * inverse_inertia) *
                                 at<float>(car, 72) * 30.f);
    const int idle = at<int>(car, 656);
    if (!(at<Byte>(car, 1588) & 1) && rpm_from_speed(at<float>(car, 4)) < idle) {
        at<float>(car, 4) = float(I(idle) * 0.10471975803375244140625f);
        physics::legacy::engine_torque_f32(bytes(car) + 656, idle, vector_at(car, 32));
        at<float>(car, 68) = float(N(at<float>(car, 68)) * .5f);
    } else {
        float torque;
        physics::legacy::engine_torque_f32(bytes(car) + 656, rpm_from_speed(at<float>(car, 4)), &torque);
        at<float>(car, 12) = torque;
        if (!(throttle > 0))
            at<float>(car, 32) = float(-A(N(at<float>(car, 24)) * torque * .4f));
        else {
            const auto remaining = N(1) - at<float>(car, 1612);
            at<float>(car, 32) = float(N(torque) - remaining * remaining * torque);
        }
    }
    auto torque = N(at<float>(car, 32));
    if (at<Dword>(car, 96) != 1 && at<float>(car, 68) != 0)
        torque = torque * unloaded - N(coupling) * load * load;
    if (speed_limited(car) && N(0) < torque)
        torque = N(0);
    auto acceleration = torque * at<float>(car, 16);
    if (at<Byte>(car, 1584) != 1) {
        const float minimum = float(N(at<float>(car, 24)) * -1600.f);
        if (!(N(minimum) < acceleration))
            acceleration = N(minimum);
    }
    auto speed = acceleration * 0.0333333350718021392822265625f + at<float>(car, 4);
    const float maximum = float(I(at<int>(car, 28)) * 0.10471975803375244140625f);
    const float minimum = float(I(at<int>(car, 656)) * 0.10471975803375244140625f);
    if (speed < N(minimum))
        speed = N(minimum);
    else if (N(maximum) < speed)
        speed = N(maximum);
    at<float>(car, 4) = float(speed);
    if (speed_limited(car)) {
        const float road_rpm =
            float(N(at<float>(car, 184 + at<Byte>(car, 1584) * 4)) * at<float>(car, 3168));
        const int road = physics::legacy::round_road_rpm(road_rpm);
        const int current = rpm_from_speed(at<float>(car, 4));
        at<float>(car, 4) = float(I(current < road ? current : road) * 0.10471975803375244140625f);
    }
    at<float>(car, 24) =
        float(I(rpm_from_speed(at<float>(car, 4))) / I(at<int>(car, 28)));
}

void distribute_drive_torque(void *car) {
    if (!(at<Byte>(car, 1588) & 1) && at<float>(car, 32) <= 0 && at<float>(car, 68) < 1)
        at<float>(car, 80) = at<float>(car, 92) < 1 ? 10.f : 0.f;
    else
        at<float>(car, 80) = float(N(at<float>(car, 72)) * at<float>(car, 32));
    if (speed_limited(car))
        at<Dword>(car, 80) = 0;
    const Dword gear = at<Dword>(car, 96);
    if (gear == 1)
        at<Dword>(car, 88) = 0;
    else if (!at<Dword>(car, 100))
        at<float>(car, 88) =
            float(N(at<float>(car, 820 + gear * 4)) * at<float>(car, 788 + gear * 4) *
                  at<float>(car, 892) * at<float>(car, 80));
    else {
        at<Dword>(car, 88) = 0;
        --at<Dword>(car, 100);
    }
    for (unsigned index = 0; index < 4; ++index) {
        void *wheel = wheel_at(car, index);
        at<float>(wheel, 208) = float(N(at<float>(car, index < 2 ? 888 : 884)) *
                                       at<float>(car, 88) * at<float>(wheel, 204));
    }
    if (at<Byte>(car, 776) && at<Byte>(car, 1586)) {
        if (at<float>(car, 84) < 0 && at<float>(car, 68) == 0)
            at<Dword>(car, 8) |= 32;
        else
            at<Dword>(car, 8) &= ~32u;
    }
}

void update_car_transmission(void *car) {
    const Dword gear = at<Dword>(car, 96);
    const float ratio = at<float>(car, 788 + gear * 4);
    float wheel_speed = 0;
    for (unsigned index = 0; index < 4; ++index) {
        void *wheel = wheel_at(car, index);
        if (at<float>(wheel, 204) > 0 && at<Byte>(wheel, 256)) {
            const auto speed = A(N(at<float>(car, index < 2 ? 888 : 884)) *
                                         at<float>(wheel, 156));
            if (!(speed < N(wheel_speed)))
                wheel_speed = float(speed);
            at<float>(car, 92) = wheel_speed;
        }
    }
    at<float>(car, 84) = gear == 1 ? 0.f : float(N(at<float>(car, 92)) * ratio);
    auto clutch = N(1);
    if (!race_started)
        clutch = N(0);
    else if (!at<Byte>(car, 776) && at<Dword>(car, 100)) {
        const float phase =
            float((I(at<int>(car, 100)) * .25f) / I(at<int>(car, 784)));
        clutch = N(1) -
                 turn_sine(phase);
        if (clutch < N(0))
            clutch = N(0);
        else if (N(1) < clutch)
            clutch = N(1);
    } else if (at<std::int16_t>(at<void *>(car, 1768), 596)) {
        clutch = N(at<float>(car, 68)) - .1f;
        if (clutch < N(0))
            clutch = N(0);
        else if (N(1) < clutch)
            clutch = N(1);
    }
    at<float>(car, 68) = float(clutch);
    at<float>(car, 72) = float(clutch);
    integrate_engine_speed(car);
    distribute_drive_torque(car);
    apply_drive_torque(car);
}
} // namespace nfs::game::runtime::physics::car_frame
