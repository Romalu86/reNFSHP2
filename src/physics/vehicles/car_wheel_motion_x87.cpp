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


static long double rolling_resistance(void *car) {
    const float speed = float(A(N(at<float>(car, 3132))));
    if (speed < 2)
        return N(0);
    const float bank = float(A(N(at<float>(car, 456))));
    const float front_rolling = at<float>(car, 932);
    const float rear_rolling = at<float>(car, 1208);
    auto resistance = N(0);
    for (unsigned index = 0; index < 4; ++index) {
        void *wheel = wheel_at(car, index);
        const auto unscaled = speed < 1 ? N(0) : N(speed) * speed * 0.000002739999899858958087861537933349609375f + .011f;
        float rolling = float(unscaled * (index < 2 ? front_rolling : rear_rolling));
        switch (at<Dword>(wheel, 132)) {
        case 2:
            rolling = float(N(rolling) * 1.5f);
            break;
        case 3:
        case 4:
            rolling = float(N(rolling) + rolling);
            break;
        case 5:
        case 9:
            rolling = float(N(rolling) * 2.5f);
            break;
        case 10:
            rolling = float(N(rolling) * 3.f);
            break;
        }
        const auto degrees = A(N(at<float>(wheel, 20)) * 180.f);
        auto steering = N(float((degrees * .0988f + .0831f) * degrees * .5f + 1.f));
        if (steering < N(1))
            steering = N(1);
        else if (N(8) < steering)
            steering = N(8);
        const float load = at<float>(wheel, 232);
        const float factor = float(wheel_scale(wheel));
        resistance = -((turn_cosine(bank) * factor) * (steering * rolling) * load);
        if (at<float>(wheel, 156) < 0)
            resistance = -resistance;
        if (at<Byte>(wheel, 256) && at<float>(wheel, 156) != 0 && at<float>(wheel, 232) != 0)
            at<float>(wheel, 236) = float(N(at<float>(wheel, 236)) + resistance);
    }
    return resistance;
}

namespace x87 {
void rolling_resistance_native80(void *car, void *output) {
    const long double value = rolling_resistance(car);
    std::memcpy(output, &value, 10);
}
}


void integrate_wheel_angular_speed(void *wheel) {
    if (at<float>(wheel, 232) != 0 && at<Byte>(wheel, 256)) {
        const float angular_error =
            float(-((N(at<float>(wheel, 156)) -
                     N(at<float>(wheel, 44)) / N(at<float>(wheel, 216))) *
                        30.f +
                    at<float>(wheel, 168)));
        void *tire = at<void *>(wheel, 4);
        const float sliding = float(N(at<float>(tire, 268)) * at<float>(wheel, 140));
        const float static_grip = float(N(at<float>(tire, 268)) * at<float>(wheel, 136));
        const float scale = float(wheel_scale(wheel));
        const float squared = float(N(at<float>(wheel, 236)) * at<float>(wheel, 236) +
                                    N(at<float>(wheel, 228)) * at<float>(wheel, 228));
        const float force = float(__builtin_sqrtl(N(squared)));
        float fraction = 1;
        if (N(static_grip) * at<float>(wheel, 232) < N(force))
            fraction =
                float(N(sliding) * at<float>(wheel, 232) * (N(scale) / N(force)));
        void *car = at<void *>(wheel, 0);
        auto retained_fraction = N(fraction);
        if (contacts_pending::wheel_is_punctured(bytes(car) + 7952, Byte(at<Dword>(wheel, 12))))
            retained_fraction = retained_fraction * .3f;
        at<float>(wheel, 168) = float(retained_fraction * angular_error + at<float>(wheel, 168));
    }
    const float angular_speed =
        float(N(at<float>(wheel, 168)) * 0.0333333350718021392822265625f + at<float>(wheel, 156));
    at<float>(wheel, 156) = angular_speed;
    if (!at<Byte>(wheel, 221))
        return;
    void *car = at<void *>(wheel, 0);
    if (at<Byte>(car, 776))
        return;
    const Byte gear = at<Byte>(car, 1584);
    const float axle = at<float>(car, at<int>(wheel, 12) < 2 ? 888 : 884);
    float limit;
    if (gear > 1 && int(gear) < at<int>(car, 780)) {
        limit = float((I(at<int>(car, 660)) * 0.10471975803375244140625f) /
                      (N(axle) * at<float>(car, 788 + gear * 4)));
        if (!(limit < angular_speed) || !(limit > 0))
            return;
    } else if (!gear) {
        limit = float(I(at<int>(car, 660)) * 0.10471975803375244140625f *
                      (N(-1) / (N(axle) * at<float>(car, 788))));
        if (!(angular_speed < limit) || !(limit < 0))
            return;
    } else
        return;
    auto fraction = N(limit) / N(angular_speed);
    if (!(fraction < N(1)))
        fraction = N(1);
    apply_wheel_brake_torque(wheel, float(N(1) - fraction * .5f), 0);
    at<float>(wheel, 156) = limit;
}


void accumulate_car_torques(void *car) {
    const float front_track = float(N(at<float>(car, 532)) * .5f);
    const float rear_track = float(N(at<float>(car, 536)) * .5f);
    const float height = float(N(at<float>(car, 544)) * .9090909f);
    const float front_factor = float(N(1) - at<float>(car, 632));
    const float rear_factor = float(N(1) - at<float>(car, 636));
    float sum[3] = {};
    auto final_x = N(0), final_y = N(0);
    for (unsigned index = 0; index < 4; ++index) {
        const unsigned offset = index == 0   ? 2176
                                : index == 1 ? 2472
                                : index == 2 ? 3064
                                             : 2768;
        const float track = index < 2 ? front_track : rear_track;
        float force[3] = {at<float>(car, offset), 0, at<float>(car, offset + 8)};
        float lever[3] = {(index & 1) ? track : -track, height,
                          index < 2 ? at<float>(car, 112) : -at<float>(car, 116)};
        float torque[3] = {};
        cross_vectors(force, lever, torque);
        if (index == 3) {
            final_x = N(sum[0]) + torque[0];
            final_y = N(sum[1]) - torque[1];
        } else {
            sum[0] = float(N(sum[0]) + torque[0]);
            sum[1] = float(N(sum[1]) - torque[1]);
        }
        sum[2] = float(N(index < 2 ? front_factor : rear_factor) * torque[2] + sum[2]);
    }
    at<float>(car, 3184) =
        float(N(at<float>(car, 452)) * at<float>(car, 140) * final_x * 0.15915493667125701904296875f);
    const auto yaw = final_y * at<float>(car, 144) * 0.15915493667125701904296875f;
    at<float>(car, 3188) = float(yaw);
    at<float>(car, 3192) = float(N(at<float>(car, 148)) * sum[2] * 0.15915493667125701904296875f);
    auto resistance =
        A(N(at<float>(car, 2784))) + A(N(at<float>(car, 3080))) +
        A(N(at<float>(car, 2488))) + A(N(at<float>(car, 2192)));
    resistance = resistance * at<float>(car, 144) * 0.15915493667125701904296875f;
    at<float>(car, 3188) =
        float(at<float>(car, 3200) > 0 ? yaw - resistance : resistance + yaw);
}
} // namespace nfs::game::runtime::physics::car_frame
