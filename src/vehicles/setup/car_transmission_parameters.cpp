#include "car_asset_parameters.hpp"
#include <cstdio>
namespace nfs::game::world::vehicle_setup {

void clear_transmission_curve(void *transmission) {
    kernel::memory_zero(bytes(transmission) + 76, 32);
}

bool read_transmission_parameters(ini::Reader *reader, void *car, const char *section,
                                  Byte automatic) {
    const auto format = kernel::numeric::current_format();
    auto E = [&](float value) { return Extended(value, format); };
    float value = 0;
    int integer = 0;
    at<Byte>(car, 776) = automatic;
    auto get_integer = [&](const char *key) {
        if (ini::get_int(reader, section, key, &integer))
            return true;
        report_missing_parameter(reader, section, key);
        return false;
    };
    if (!get_integer("NumberOfGears"))
        return false;
    at<int>(car, 780) = integer;
    if (!get_integer("GearShiftDelay"))
        return false;
    if (!automatic)
        integer = int(Dword(integer) + 12);
    at<int>(car, 784) = integer;
    const float efficiency_factor = automatic ? 0.9700000286102294921875f : 1.f;
    for (int gear = 0; gear < 8; ++gear) {
        char key[20];
        std::sprintf(key, "GearRatios_%d", gear);
        if (!ini::get_float(reader, section, key, &value)) {
            report_missing_parameter(reader, section, key);
            at<Dword>(car, 788 + gear * 4) = 0;
        } else
            at<float>(car, 788 + gear * 4) = value;
    }
    for (int gear = 0; gear < 8; ++gear) {
        char key[20];
        std::sprintf(key, "GearEfficiency_%d", gear);
        if (!ini::get_float(reader, section, key, &value)) {
            report_missing_parameter(reader, section, key);
            value = 1;
        }
        value = float(E(efficiency_factor) * value);
        at<float>(car, 820 + gear * 4) = value;
    }
    clear_transmission_curve(bytes(car) + 776);
    if (!read_parameter_float(reader, section, "TorqueConverterRatio", &value))
        value = 0;
    at<float>(car, 900) = value;
    if (!get_integer("TorqueConverterMaxRPM"))
        integer = 0;
    at<int>(car, 904) = integer;
    if (!read_parameter_float(reader, section, "GearShiftPointCoefficient", &value))
        value = .9f;
    at<float>(car, 908) = float(clamp_parameter(value, .75f, 1.2f));
    return true;
}
} // namespace nfs::game::world::vehicle_setup
