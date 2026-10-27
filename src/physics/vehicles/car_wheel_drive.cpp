#include "car_physics_frame.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
#include "../../math/precision/frame_x87_bridge.hpp"
#include "nfs/graphics/resources/eagl_graphics.hpp"
namespace nfs::game::runtime::physics::car_frame {
namespace {
long double N(float value) { return static_cast<long double>(value); }
long double I(std::int64_t value) { return static_cast<long double>(value); }
long double A(long double value) { return value < 0.0L ? -value : value; }
}

void apply_drive_torque(void *car) {
    const Dword gear = at<Dword>(car, 96);
    for (unsigned index = 0; index < 4; ++index) {
        void *wheel = wheel_at(car, index);
        if (!at<Byte>(wheel, 221))
            continue;
        const float ratio = float(N(at<float>(car, index < 2 ? 888 : 884)) *
                                  at<float>(car, 788 + gear * 4));
        if (ratio == 0)
            continue;
        auto drive = N(at<float>(wheel, 208));
        if (at<Byte>(car, 8) & 32) {
            at<Dword>(wheel, 168) = 0;
            drive = N(0);
            at<Dword>(wheel, 156) = 0;
        } else if (!at<Dword>(car, 96)) {
            drive = -drive;
        }
        float acceleration = float(drive / N(at<float>(wheel, 216)));
        if (!gear) {
            const int rpm = physics::legacy::speed_to_rpm(at<float>(car, 4));
            if (N(at<float>(car, 3168)) < I(rpm) * at<float>(car, 276))
                acceleration = 0;
        }
        if (at<Byte>(car, 1588) & 1) {
            at<Dword>(wheel, 168) = 0;
            at<Dword>(wheel, 156) = 0;
            acceleration = 0;
        }
        if (at<Byte>(car, 1584) != 1 && acceleration != 0) {
            auto driven_speed = N(at<float>(car, 4)) / N(ratio);
            if (!gear)
                driven_speed = -driven_speed;
            const auto difference = (driven_speed - at<float>(wheel, 156)) * 30.f;
            at<float>(wheel, 168) = float(difference);
            if (N(acceleration) * difference < N(0))
                acceleration = 0;
        }
        if (A(I(at<int>(car, 660)) * at<float>(car, 276 + gear * 4)) <
            A(N(at<float>(car, 3168))))
            acceleration = 0;
        if (at<Byte>(car, 7952)) {
            const int ticks =
                int(Dword(graphics::unresolved::camera_state) - at<Dword>(car, 7956));
            const auto elapsed = I(ticks) * 0.001666667f;
            const float stored_elapsed = float(elapsed);
            auto limit = N(0.89406f);
            if (!(N(1) < elapsed)) {
                limit = contacts_pending::wheel_is_punctured(bytes(car) + 7952, Byte(index))
                            ? N(26.82180023193359375f) - N(stored_elapsed) * 25.9277400970458984375f
                            : N(111.75749969482421875f) - N(stored_elapsed) * 110.86344146728515625f;
            }
            if (limit < A(N(at<float>(car, 3168))))
                acceleration = 0;
            else
                acceleration = float(
                    (N(1) - A(N(at<float>(car, 3168))) / limit) * acceleration);
        }
        if (at<Byte>(wheel, 256) && at<float>(wheel, 232) != 0)
            at<float>(wheel, 236) = float(N(at<float>(wheel, 236)) + acceleration);
    }
    if (at<float>(car, 560) == 0)
        return;
    // Rear/front diagonal transfer order is encoded explicitly by the body.
    const auto transfer = [&](unsigned rear, unsigned front) {
        const float rear_coefficient =
            at<float>(car, at<float>(car, rear + 32) >= 1 ? rear + 140 : rear + 136);
        const auto rear_capacity = N(rear_coefficient) * at<float>(car, rear + 232);
        if (!(rear_capacity < N(at<float>(car, rear + 236))))
            return;
        const auto rear_excess = N(at<float>(car, rear + 236)) - rear_capacity;
        float excess = float(rear_excess);
        if (!(N(0) < rear_excess))
            excess = 0;
        if (!at<Byte>(car, front + 256) || at<float>(car, front + 232) == 0)
            return;
        const float front_coefficient =
            at<float>(car, at<float>(car, front + 32) >= 1 ? front + 140 : front + 136);
        auto capacity = N(front_coefficient) * at<float>(car, front + 232) -
                        at<float>(car, front + 236);
        if (!(N(0) < capacity))
            capacity = N(0);
        const auto transfer_amount = capacity < N(excess) ? capacity : N(excess);
        at<float>(car, front + 236) =
            float(transfer_amount + at<float>(car, front + 236));
        at<float>(car, rear + 236) =
            float(N(at<float>(car, rear + 236)) - transfer_amount);
    };
    transfer(2540, 2244);
    transfer(2836, 1948);
}
} // namespace nfs::game::runtime::physics::car_frame
