#include "car_asset_parameters.hpp"
#include <cstring>
namespace nfs::game::world::vehicle_setup {
const Dword tire_presets[754] = {
#include "car_tire_presets.inc"
};

void clear_tire_lateral_curve(void *tire) {
    kernel::memory_zero(bytes(tire) + 56, 104);
}

void clear_tire_aligning_curve(void *tire) {
    kernel::memory_zero(bytes(tire) + 160, 104);
}

void copy_tire_lateral_curve(void *tire, int index, const void *samples, Dword length) {
    kernel::memory_copy(bytes(tire) + 56 + Dword(index) * 4, samples, length);
}

void copy_tire_aligning_curve(void *tire, int index, const void *samples, Dword length) {
    kernel::memory_copy(bytes(tire) + 160 + Dword(index) * 4, samples, length);
}

bool read_brake_parameters(ini::Reader *reader, void *brake, const char *section) {
    const auto format = kernel::numeric::current_format();
    int value = 0;
    auto get = [&](const char *key) {
        if (ini::get_int(reader, section, key, &value))
            return true;
        report_missing_parameter(reader, section, key);
        return false;
    };
    at<Dword>(brake, 0) = 0;
    bool valid = get("BrakeMaterialType");
    at<int>(brake, 4) = value;
    valid = valid && get("BrakeDiameter");
    at<float>(brake, 8) = float(Extended::integer(value, format) * .001f);
    valid = valid && get("ABS");
    at<int>(brake, 12) = value;
    return valid;
}

bool read_tire_attachment(ini::Reader *reader, void *parameters, void *car, const char *section) {
    const auto format = kernel::numeric::current_format();
    auto E = [&](float value) { return Extended(value, format); };
    at<float>(car, 520) = float(E(at<float>(car, 520)) + 12.f);
    at<float>(car, 524) = float(E(at<float>(car, 524)) + 12.f);
    // SectionWidth reuses the saved stack argument initially containing the car pointer.
    int width = int(reinterpret_cast<Dword>(car));
    const bool width_present = ini::get_int(reader, section, "SectionWidth", &width);
    if (!width_present)
        report_missing_parameter(reader, section, "SectionWidth");
    auto width_value = Extended::integer(width, format);
    if (E(215) < width_value) {
        auto extra_drag = (width_value - 215.f) * .00004f;
        if (extra_drag < E(0))
            extra_drag = E(0);
        else if (E(.005f) < extra_drag)
            extra_drag = E(.005f);
        at<float>(car, 500) = float(extra_drag + at<float>(car, 500));
    }
    if (!width_present)
        return false;
    int attachment;
    if (!ini::get_int(reader, section, "attach", &attachment)) {
        report_missing_parameter(reader, section, "attach");
        return false;
    }
    void *tire = bytes(parameters) + (attachment == 10 ? 412 : 688);
    return read_tire_parameters(reader, tire, section, width);
}

bool read_tire_parameters(ini::Reader *reader, void *tire, const char *section, int width) {
    const auto format = kernel::numeric::current_format();
    auto E = [&](float value) { return Extended(value, format); };
    float value = 0;
    int integer = 0;
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
    at<Dword>(tire, 12) = 0;
    if (!get_float("RollingResistanceFactor"))
        value = 1;
    at<float>(tire, 20) = value;
    at<float>(tire, 32) = 10;
    if (!get_float("PeakFriction"))
        value = 1.6f;
    at<int>(tire, 0) = width;
    at<float>(tire, 24) = value;
    at<float>(tire, 28) = .01f;
    at<Dword>(tire, 16) = 0;
    bool valid = get_integer("AspectRatio");
    at<int>(tire, 4) = integer;
    valid = valid && get_integer("WheelDiameter");
    at<int>(tire, 8) = integer;
    if (!get_float("ForceScale"))
        value = 1;
    at<float>(tire, 36) = value;
    if (!get_float("SlipAngleScale"))
        value = 1;
    at<float>(tire, 40) = value;
    at<float>(tire, 28) = .01f;
    if (!get_float("AligningMomentScale"))
        value = 1;
    at<float>(tire, 44) = value;
    clear_tire_lateral_curve(tire);
    clear_tire_aligning_curve(tire);
    valid = valid && get_integer("TypeCode");
    const Byte *preset = bytes(tire_presets) + Dword(integer) * 232;
    at<Dword>(tire, 272) = at<Dword>(preset, 28);
    if (!get_float("SlipRatio"))
        value = 1;
    at<float>(tire, 264) = value;
    copy_tire_lateral_curve(tire, 1, preset + 32, 100);
    copy_tire_aligning_curve(tire, 1, preset + 132, 100);
    float peak = at<float>(tire, 24);
    const float scale = at<float>(tire, 36);
    for (unsigned sample = 0; sample < 25; ++sample) {
        const float force = float(E(scale) * at<float>(preset, 32 + sample * 4) * .00025f);
        if (!(peak > force))
            peak = force;
    }
    at<float>(tire, 24) = peak;
    at<Dword>(&value, 0) = 1033637567;
    get_float("MaxRenderTravelUp");
    at<float>(tire, 52) = value;
    at<Dword>(&value, 0) = 3189509823u;
    get_float("MaxRenderTravelDown");
    at<float>(tire, 48) = value;
    value = .4f;
    read_parameter_float(reader, section, "TractionCoefft", &value);
    at<float>(tire, 268) = value;
    return valid;
}
} // namespace nfs::game::world::vehicle_setup
