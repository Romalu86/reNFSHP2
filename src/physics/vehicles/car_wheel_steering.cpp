#include "car_physics_frame.hpp"
namespace nfs::game::runtime::physics::car_frame {
// Receiver ECX, one float stack argument.
void set_wheel_steering(void *wheel, float input) {
    at<Dword>(wheel, 16) = 0;
    if (!at<Byte>(wheel, 220))
        return;
    void *car = at<void *>(wheel, 0);
    float wheelbase = at<float>(car, 116);
    if (wheelbase < at<float>(car, 112))
        wheelbase = at<float>(car, 112);
    const float steering_speed =
        float(magnitude(number(wheelbase) * at<float>(car, 3200) * 6.283185482025146484375f) +
              magnitude(number(at<float>(car, 3132))));
    auto steering = number(at<float>(car, 124)) * input;
    input = float(steering);
    if (!(magnitude(steering) < number(.001f))) {
        const int index = at<int>(wheel, 12);
        if (index < 2) {
            const float length = at<float>(car, 528), width = at<float>(car, 532);
            const auto tangent = turn_tangent(float(magnitude(number(input))));
            const auto inside =
                entities::pending::direction_turns(length, float(number(length) / tangent - width));
            if (input > 0) {
                at<float>(wheel, 16) = index == 1 ? float(inside) : input;
                return;
            }
            if (!(input < 0))
                return;
            at<float>(wheel, 16) = index == 1 ? input : -float(inside);
            return;
        }
        const bool negative = steering < number(0) || std::isnan(double(steering));
        if (!(8.9406f < steering_speed)) {
            steering = steering * -.2f;
            if (negative) {
                if (!(steering < number(0.0555555559694766998291015625f)))
                    steering = number(0.0555555559694766998291015625f);
            } else if (!(number(-0.0555555559694766998291015625f) < steering))
                steering = number(-0.0555555559694766998291015625f);
        } else {
            steering = steering * .5f;
            if (negative) {
                if (!(number(-0.02777777798473834991455078125f) < steering))
                    steering = number(-0.02777777798473834991455078125f);
            } else if (!(steering < number(0.02777777798473834991455078125f)))
                steering = number(0.02777777798473834991455078125f);
        }
    }
    at<float>(wheel, 16) = float(steering);
}

void sample_wheel_slip_angle(void *wheel) {
    void *car = at<void *>(wheel, 0);
    const float yaw = float(number(at<float>(car, 3200)) * 6.283185482025146484375f);
    const float lateral = float(number(at<float>(car, 536)) * .5f);
    float x = at<float>(car, 3160), z = at<float>(car, 3168);
    switch (at<Dword>(wheel, 12)) {
    case 0:
        x = float(number(x) + number(yaw) * at<float>(car, 112));
        z = float(number(z) + number(yaw) * lateral);
        break;
    case 1:
        x = float(number(x) + number(yaw) * at<float>(car, 112));
        z = float(number(z) - number(yaw) * lateral);
        break;
    case 2:
        x = float(number(x) - number(yaw) * at<float>(car, 116));
        z = float(number(z) - number(yaw) * lateral);
        break;
    case 3:
        x = float(number(x) - number(yaw) * at<float>(car, 116));
        z = float(number(z) + number(yaw) * lateral);
        break;
    }
    if (!race_started) {
        at<Dword>(wheel, 20) = 0;
        return;
    }
    const float cosine = float(turn_cosine(-at<float>(wheel, 16)));
    const auto sin_angle = sine(-at<float>(wheel, 16));
    const auto forward = magnitude(number(cosine) * z - sin_angle * x);
    at<float>(wheel, 20) = float(entities::pending::direction_turns(
        float(number(cosine) * x + number(z) * sin_angle), float(forward)));
}
} // namespace nfs::game::runtime::physics::car_frame
