#include "../../vehicles/setup/car_initialization_runtime.hpp"
#include "car_curve_x87.hpp"
#include "car_drive_frame_x87.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
#include "../../math/precision/frame_x87_bridge.hpp"
#include <cstring>
namespace nfs::game::world::vehicle_setup {
namespace {
long double retain_float(float value) { return static_cast<long double>(value); }
long double retain_integer(std::int64_t value) { return static_cast<long double>(value); }
}

static long double calculate_engine_torque(void *engine, int rpm) {
    const int interval = rpm / 500;
    const auto first = retain_float(at<float>(engine, 12 + Dword(interval) * 4));
    const auto remainder = retain_integer(rpm) - retain_integer(interval) * 500.f;
    if (interval <= at<int>(engine, 4) / 500)
        return (retain_float(at<float>(engine, 16 + Dword(interval) * 4)) - first) * remainder *
                   0.00200000009499490261077880859375f +
               first;
    const auto result =
        (-first / retain_integer(int(at<Dword>(engine, 4) - Dword(interval) * 500u))) * remainder + first;
    return retain_float(0) < result ? result : retain_float(0);
}


static long double calculate_brake_torque(float diameter, Dword compound, int attachment, float pressure) {
    const auto radius = retain_float(diameter) * .5f;
    const float friction = compound == 0 ? .85f : compound == 1 ? .9f : compound == 2 ? .95f : 1.f;
    auto result = radius * radius * 3.1415927410125732421875f * 16.31012725830078125f * 2500.f * friction * pressure;
    if (attachment == 1)
        result = result * .98f;
    return result;
}


static long double calculate_tire_lateral_force(void *tire, void *wheel) {
    const auto slip = retain_float(at<float>(wheel, 20)) * at<float>(tire, 40) * 180.f;
    const float signed_slip = float(slip);
    const auto magnitude = slip < retain_float(0) ? -slip : slip;
    const float degrees = magnitude < retain_float(25) ? float(magnitude) : 25.f;
    const int interval = legacy::slip_interval_f32(degrees);
    const auto first = retain_float(at<float>(tire, 56 + Dword(interval) * 4)) * at<float>(tire, 36);
    const float next = at<float>(tire, interval < 25 ? 60 + Dword(interval) * 4 : 156);
    const auto curve = (retain_float(next) * at<float>(tire, 36) - first) *
                           (retain_float(degrees) - retain_integer(interval)) +
                       first;
    auto load = retain_float(at<float>(wheel, 232)) * .00025f;
    if (load < retain_float(0))
        load = retain_float(0);
    else if (retain_float(2.5f) < load)
        load = retain_float(2.5f);
    load = (retain_float(1.24532997608184814453125f) - retain_float(0.2425740063190460205078125f) * load) * load + 0.0072599998675286769866943359375f;
    if (load < retain_float(0))
        load = retain_float(0);
    else if (retain_float(1.64999997615814208984375f) < load)
        load = retain_float(1.64999997615814208984375f);
    auto force = (load * curve + retain_float(at<float>(wheel, 24)) * 180.f * at<float>(tire, 28) *
                                     at<float>(wheel, 232)) *
                 at<float>(wheel, 268);
    if (signed_slip > 0)
        force = -force;
    return force;
}


static long double calculate_tire_aligning_moment(void *tire, void *wheel) {
    const auto slip = retain_float(at<float>(wheel, 20)) * at<float>(tire, 40) * 180.f;
    const float signed_slip = float(slip);
    const auto magnitude = slip < retain_float(0) ? -slip : slip;
    const float degrees = magnitude < retain_float(25) ? float(magnitude) : 25.f;
    const int interval = legacy::slip_interval_f32(degrees);
    const auto first = retain_float(at<float>(tire, 160 + Dword(interval) * 4)) * at<float>(tire, 44);
    const float next = at<float>(tire, interval < 25 ? 164 + Dword(interval) * 4 : 260);
    const auto curve = (retain_float(next) * at<float>(tire, 44) - first) *
                           (retain_float(degrees) - retain_integer(interval)) +
                       first;
    auto load = retain_float(at<float>(wheel, 232)) * .00025f;
    if (retain_float(1) < load) {
        load = load * 2.8299999237060546875f - 2.f;
        if (load < retain_float(1))
            load = retain_float(1);
        else if (retain_float(5) < load)
            load = retain_float(5);
    } else if (load < retain_float(0))
        load = retain_float(0);
    auto moment = (load * curve + retain_float(at<float>(wheel, 24)) * 180.f * at<float>(tire, 28) *
                                      at<float>(wheel, 232) * .15f) *
                  at<float>(wheel, 268);
    if (signed_slip < 0)
        moment = -moment;
    return moment;
}


static long double calculate_rolling_resistance(float speed) {
    if (speed < 1)
        return retain_float(0);
    return retain_float(speed) * speed * 0.000002739999899858958087861537933349609375f + .011f;
}
namespace curves_x87 {
void engine_torque_native80(void *engine, int rpm, void *output) {
    const long double result = calculate_engine_torque(engine, rpm);
    std::memcpy(output, &result, 10);
}

void brake_torque_native80(float diameter, Dword compound, int attachment, float pressure, void *output) {
    const long double result = calculate_brake_torque(diameter, compound, attachment, pressure);
    std::memcpy(output, &result, 10);
}

void tire_lateral_force_native80(void *tire, void *wheel, void *output) {
    const long double result = calculate_tire_lateral_force(tire, wheel);
    std::memcpy(output, &result, 10);
}

void tire_aligning_moment_native80(void *tire, void *wheel, void *output) {
    const long double result = calculate_tire_aligning_moment(tire, wheel);
    std::memcpy(output, &result, 10);
}

void rolling_resistance_native80(float speed, void *output) {
    const long double result = calculate_rolling_resistance(speed);
    std::memcpy(output, &result, 10);
}
}
} // namespace

namespace nfs::game::runtime::physics::legacy {
// The native physics callers store the retained result directly to F32.
// Keep these pointer/scalar interfaces independent of the public Extended ABI.
void tire_lateral_f32(void *tire, void *wheel, float *output) {
    *output = static_cast<float>(world::vehicle_setup::calculate_tire_lateral_force(tire, wheel));
}

void tire_aligning_f32(void *tire, void *wheel, float *output) {
    *output = static_cast<float>(world::vehicle_setup::calculate_tire_aligning_moment(tire, wheel));
}

void engine_torque_f32(void *engine, int rpm, float *output) {
    *output = static_cast<float>(world::vehicle_setup::calculate_engine_torque(engine, rpm));
}

int speed_to_rpm(float speed) {
    constexpr float radians_per_second_to_rpm = 9.54929637908935546875f;
    const long double rpm = static_cast<long double>(speed) * radians_per_second_to_rpm + .5f;
    return car_frame::drive::truncate_engine_rpm(&rpm);
}

int round_road_rpm(float rpm) {
    // F32 reload followed by FISTP DWORD; keep the active rounding mode.
    return hardware::current_integer_f32(rpm);
}
} // namespace nfs::game::runtime::physics::legacy
