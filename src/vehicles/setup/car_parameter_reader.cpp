#include "car_asset_parameters.hpp"
namespace nfs::game::world::vehicle_setup {

const gui::String *parameter_filename(const ini::Reader *reader) {
    if (reader->document && reader->document->document.directory)
        return &reader->document->document.directory->string;
    return &lifecycle::construction::unresolved::shared_string;
}

const gui::String *parameter_directory(const ini::Reader *reader) {
    if (reader->document && reader->document->document.name)
        return &reader->document->document.name->string;
    return &lifecycle::construction::unresolved::shared_string;
}
void report_missing_parameter(const ini::Reader *reader, const char *section, const char *key) {
    const auto *filename = parameter_filename(reader);
    const char *filename_text =
        gui::string_equal(filename, &lifecycle::construction::unresolved::shared_string)
            ? ""
            : gui::string_ascii(parameter_filename(reader));
    const char *directory_text = gui::string_ascii(parameter_directory(reader));

    (void)section;
    (void)key;
    (void)filename_text;
    (void)directory_text;
    runtime::particles::lifecycle::pending::diagnostic_message(
        "ERROR: [%14s] %-27s missing from %s %s\r\n");
}

Byte read_parameter_float(ini::Reader *reader, const char *section, const char *key, float *value) {
    const Byte present = ini::get_float(reader, section, key, value);
    if (!present)
        report_missing_parameter(reader, section, key);
    return present;
}

Byte read_parameter_string(ini::Reader *reader, const char *section, const char *key, char *value,
                           Dword capacity) {
    const Byte present = Byte(ini::get_string(reader, section, key, value, capacity));
    if (!present)
        report_missing_parameter(reader, section, key);
    return present;
}

Extended clamp_parameter(float value, float minimum, float maximum) {
    const auto format = kernel::numeric::current_format();
    if (value < minimum)
        return Extended(minimum, format);
    if (value > maximum)
        return Extended(maximum, format);
    return Extended(value, format);
}
} // namespace nfs::game::world::vehicle_setup
