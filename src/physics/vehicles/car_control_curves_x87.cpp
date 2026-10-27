#include "car_physics_frame.hpp"
#include "car_curve_x87.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
#include "../../math/precision/frame_x87_bridge.hpp"
#include <cstring>
namespace nfs::game::runtime::physics::car_frame {
namespace {
long double E(float value) { return static_cast<long double>(value); }
long double N(float value) { return static_cast<long double>(value); }
long double I(std::int64_t value) { return static_cast<long double>(value); }
long double A(long double value) { return value < 0.0L ? -value : value; }
}




static long double calculate_car_throttle_curve(void *car) {
    auto pedal = I(at<Byte>(car, 1580));
    if (pedal < N(0))
        pedal = N(0);
    else if (N(255) < pedal)
        pedal = N(255);
    const auto normalized = pedal * 0.003921569f;
    auto result = ((N(1) + normalized) * .5f) * normalized;
    if (result < N(0))
        return N(0);
    return N(1) < result ? N(1) : result;
}


static long double calculate_car_steering_input(void *car) {
    auto wheel_angle = N(at<float>(car, at<float>(car, 3200) > 0 ? 2856 : 2560)) /
                       N(at<float>(car, 124));
    if (wheel_angle < N(-1))
        wheel_angle = N(-1);
    else if (N(1) < wheel_angle)
        wheel_angle = N(1);
    const float input = float(A(N(at<float>(car, 1604))));
    auto speed_factor = (A(N(at<float>(car, 3168))) - at<float>(car, 600)) /
                        N(at<float>(car, 604));
    if (speed_factor < N(0))
        speed_factor = N(0);
    else if (N(1) < speed_factor)
        speed_factor = N(1);
    const auto remaining = N(1) - speed_factor;
    float requested = float(((N(1) - remaining) * input + remaining) * input);
    if (!(requested < 1))
        requested = 1;
    if (at<float>(car, 1604) < 0)
        requested = -requested;
    const auto limit = N(at<float>(car, 608)) * speed_factor + remaining;
    auto lower = -limit;
    if (!(lower < wheel_angle))
        lower = wheel_angle;
    const float stored_lower = float(lower);
    auto upper = wheel_angle;
    if (wheel_angle < limit)
        upper = N(float(limit));
    if (requested < stored_lower)
        return N(stored_lower);
    if (!(upper < N(requested)))
        upper = N(requested);
    return upper;
}


static long double calculate_turn_tangent(float turns) {
    return runtime::turn_tangent_retained(turns);
}
namespace curves_x87 {



void car_throttle_curve_native80(void *car, void *output) {
    const long double result = calculate_car_throttle_curve(car);
    std::memcpy(output, &result, 10);
}

void car_steering_input_native80(void *car, void *output) {
    const long double result = calculate_car_steering_input(car);
    std::memcpy(output, &result, 10);
}

void turn_tangent_native80(float turns, void *output) {
    const long double result = calculate_turn_tangent(turns);
    std::memcpy(output, &result, 10);
}
}
} // namespace
