#include "car_asset_parameters.hpp"
namespace nfs::game::world::vehicle_setup {

bool read_basic_parameters(ini::Reader *reader, void *car, const char *section) {
    const auto format = kernel::numeric::current_format();
    auto E = [&](float value) { return Extended(value, format); };
    at<Dword>(car, 1568) = 1073741824;
    at<Dword>(car, 1572) = 20;
    int integer = 0;
    float value = 1;
    auto get_float = [&](const char *key) {
        if (ini::get_float(reader, section, key, &value))
            return true;
        report_missing_parameter(reader, section, key);
        return false;
    };
    auto get_integer = [&](const char *key) {
        if (ini::get_int(reader, section, key, &integer))
            return true;
        report_missing_parameter(reader, section, key);
        return false;
    };
    get_float("AISpeedFactor");
    at<float>(car, 576) = float(clamp_parameter(value, 0, 1));
    bool valid = get_float("AeroDragCoefficient");
    at<float>(car, 500) = value;
    valid = valid && get_float("AeroFrontDownforceCoefficient");
    at<float>(car, 504) = value;
    valid = valid && get_float("AeroRearDownforceCoefficient");
    at<float>(car, 508) = value;
    valid = valid && get_float("BodyLiftCoefficient");
    at<float>(car, 512) = value;
    value = 1.1f;
    get_float("UnderBodyDragCoefficient");
    at<float>(car, 516) = value;
    valid = valid && get_float("Mass");
    at<float>(car, 520) = value;
    at<float>(car, 524) = float(E(value) * .1f);
    valid = valid && get_float("WheelBase");
    at<float>(car, 528) = value;
    valid = valid && get_float("FrontTrackWidth");
    at<float>(car, 532) = value;
    valid = valid && get_float("RearTrackWidth");
    at<float>(car, 536) = value;
    valid = valid && get_float("VehicleHeight");
    at<float>(car, 540) = value;
    valid = valid && get_float("HeightOfGravity");
    at<float>(car, 544) = value;
    valid = valid && get_float("FrontGripBias");
    at<float>(car, 548) = value;
    valid = valid && get_integer("BodyShape");
    at<int>(car, 552) = integer;
    at<Dword>(car, 556) = 90;
    value = 0;
    get_float("SpeedGovernor");
    at<float>(car, 572) = value;
    valid = valid && get_float("FrontDriveRatio");
    at<float>(car, 560) = value;
    for (unsigned wheel = 0; wheel < 4; ++wheel) {
        const char *keys[] = {"driveWheels_0", "driveWheels_1", "driveWheels_2", "driveWheels_3"};
        valid = valid && get_integer(keys[wheel]);
        at<Byte>(car, 568 + wheel) = Byte(integer);
    }
    if (!at<Byte>(car, 568) && !at<Byte>(car, 569) && !at<Byte>(car, 570) && !Byte(integer))
        at<Byte>(car, 570) = at<Byte>(car, 571) = 1;
    at<Byte>(car, 584) = 0;
    valid = valid && get_float("TurningCircleRadius");
    at<float>(car, 588) = value;
    valid = valid && get_float("TurninRamp");
    at<float>(car, 592) = float(E(value) * 0.0078125f);
    valid = valid && get_float("TurnoutRamp");
    at<float>(car, 596) = float(E(value) * 0.0078125f);
    value = 8.94060039520263671875f;
    get_float("BaseDampingVelocity");
    at<float>(car, 600) = value;
    value = 22.3515f;
    get_float("RangeDampingVelocity");
    at<float>(car, 604) = value;
    value = 1;
    get_float("DampingLimit");
    at<float>(car, 608) = value;
    value = 1;
    get_float("YawInertiaFactor");
    value = float(clamp_parameter(value, .3f, 2));
    at<float>(car, 612) = value;
    valid = valid && get_float("FrontRollStiffness");
    at<float>(car, 616) = value;
    valid = valid && read_parameter_float(reader, section, "RearRollStiffness", &value);
    at<float>(car, 620) = value;
    valid = valid && read_parameter_float(reader, section, "AntiDive", &value);
    at<float>(car, 624) = value;
    valid = valid && read_parameter_float(reader, section, "AntiSquat", &value);
    at<float>(car, 628) = value;
    value = 0;
    read_parameter_float(reader, section, "FrontAntiRoll", &value);
    at<float>(car, 624) = value;
    value = 0;
    read_parameter_float(reader, section, "RearAntiRoll", &value);
    at<float>(car, 628) = value;
    valid = valid && read_parameter_float(reader, section, "Damper", &value);
    at<float>(car, 640) = at<float>(car, 644) = value;
    valid = valid && read_parameter_float(reader, section, "TravelFront", &value);
    at<float>(car, 648) = value;
    valid = valid && read_parameter_float(reader, section, "TravelRear", &value);
    at<float>(car, 652) = value;
    value = .85f;
    read_parameter_float(reader, section, "RearSlideFactor", &value);
    at<float>(car, 564) = value > 0 ? value : 0;
    at<float>(car, 580) = 1;
    return valid;
}
} // namespace nfs::game::world::vehicle_setup
