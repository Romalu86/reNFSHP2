#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "server_runtime.hpp"
#include "nfs/graphics/configuration/render_settings.hpp"
#include "nfs/frontend/network/frontend_network.hpp"
#include "nfs/core/errors/assertions.hpp"
#include <cstdio>
#include <cstring>
namespace nfs::services::pending {
Byte (&server_settings)[852] =
    *reinterpret_cast<Byte (*)[852]>(network::frontend::state + 8);
}
namespace nfs::application::pending {
Dword server_password_enabled{};
// Unwritten settings bytes retain their existing contents.
void *construct_server_settings(void *settings) {
    std::strcpy(reinterpret_cast<char *>(settings), assertions::empty_text);
    std::strcpy(reinterpret_cast<char *>(bytes(settings) + 24), assertions::empty_text);
    std::strcpy(reinterpret_cast<char *>(bytes(settings) + 324), assertions::empty_text);
    std::strcpy(reinterpret_cast<char *>(bytes(settings) + 532), assertions::empty_text);
    std::strcpy(reinterpret_cast<char *>(bytes(settings) + 632), assertions::empty_text);
    std::strcpy(reinterpret_cast<char *>(bytes(settings) + 704), assertions::empty_text);
    at<Dword>(settings, 656) = 0;
    at<Dword>(settings, 660) = 0;
    at<Dword>(settings, 664) = 0;
    at<Dword>(settings, 672) = 0;
    at<Dword>(settings, 676) = 0;
    at<Dword>(settings, 524) = 0;
    at<Dword>(settings, 528) = 0;
    at<Byte>(settings, 848) = 0;
    at<Byte>(settings, 849) = 0;
    at<Dword>(settings, 668) = 8;
    at<Dword>(settings, 832) = 0;
    at<Dword>(settings, 836) = 0;
    at<Dword>(settings, 840) = 0;
    at<Dword>(settings, 844) = 0;
    at<Dword>(settings, 680) = 0;
    at<Byte>(settings, 692) = 0;
    at<Byte>(settings, 693) = 0;
    at<Byte>(settings, 694) = 0;
    at<Dword>(settings, 696) = 0;
    at<Dword>(settings, 700) = 0;
    at<Dword>(settings, 688) = 6;
    return settings;
}

void initialize_server_settings() {
    construct_server_settings(services::pending::server_settings);
}

int next_server_track(void *settings) {
    const Byte count = at<Byte>(settings, 848);
    if (!count)
        return -1;
    const Byte current = at<Byte>(settings, 849);
    const Byte track = at<Byte>(settings, 832 + current);
    const Byte next = Byte(current + 1);
    at<Byte>(settings, 849) = next;
    if (count <= next)
        at<Byte>(settings, 849) = 0;
    return track;
}

void set_server_direction(void *settings, Dword value) {
    at<Dword>(settings, 684) = value;
}

void set_server_car_class(void *settings, int value) {
    if (value != 6 && (value < 38 || value > 42))
        value = 6;
    at<int>(settings, 688) = value;
}

void save_server_settings(void *settings) {
    char filename[260]{};
    std::sprintf(filename, "%s%s", graphics::startup::configuration_path(), "server.ini");
    ini::Reader reader;
    ini::reader_construct(&reader, filename, 0);
    try {
        services::dependency::set_ini_text(&reader, "Settings", "hostName",
                                           reinterpret_cast<char *>(settings));
        ini::set_int(&reader, "Settings", "trackId", at<int>(settings, 524));
        services::dependency::set_ini_text(&reader, "Settings", "password",
                                           reinterpret_cast<char *>(bytes(settings) + 632));
        ini::set_int(&reader, "Settings", "laps", at<int>(settings, 672));
        ini::set_int(&reader, "Settings", "gameType", at<int>(settings, 680));
        ini::set_int(&reader, "Settings", "direction", at<int>(settings, 684));
        ini::set_int(&reader, "Settings", "classRestriction", at<int>(settings, 688));
        ini::set_int(&reader, "Settings", "isDedicated", at<Byte>(settings, 692));
        services::dependency::set_ini_text(&reader, "Settings", "messageOfTheDay",
                                           reinterpret_cast<char *>(bytes(settings) + 704));
    } catch (...) {
        ini::reader_clear(&reader);
        throw;
    }
    ini::reader_clear(&reader);
}
} // namespace nfs::application::pending
NFS_KERNEL_INITIALIZER(startup_initialize_server_settings, ".CRT$XCU_NFS_006902A0",
                       nfs::application::pending::initialize_server_settings);
