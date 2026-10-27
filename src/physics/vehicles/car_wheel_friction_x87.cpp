#include "car_physics_frame.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
#include "../../math/precision/frame_x87_bridge.hpp"
#include <cstring>
namespace nfs::game::runtime::physics::car_frame {
namespace {
long double N(float value) { return static_cast<long double>(value); }
long double I(std::int64_t value) { return static_cast<long double>(value); }
long double A(long double value) { return value < 0.0L ? -value : value; }
long double wheel_scale(void *wheel) {
    long double value;
    x87::wheel_force_scale_native80(wheel, &value);
    return value;
}
}


static long double force_scale(void *wheel) {
    const float slip = at<float>(wheel, 32);
    if (slip >= 0 && slip <= .15f)
        return N(1);
    if (slip > .15f && slip < 1)
        return (N(1) - slip) * 0.4117647111415863037109375f + .65f;
    if (slip >= 1 && slip <= 8)
        return N(0.707099974155426025390625f) - N(slip) * 0.0571000017225742340087890625f;
    if (slip > 8) {
        const auto result = N(2) / N(slip);
        return N(.0625f) < result ? result : N(.0625f);
    }
    if (slip < -.05f && slip >= -.2f)
        return N(slip) * -4.375f;
    if (slip < -.2f && slip > -1)
        return N(.875f) - A(N(slip) * .375f);
    if (slip > -1 || std::isnan(slip))
        return N(1);
    const auto result = A(N(.5f) / N(slip));
    return N(.125f) < result ? result : N(.125f);
}

namespace x87 {
void wheel_force_scale_native80(void *wheel, void *output) {
    const long double value = force_scale(wheel);
    std::memcpy(output, &value, 10);
}
}


void solve_wheel_lateral_force(void *wheel) {
    void *car = at<void *>(wheel, 0), *tire = at<void *>(wheel, 4);
    at<Dword>(wheel, 228) = 0;
    float wheelbase = at<float>(car, 116);
    if (at<float>(car, 112) > wheelbase)
        wheelbase = at<float>(car, 112);
    const float quarter_mass = float(N(at<float>(at<void *>(car, 1768), 32)) * .25f);
    const float speed =
        float(A(N(at<float>(car, 3132))) +
              A(N(wheelbase) * at<float>(car, 3200) * 6.283185482025146484375f));
    if (!(at<float>(wheel, 232) > 0) || !at<Byte>(wheel, 256)) {
        at<Dword>(wheel, 228) = 0;
        at<Dword>(wheel, 236) = 0;
        at<Dword>(wheel, 244) = 0;
    } else if (speed < .25f) {
        at<float>(wheel, 228) = float(N(quarter_mass) * at<float>(car, 3148));
        at<Dword>(wheel, 244) = 0;
    } else {
        if (speed < 6 && at<float>(wheel, 32) < .15f) {

            const float blend = float((N(speed) - .25f) * 0.173913f);
            auto angle = turn_sine(at<float>(wheel, 20)) * 0.008333334f * (N(1) - blend) +
                         N(at<float>(wheel, 20)) * blend;
            static_cast<volatile float &>(at<float>(wheel, 20)) = float(angle);
            if (angle < N(-1))
                angle = N(-1);
            else if (N(1) < angle)
                angle = N(1);
            at<float>(wheel, 20) = float(angle);
        }
        physics::legacy::tire_lateral_f32(tire, wheel, vector_at(wheel, 228));
        physics::legacy::tire_aligning_f32(tire, wheel, vector_at(wheel, 244));
        if (contacts_pending::wheel_is_punctured(bytes(at<void *>(wheel, 0)) + 7952,
                                                Byte(at<Dword>(wheel, 12))))
            at<float>(wheel, 228) = float(N(at<float>(wheel, 228)) * .5f);
    }
    const auto force = A(N(at<float>(wheel, 228)));
    if ((force < N(10) || float(force) == 10) && speed > 6)
        at<Dword>(wheel, 228) = 0;
}


void solve_low_speed_wheel_forces(void *car) {
    float wheelbase = at<float>(car, 116);
    if (at<float>(car, 112) > wheelbase)
        wheelbase = at<float>(car, 112);
    const auto speed = A(N(at<float>(car, 3132))) +
                       A(N(wheelbase) * at<float>(car, 3200) * 6.283185482025146484375f);
    if (speed <= N(.25f)) {
        at<float>(car, 3200) = float(N(at<float>(car, 3200)) * .95f);
        const float lateral = at<float>(car, 3160);
        at<Dword>(car, 3160) = 0;
        at<float>(car, 3148) = float(N(lateral) * -30.f);
    }
    for (unsigned index = 0; index < 4; ++index) {
        sample_wheel_slip_angle(wheel_at(car, index));
        solve_wheel_lateral_force(wheel_at(car, index));
    }
}


void limit_wheel_friction_forces(void *wheel) {
    void *tire = at<void *>(wheel, 4);
    const float square = float(N(at<float>(wheel, 236)) * at<float>(wheel, 236) +
                               N(at<float>(wheel, 228)) * at<float>(wheel, 228));
    const float force = float(__builtin_sqrtl(N(square)));
    if (!(N(at<float>(wheel, 136)) * at<float>(wheel, 232) < N(force)) || !(force > 0))
        return;
    const auto fraction = (N(at<float>(wheel, 140)) * at<float>(wheel, 232)) / N(force);
    const float stored_fraction = float(fraction);
    const float root_input = float(fraction * .99f);
    const float factor = float(__builtin_sqrtl(N(root_input)));
    const float lateral = float(N(factor) * at<float>(wheel, 228));
    void *car = at<void *>(wheel, 0);
    at<float>(wheel, 228) = lateral;
    const float longitudinal = float(N(factor) * at<float>(wheel, 236));
    at<float>(wheel, 236) = longitudinal;
    const float aligning = float(N(stored_fraction) * at<float>(wheel, 244));
    at<float>(wheel, 244) = aligning;
    if (A(N(at<float>(car, 3132))) < N(18) && at<float>(wheel, 32) != 0 &&
        !car_is_racer(car)) {
        const auto scale = wheel_scale(wheel);
        const float stored_scale = float(scale);
        const auto lateral_scale = physics::legacy::power_retained(
            double(scale), double(at<float>(tire, 264)));
        at<float>(wheel, 228) = float(lateral_scale * lateral);
        const auto longitudinal_scale =
            physics::legacy::power_retained(double(stored_scale), .25);
        at<float>(wheel, 236) = float(longitudinal_scale * longitudinal);
        at<float>(wheel, 244) = float(N(aligning) * stored_scale);
    }
}
} // namespace nfs::game::runtime::physics::car_frame
