#include "car_asset_parameters.hpp"
#include "nfs/core/configuration/launch_options.hpp"
#include <cstdio>
#include <cstring>
namespace nfs::game::world::vehicle_setup {
// The saved implicit receiver is car+1F4.
bool load_physical_parameters(void *car, const char *name, Dword kind, Dword automatic) {
    const char *format;
    if (kind == 1 || launch::field<Dword>(8627700))
        format = "%s%s\\car_hp.ini";
    else if (kind == 3 || launch::field<Dword>(8627704))
        format = "%s%s\\car_cop.ini";
    else if (kind == 2)
        format = "%s%s\\carracer.ini";
    else
        format = "%s%s\\car.ini";
    char filename[260];
    std::sprintf(filename, format, bytes(subsystem(4)) + 2084, name);
    ini::Reader reader;
    ini::reader_construct(&reader, filename, 0);
    try {
        at<Dword>(car, 1564) = 0;
        std::strcpy(reinterpret_cast<char *>(bytes(car) + 1532), name);
        bool valid = read_basic_parameters(&reader, car, "basic") &&
                     read_engine_parameters(&reader, car, "engineblock") &&
                     read_transmission_parameters(&reader, car, "transmission", Byte(automatic));
        if (valid) {
            float ratio = 0;
            valid = ini::get_float(&reader, "rearend", "FinalGearRatio", &ratio);
            if (!valid)
                report_missing_parameter(&reader, "rearend", "FinalGearRatio");
            at<float>(car, 884) = ratio;
            if (!read_parameter_float(&reader, "rearend", "FrontFinalGearRatio", &ratio))
                ratio = 0;
            at<float>(car, 888) = ratio;
            at<Dword>(car, 896) = 1;
            at<float>(car, 892) = 1;
            auto brake = [&](const char *section) {
                const auto numeric_format = kernel::numeric::current_format();
                at<float>(car, 520) =
                    float(Extended(at<float>(car, 520), numeric_format) + 8.f);
                at<float>(car, 524) =
                    float(Extended(at<float>(car, 524), numeric_format) + 8.f);
                int attachment = 0;
                const bool present = ini::get_int(&reader, section, "attach", &attachment);
                if (!present)
                    report_missing_parameter(&reader, section, "attach");
                void *target = bytes(car) + 500 + (attachment == 10 ? 964 : 980);
                if (attachment == 10)
                    return read_brake_parameters(&reader, target, section);
                return present && read_brake_parameters(&reader, target, section);
            };
            valid = valid && brake("brake_front") && brake("brake_rear") &&
                    read_tire_attachment(&reader, bytes(car) + 500, car, "tire_front") &&
                    read_tire_attachment(&reader, bytes(car) + 500, car, "tire_rear");
        }
        char horn[32] = {};
        const bool horn_present =
            read_parameter_string(&reader, "horn", "dbAudio", horn, sizeof horn);
        if (!(horn_present && std::strncmp(horn, "NULL", 5) == 0) && horn[0]) {
            std::strncpy(reinterpret_cast<char *>(bytes(car) + 1500), horn, 32);
            at<Dword>(car, 1496) = 1;
        }
        ini::reader_clear(&reader);
        return valid;
    } catch (...) {
        ini::reader_clear(&reader);
        throw;
    }
}
} // namespace nfs::game::world::vehicle_setup
