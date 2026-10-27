#include "nfs/core/lifecycle/scope_cleanup.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
#include "controller_interfaces.hpp"
#include "nfs/resources/ini/ini.hpp"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cctype>
namespace nfs::controllers {

Dword resolve_control_reference(const char *name) {
    return gui::string_resolver ? invoke<Dword>(gui::string_resolver, 3, name) : 0;
}
// Retain the32-byte lowercase loop and fixed-size comparisons.
Byte parse_direction(const char *text) {
    if (!text)
        return 0;
    char lowered[32];
    lowered[31] = 0;
    for (unsigned index = 0; index < 32; ++index) {
        lowered[index] = char(std::tolower(int(text[index])));
        if (!lowered[index])
            break;
    }
    if (!std::strncmp(lowered, "up", 3))
        return 1;
    if (!std::strncmp(lowered, "down", 5))
        return 2;
    if (!std::strncmp(lowered, "left", 5))
        return 3;
    return !std::strncmp(lowered, "right", 6) ? 4 : 0;
}
namespace {
Dword control_reference(const char *name) {
    const Dword reference = resolve_control_reference(name);
    return reference ? reference : 758;
}
void resolve_control_label(Dword reference) {
    LocalFrontendString label;
    gui::string_reference(&label.value, reference);
}
} // namespace

int load_definitions(void *owner) {
    if (at<Byte>(owner, 0))
        return 0;
    const void *paths = system(4);
    at<Byte>(owner, 0) = 1;
    int loaded = 0;
    char filename[260];
    std::sprintf(filename, "%s%s", reinterpret_cast<const char *>(bytes(paths) + 3904),
                 "definitions.ini");
    if (!files::file::exists(filename))
        return 0;
    ini::Reader reader;
    ini::reader_construct(&reader, filename, 0);
    kernel::lifecycle::Cleanup reader_cleanup([&] { ini::reader_clear(&reader); });
    {
        for (int device = 0; device < 48; ++device) {
            char section[128], name[256];
            std::sprintf(section, "joystick %d", device);
            if (!ini::get_string(&reader, section, "name", name, 255))
                break;
            lower_controller_name(name);
            ++loaded;
            auto *joystick = static_cast<Joystick *>(
                lifecycle::allocate(1124, "Controllers::JoyStick", lifecycle::ui_pool));
            {
                kernel::lifecycle::Cleanup allocation([&] {
                    game::runtime::track::pending::release_named(joystick, "Controllers::JoyStick", lifecycle::ui_pool);
                });
                if (joystick) {
                construct_controller(&joystick->base, device, name);
                kernel::lifecycle::Cleanup controller([&] { destroy_keyboard(joystick); });
                construct_axis_list(&joystick->axes);
                kernel::lifecycle::Cleanup axes([&] { destroy_axis_list(&joystick->axes); });
                construct_button_list(&joystick->buttons);
                kernel::lifecycle::Cleanup buttons([&] { destroy_button_list(&joystick->buttons); });
                construct_pov_list(&joystick->povs);
                kernel::lifecycle::Cleanup povs([&] { destroy_pov_list(&joystick->povs); });
                joystick->base.methods = joystick_methods;
                povs.dismiss();buttons.dismiss();axes.dismiss();controller.dismiss();
                }
                allocation.dismiss();
            }
            const Dword count = at<Dword>(owner, 4);
            at<void *>(owner, 8 + count * 4) = joystick;
            at<Dword>(owner, 4) = count + 1;
            unsigned index = 0;
            for (; index < 24; ++index) {
                char key[32], text[256];
                std::sprintf(key, "button%d", index);
                if (!ini::get_string(&reader, section, key, text, 255))
                    break;
                if (ControlValue *value = button_slot(joystick, index)) {
                    const Dword reference = control_reference(text);
                    value->index = index;
                    set_control_reference(value, reference);
                    resolve_control_label(value->reference);
                }
            }
            joystick->buttons.count = index;
            for (index = 0; index < 24; ++index) {
                char key[32], text[256];
                std::sprintf(key, "axis%d", index);
                if (!ini::get_string(&reader, section, key, text, 255))
                    break;
                Dword axis = 0, polarity = 0, minimum = 0, maximum = 0;
                const char *reference_name = nullptr;
                if (char *token = std::strtok(text, ",")) {
                    axis = Dword(std::atoi(token));
                    if ((token = std::strtok(nullptr, ",")) != nullptr) {
                        polarity = parse_direction(token);
                        if ((token = std::strtok(nullptr, ",")) != nullptr) {
                            minimum = Dword(std::atoi(token));
                            if ((token = std::strtok(nullptr, ",")) != nullptr) {
                                maximum = Dword(std::atoi(token));
                                reference_name = std::strtok(nullptr, ",");
                            }
                        }
                    }
                }
                if (AxisValue *value = axis_slot(joystick, index)) {
                    set_axis(value, axis, control_reference(reference_name), minimum, maximum,
                             polarity);
                    resolve_control_label(value->reference);
                }
            }
            joystick->axes.count = index;
            for (index = 0; index < 4; ++index) {
                char key[32], text[256];
                std::sprintf(key, "pov%d", index);
                if (!ini::get_string(&reader, section, key, text, 255))
                    break;
                Dword pov = 0, direction = 0;
                const char *reference_name = nullptr;
                if (char *token = std::strtok(text, ",")) {
                    pov = Dword(std::atoi(token));
                    if ((token = std::strtok(nullptr, ",")) != nullptr) {
                        direction = parse_direction(token);
                        reference_name = std::strtok(nullptr, ",");
                    }
                }
                if (ControlValue *value = pov_slot(joystick, index)) {
                    set_pov(value, pov, control_reference(reference_name), direction);
                    resolve_control_label(value->reference);
                }
            }
            joystick->povs.count = index;
        }
    }
    reader_cleanup.run();
    return loaded;
}
} // namespace nfs::controllers
