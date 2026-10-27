#include "../../vehicles/setup/car_initialization_runtime.hpp"
#include "car_curve_x87.hpp"
#include "../../math/precision/frame_x87_bridge.hpp"
namespace nfs::game::world::vehicle_setup {
namespace {
int slip_interval(float degrees, kernel::numeric::Format format) {
    const auto value = Extended(degrees, format);
    auto rounded = Extended::integer(value.round_to_integer(), format);
    if (value < rounded)
        rounded = rounded - 1.f;
    const float stored = float(rounded);
    return game::runtime::hardware::current_integer_f32(stored);
}
} // namespace
namespace legacy {
int slip_interval_f32(float value) {
    return slip_interval(value, kernel::numeric::current_format());
}
}

Extended engine_torque(void *engine, int rpm) {
    const auto format = kernel::numeric::current_format();
    unsigned char native_value[10];
    curves_x87::engine_torque_native80(engine, rpm, native_value);
    return Extended::from_native80(native_value, format);
}


Extended brake_torque(float diameter, Dword compound, int attachment, float pressure) {
    const auto format = kernel::numeric::current_format();
    unsigned char native_value[10];
    curves_x87::brake_torque_native80(diameter, compound, attachment, pressure, native_value);
    return Extended::from_native80(native_value, format);
}


Extended tire_lateral_force(void *tire, void *wheel) {
    const auto format = kernel::numeric::current_format();
    unsigned char native_value[10];
    curves_x87::tire_lateral_force_native80(tire, wheel, native_value);
    return Extended::from_native80(native_value, format);
}


Extended tire_aligning_moment(void *tire, void *wheel) {
    const auto format = kernel::numeric::current_format();
    unsigned char native_value[10];
    curves_x87::tire_aligning_moment_native80(tire, wheel, native_value);
    return Extended::from_native80(native_value, format);
}


Extended rolling_resistance(float speed) {
    const auto format = kernel::numeric::current_format();
    unsigned char native_value[10];
    curves_x87::rolling_resistance_native80(speed, native_value);
    return Extended::from_native80(native_value, format);
}
} // namespace
