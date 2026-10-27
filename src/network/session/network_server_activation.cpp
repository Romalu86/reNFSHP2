#include "network_lifecycle_runtime.hpp"
#include "server_runtime.hpp"
#include "nfs/gui/resources/gui_format.hpp"
namespace nfs::services::pending {

void activate_server(void *owner, Dword dedicated) {
    using namespace network::session;
    at<Byte>(server_settings, 692) = dedicated == 1;
    at<Dword>(server_settings, 668) = at<Byte>(owner, 96);
    at<Dword>(owner, 228) = dedicated;
    listen_transport(network::transport);
    application::pending::rotate_server_race(owner);
    network::saved_graphics_enabled = network::graphics_enabled;
    at<Dword>(owner, 44) = 1;
    at<Dword>(owner, 48) = dedicated;
    if (dedicated) {
        if (network::instance_mutex)
            CloseHandle(network::instance_mutex);
        network::instance_mutex = nullptr;
        network::graphics_enabled = 0;
    }
    initialize_server_query();
    initialize_authentication();
    if (network::ui::console_available()) {
        network::packets::ScopedString line;
        network::ui::console_print(reinterpret_cast<char *>(server_settings));
        gui::string_format_ascii(&line.value, "IP : %s",
                                 reinterpret_cast<char *>(server_settings) + 24);
        network::ui::console_print(gui::string_ascii(&line.value));
        gui::string_format_ascii(&line.value, "Port : %d", at<Dword>(server_settings, 656));
        network::ui::console_print(gui::string_ascii(&line.value));
        gui::string_format_ascii(&line.value, "Mode : %s",
                                 reinterpret_cast<char *>(server_settings) + 532);
        network::ui::console_print(gui::string_ascii(&line.value));
        gui::string_format_ascii(&line.value, "Map : %s",
                                 reinterpret_cast<char *>(server_settings) + 324);
        network::ui::console_print(gui::string_ascii(&line.value));
    }
}
} // namespace nfs::services::pending
