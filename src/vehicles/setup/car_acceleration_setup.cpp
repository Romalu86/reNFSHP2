#include "car_initialization_runtime.hpp"
namespace nfs::game::world::vehicle_setup {
namespace {
void road_load_curve(void *car, float *resistance) {
    const auto format = kernel::numeric::current_format();
    const float area = at<float>(car, 120), drag = at<float>(car, 500),
                mass = at<float>(car, 520);
    for (int speed = 0; speed < 135; ++speed) {
        const auto rolling = rolling_resistance(float(speed)) * 1.1f;
        const auto aerodynamic =
            Extended::integer(speed * speed, format) * drag * area * 0.59049999713897705078125f;
        resistance[speed] = float(Extended(9.81000041961669921875f, format) * mass * rolling + aerodynamic);
    }
}
} // namespace

Extended gear_shift_speed(void *car, int gear) {
    const auto format = kernel::numeric::current_format();
    auto E = [&](float value) { return Extended(value, format); };
    auto I = [&](int value) { return Extended::integer(value, format); };
    if (gear < 2 || gear > int(at<Dword>(car, 780) - 2))
        return E(-1);
    float current[135] = {}, next[135] = {}, resistance[135] = {};
    build_gear_force_curve(car, gear, current);
    build_gear_force_curve(car, gear + 1, next);
    road_load_curve(car, resistance);
    for (unsigned speed = 0; speed < 135; ++speed) {
        auto remaining = E(current[speed]) - resistance[speed];
        current[speed] = float(remaining);
        if (!(E(0) < remaining))
            current[speed] = 0;
        remaining = E(next[speed]) - resistance[speed];
        next[speed] = float(remaining);
        if (!(E(0) < remaining))
            next[speed] = 0;
    }
    for (int speed = 1; speed < 135; ++speed) {
        if (!(current[speed] < next[speed]) || !(current[speed - 1] >= next[speed - 1]))
            continue;
        const auto previous = I(speed - 1), index = I(speed);
        const auto denominator =
            ((previous * next[speed] - index * next[speed - 1]) + index * current[speed - 1]) -
            previous * current[speed];
        const auto numerator =
            ((E(current[speed - 1]) - current[speed]) + next[speed]) - next[speed - 1];

        const float crossing = float(denominator / numerator);
        const float stored_rpm = float(E(at<float>(car, 184 + Dword(gear) * 4)) * crossing);
        const auto rounded = E(stored_rpm).round_to_integer();
        const int rpm = rounded < INT32_MIN || rounded > INT32_MAX ? INT32_MIN : int(rounded);
        const int midpoint = int(at<Dword>(car, 660) + at<Dword>(car, 656)) / 2;
        if (rpm >= midpoint)
            return E(at<float>(car, 908)) * crossing;
    }
    const int rpm =
        gear == 2 ? int(at<Dword>(car, 660) * 7u) / 10 : int(at<Dword>(car, 660) * 3u) / 4;
    return I(rpm) / E(at<float>(car, 184 + Dword(gear) * 4));
}

void initialize_top_speed(void *car) {
    const auto format = kernel::numeric::current_format();
    float forces[135] = {}, resistance[135] = {};
    const int gear_count = at<int>(car, 780);
    build_gear_force_curve(car, int(Dword(gear_count) - 1), forces);
    const float drag = at<float>(car, 500);
    road_load_curve(car, resistance);
    at<Dword>(car, 260) = 0;
    auto crossing = [&]() {
        for (int speed = 135; speed >= 20; --speed) {
            // -10h is drag, immediately after the 135 force entries. Resistance entry135 is the adjoining
            // force entry0, not an unwritten value.
            const float current_force = speed == 135 ? drag : forces[speed];
            const float previous_force = forces[speed - 1];
            const float current_resistance = speed == 135 ? forces[0] : resistance[speed];
            const float previous_resistance = resistance[speed - 1];
            const bool exhausted =
                current_force == 0 && previous_force != 0 && previous_force > previous_resistance;
            const bool below =
                (current_force < current_resistance || current_force == current_resistance) &&
                !(previous_force < previous_resistance) && !std::isnan(previous_force) &&
                !std::isnan(previous_resistance);
            if (exhausted || below)
                return speed;
        }
        return 20;
    };
    at<float>(car, 260) = float(Extended::integer(crossing(), format));
    kernel::memory_zero(forces, sizeof forces);
    build_gear_force_curve(car, int(Dword(gear_count) - 2), forces);
    const float second = float(Extended::integer(crossing(), format));
    if (!(at<float>(car, 260) > second))
        at<float>(car, 260) = second;
}
} // namespace nfs::game::world::vehicle_setup
