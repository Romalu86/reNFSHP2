#include "car_physics_frame.hpp"
#include "car_drive_frame_x87.hpp"
namespace nfs::game::runtime::physics::car_frame {
// Callers consume AL only.
Byte wheel_reverse_brake(void *wheel) {
    if (!at<Byte>(wheel, 221))
        return 0;
    void *car = at<void *>(wheel, 0);
    if (!car)
        return 0;
    const Byte gear = at<Byte>(car, 1584);
    return (!gear && at<float>(wheel, 156) > 0) || (gear > 1 && at<float>(wheel, 156) <= 0);
}
// ECX wheel; float amount, low-byte mode.
void apply_wheel_brake_torque(void *wheel, float amount, int handbrake) {
    if (!at<Byte>(wheel, 256) || at<float>(wheel, 232) <= 0)
        return;
    void *brake = at<void *>(wheel, 8), *car = at<void *>(wheel, 0);
    if (!brake || !car)
        return;
    const auto speed = drive::absolute(drive::retained(at<float>(wheel, 44)));
    const bool match_ground = speed < drive::retained(.25f) || (Byte(handbrake) && at<Dword>(brake, 12));
    const float stop = float(drive::retained(at<float>(wheel, 156)) * 30.f + at<float>(wheel, 168));
    const float inertia = float(drive::retained(at<float>(car, 216 + at<Dword>(car, 96) * 4)) *
                                    at<float>(wheel, 204) * .5f +
                                at<float>(wheel, 180));
    float torque;
    if (drive::retained(1) < speed || __builtin_isnan(speed)) {
        auto pressure = drive::brake_pressure(at<float>(brake, 8), at<Dword>(brake, 4),
                                                           at<int>(brake, 0), amount);
        torque = float(pressure);
        auto previous = drive::absolute(drive::retained(at<float>(wheel, 168)) * inertia);
        const float half = float(pressure * .5f);
        if (!(previous < drive::retained(half)))
            previous = drive::retained(half);
        if (drive::absolute(drive::retained(at<float>(wheel, 156))) < drive::retained(.05f))
            torque = float(pressure - previous);
    } else {
        const float clamped = amount < 1 ? amount : 1.f;
        torque = float(drive::retained(clamped) * at<float>(wheel, 216) * at<float>(wheel, 232));
    }
    if (match_ground) {
        auto maximum = drive::retained(at<float>(wheel, 232)) * at<float>(wheel, 136);
        if (drive::retained(0) < maximum || maximum < drive::retained(0) || __builtin_isnan(maximum)) {
            const auto square =
                maximum * maximum - drive::retained(at<float>(wheel, 228)) * at<float>(wheel, 228);
            const float stored_square = float(square);
            maximum = square < drive::retained(0) ? drive::retained(0)
                                         : drive::retained(float(__builtin_sqrtl(drive::retained(stored_square))));
        } else
            maximum = drive::retained(0);
        maximum = maximum * at<float>(wheel, 216);
        if (!(drive::retained(torque) < maximum))
            torque = float(maximum);
    }
    const auto reciprocal = drive::retained(1) / drive::retained(at<float>(wheel, 216));
    const float stored_reciprocal = float(reciprocal);
    const float force = float(reciprocal * torque);
    auto angular = drive::retained(torque) / drive::retained(inertia);
    if (stop > 0) {
        angular = -angular;
        if (!(drive::retained(-stop) < angular))
            angular = drive::retained(-stop);
    } else if (!(angular < drive::retained(-stop)))
        angular = drive::retained(-stop);
    if (match_ground)
        angular = -(
            (drive::retained(at<float>(wheel, 156)) - drive::retained(at<float>(wheel, 44)) * stored_reciprocal) *
            30.f);
    else
        angular += drive::retained(at<float>(wheel, 168));
    at<float>(wheel, 168) = float(angular);
    auto direction = drive::cosine(at<float>(wheel, 20));
    if (at<int>(wheel, 12) < 2 && !match_ground) {
        const auto angle = drive::absolute(drive::retained(at<float>(wheel, 20)));
        const float clamped = angle < drive::retained(.25f) ? float(angle) : .25f;
        direction = drive::retained(1) - physics::legacy::power_retained(
                                    double(drive::retained(clamped) * 4.f), 0.15000000596046448);
    }
    const auto applied = drive::absolute(drive::retained(force)) * direction;
    at<float>(wheel, 236) =
        float(at<float>(wheel, 44) > 0 ? drive::retained(at<float>(wheel, 236)) - applied
                                         : applied + at<float>(wheel, 236));
}

void apply_car_brakes(void *car) {
    const float service_brake = at<float>(car, 1608);
    const Byte handbrake = at<Byte>(car, 1585);
    auto handbrake_factor = drive::retained(0);
    if (!handbrake)
        at<Dword>(car, 1616) = 0;
    else {
        if (at<int>(car, 1616) < 30)
            ++at<Dword>(car, 1616);
        handbrake_factor = drive::handbrake_blend_retained(at<int>(car, 1616));
    }
    const float rear_limit = at<float>(car, 580);
    for (unsigned index = 0; index < 4; ++index) {
        void *wheel = wheel_at(car, index);
        if (wheel_reverse_brake(wheel))
            apply_wheel_brake_torque(wheel, 1, 0);
        else if (handbrake && at<int>(wheel, 12) > 1)
            apply_wheel_brake_torque(wheel, float(drive::retained(rear_limit) * handbrake_factor), 0);
        else if (service_brake != 0)
            apply_wheel_brake_torque(wheel, service_brake, 1);
    }
}
} // namespace nfs::game::runtime::physics::car_frame
