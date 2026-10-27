#include "car_physics_frame.hpp"
#include "car_curve_x87.hpp"
namespace nfs::game::runtime::physics::car_frame {

Extended clamp_control_factor(float value, float minimum, float maximum) {
    if (value < minimum)
        return number(minimum);
    if (maximum < value)
        return number(maximum);
    return number(value);
}


Extended clamp_transmission_factor(float value, float minimum, float maximum) {
    if (value < minimum)
        return number(minimum);
    if (maximum < value)
        return number(maximum);
    return number(value);
}


Extended car_clutch_limit() {
    return number(1);
}


Extended car_throttle_curve(void *car) {
    const auto format = kernel::numeric::current_format();
    unsigned char native_value[10];
    curves_x87::car_throttle_curve_native80(car, native_value);
    return Extended::from_native80(native_value, format);
}


Extended car_steering_input(void *car) {
    const auto format = kernel::numeric::current_format();
    unsigned char native_value[10];
    curves_x87::car_steering_input_native80(car, native_value);
    return Extended::from_native80(native_value, format);
}

// Uses the x87 tangent operation.
Extended turn_tangent(float turns) {
    const auto format = kernel::numeric::current_format();
    unsigned char native_value[10];
    curves_x87::turn_tangent_native80(turns, native_value);
    return Extended::from_native80(native_value, format);
}


void announce_gear_selection(int selection) {
    if (!selection)
        race_audio::pending::play_event(122, 0, 64, 127, 0, 0);
}
} // namespace
