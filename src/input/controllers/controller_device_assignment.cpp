#include "controller_configuration.hpp"
#include "nfs/core/services/service_lifecycle.hpp"
#include <cstring>
namespace nfs::services::pending {
// First argument is unused by the implementation.
void configure_input_devices(Dword, int selected) {
    using namespace application::pending;
    controllers::get_definitions();
    void *input_owner = system(100);
    controllers::clear_device_assignments(input_owner);
    const int count = int(reinterpret_cast<Dword>(input::device_get(0, 1)));
    void *controller_list = controllers::get_bindings();
    for (int index = 0; index < count; ++index) {
        if (selected != -1 && selected != index)
            continue;
        const void *about = input::device_get(index, 2);
        if (!about || !(at<Dword>(about, 4) & 1) || at<Byte>(about, 0) != 4)
            continue;
        controllers::enable_device_assignment(input_owner, Dword(index), 1);
        char name[128];
        name[127] = 0;
        std::strncpy(name, reinterpret_cast<const char *>(bytes(about) + 280), 127);
        controllers::lower_controller_name(name);
        char *end = std::strchr(name, 0);
        if (end && end - 1 > name)
            for (char *tail = end - 1; tail > name && *tail == ' '; --tail)
                *tail = 0;
        if (!controllers::add_controller(controller_list, name, Dword(index))) {
            LocalFrontendString format;
            gui::string_reference(&format.value, 1966);
            LocalFrontendString message;
            gui::string_copy(&message.value,
                             &lifecycle::construction::unresolved::shared_string);
            LocalFrontendString controller(name);
            gui::string_make_wide(&controller.value);
            gui::string_format(&message.value, &format.value, gui::string_data(&controller.value));
            MessageBox warning;
            {
                LocalFrontendString title("Controller NR");
                construct_message_box(&warning, &message.value, &title.value, 8449);
            }
            try {
                run_message_box(&warning, nullptr, 0);
                const Byte kind = at<Byte>(about, 1);
                const char *fallback = (kind == 4 || kind == 6) ? "defaultpad" : "defaultwheel";
                controllers::add_controller(controller_list, fallback, Dword(index));
            } catch (...) {
                destroy_message_box(&warning);
                throw;
            }
            destroy_message_box(&warning);
        }
    }
    profiles::pending::configure_profile_input();
}
} // namespace nfs::services::pending
