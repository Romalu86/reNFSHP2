#include "car_asset_parameters.hpp"
#include <cstdio>
#include <cstring>
namespace nfs::game::world::vehicle_setup {

void set_engine_audio_tag(void *engine, const char *tag) {
    std::strncpy(reinterpret_cast<char *>(bytes(engine) + 8), tag, 3);
    at<Byte>(engine, 11) = 0;
}

bool read_engine_parameters(ini::Reader *reader, void *car, const char *section) {
    const auto format = kernel::numeric::current_format();
    auto E = [&](float value) { return Extended(value, format); };
    float value = 0;
    int rpm = 0;
    auto get_integer = [&](const char *key) {
        if (ini::get_int(reader, section, key, &rpm))
            return true;
        report_missing_parameter(reader, section, key);
        return false;
    };
    auto get_float = [&](const char *key) {
        if (ini::get_float(reader, section, key, &value))
            return true;
        report_missing_parameter(reader, section, key);
        return false;
    };
    bool valid = get_integer("MinRPM");
    at<int>(car, 656) = rpm;
    valid = valid && get_integer("RedLine");
    at<int>(car, 660) = rpm;
    rpm = int(Dword(rpm) * 7u) / 8;
    get_integer("WarningLine");
    at<int>(car, 772) = rpm < at<int>(car, 660) ? rpm : at<int>(car, 660);
    int last_sample = 0;
    float last_torque = 0, decay = .8f;
    for (int sample = 0; sample < 25; ++sample) {
        char key[24];
        std::sprintf(key, "torqueCurve_%d", sample);
        if (!get_float(key))
            at<Dword>(car, 668 + sample * 4) = 0;
        else {
            at<float>(car, 668 + sample * 4) = value;
            if (value != 0) {
                if (last_torque != 0 && value < last_torque)
                    decay = float(E(value) / E(last_torque));
                last_torque = value;
                last_sample = sample;
            }
        }
    }
    struct AudioTagScratch {
        char tag[4];
        Dword curve_end;
    } scratch;
    // The curve cursor at byte offset -20 follows the four-byte tag at -24. Required-field failure
    // leaves tag bytes uninitialized.
    scratch.curve_end = reinterpret_cast<Dword>(bytes(car) + 768);
    for (int sample = last_sample + 1; sample < 25; ++sample) {
        last_torque = float(E(decay) * last_torque);
        decay = float(E(decay) * decay);
        at<float>(car, 668 + sample * 4) = last_torque;
    }
    if (!get_float("FlyWheelInertiaFactor"))
        value = 1;
    at<float>(car, 768) = value;
    valid = valid && read_parameter_string(reader, section, "dbAudio_1", scratch.tag, 4);
    if (valid && std::strncmp(reinterpret_cast<const char *>(&scratch), "NULL", 5) == 0)
        scratch.tag[0] = 0;
    set_engine_audio_tag(bytes(car) + 656, scratch.tag);
    at<Dword>(car, 60) = at<Dword>(car, 64) = 0;
    return valid;
}
} // namespace nfs::game::world::vehicle_setup
