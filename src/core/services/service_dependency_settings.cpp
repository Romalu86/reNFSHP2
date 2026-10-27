#include "nfs/core/services/service_dependencies.hpp"
#include "nfs/resources/ini/ini.hpp"
#include "nfs/resources/files/file_cache.hpp"
#include "nfs/graphics/configuration/render_settings.hpp"
#include "nfs/network/packets/network_packet_handlers.hpp"
#include <cstdio>
#include <cstring>
namespace nfs::services::dependency {
Byte preference_flags[5]{}, frontend_diagnostics{};
char (&preferred_player_name)[80] = network::packets::local_player_name;
char recent_addresses[5][16]{};
int maximum_concurrent_updates =
    10;
std::uint16_t internet_query_port = 61220, lan_query_port = 61220, lan_query_base = 61220,
              bandwidth_port = 1230, local_tcp_port = 0;

const char *preferred_name() {
    return network::packets::get_local_name();
}
const char *recent_address(int index) {
    return recent_addresses[index];
}
// Shift forward, replicating a nonempty predecessor.
void set_recent_address(int index, const char *address) {
    for (int i = 0; i < 5; ++i)
        if (!std::strcmp(recent_addresses[i], address)) {
            std::memset(recent_addresses[i], 0, 16);
            break;
        }
    if (index < 4)
        for (int i = index; i < 4 && *recent_addresses[i]; ++i)
            std::strncpy(recent_addresses[i + 1], recent_addresses[i], 16);
    std::strncpy(recent_addresses[index], address, 16);
    recent_addresses[index][15] = 0;
}

Dword set_ini_text(ini::Reader *reader, const char *section_name, const char *key_name,
                  const char *value) {
    if (!reader->document || ini::read_only(&reader->document->document))
        return 0;
    ini::Key *key = ini::reader_key(reader, section_name, key_name);
    if (!key) {
        ini::Section *section = ini::reader_section(reader, section_name);
        if (!section)
            section = ini::add_section(&reader->document->document, section_name, 0);
        key = ini::set(section, key_name, nullptr, 1);
        if (!key)
            return 0;
    }
    gui::string_assign(&key->value, value);
    reader->modified = 1;
    return 1;
}

void set_maximum_concurrent_updates(int count) {
    maximum_concurrent_updates = count < 1 ? 1 : count > 100 ? 100 : count;
}

void set_internet_query_port(std::uint16_t port) {
    internet_query_port = port;
}

void set_lan_query_port(std::uint16_t port) {
    lan_query_port = port;
}

void set_local_tcp_port(std::uint16_t port) {
    local_tcp_port = port;
}

void set_local_udp_port(std::uint16_t port) {
    network::packets::pending::reconnect_port = port;
}

void load_network_settings() {
    char filename[260]{};
    std::sprintf(filename, "%s%s", graphics::startup::configuration_path(), "netprefs.ini");
    ini::Reader reader;
    ini::reader_construct(&reader, filename, 0);
    try {
        if (files::file::exists(filename)) {
            int value = 0;
            char text[500]{};
            short port = 0;
            if (ini::get_int(&reader, "Settings", "guiMode", &value))
                services::pending::network_kind = static_cast<Dword>(value);
            const char *keys[] = {"showFps", "showDiagnostics", "showPlayerData", "showCarData",
                                  "queryAnotherGame"};
            for (unsigned i = 0; i < 5; ++i)
                if (ini::get_int(&reader, "Settings", keys[i], &value))
                    preference_flags[i] = value == 1;
            if (ini::get_int(&reader, "Settings", "showFEDiags", &value))
                frontend_diagnostics = value == 1;
            if (ini::get_int(&reader, "Settings", "maxConcUpdates", &value))
                set_maximum_concurrent_updates(value);
            if (ini::get_short(&reader, "Settings", "tcpPort", &port) && port && network::transport)
                field<std::uint16_t>(network::transport, 36) = static_cast<std::uint16_t>(port);
            if (ini::get_short(&reader, "Settings", "udpPort", &port) && port && network::transport)
                field<std::uint16_t>(network::transport, 38) = static_cast<std::uint16_t>(port);
            if (ini::get_short(&reader, "Settings", "localTCPPort", &port) && port &&
                network::transport)
                set_local_tcp_port(static_cast<std::uint16_t>(port));
            if (ini::get_short(&reader, "Settings", "localUDPPort", &port) && port &&
                network::transport)
                set_local_udp_port(static_cast<std::uint16_t>(port));
            if (ini::get_short(&reader, "Settings", "baseQueryPortInternet", &port) && port)
                set_internet_query_port(static_cast<std::uint16_t>(port));
            if (ini::get_short(&reader, "Settings", "baseQueryPortLan", &port) && port &&
                static_cast<std::uint16_t>(port) > lan_query_base &&
                static_cast<Dword>(static_cast<std::uint16_t>(port)) < Dword(lan_query_base) + 10)
                set_lan_query_port(static_cast<std::uint16_t>(port));
            if (ini::get_short(&reader, "Settings", "bandwidthPort", &port) && port)
                bandwidth_port = static_cast<std::uint16_t>(port);
            if (ini::get_string(&reader, "Settings", "playerName", text, 500)) {
                std::strncpy(preferred_player_name, text, 80);
                preferred_player_name[79] = 0;
            }
            const char *addresses[] = {"lastUsedIP", "lastUsedIP1", "lastUsedIP2", "lastUsedIP3",
                                       "lastUsedIP4"};
            char address[128]{};
            for (int i = 0; i < 5; ++i)
                if (ini::get_string(&reader, "Settings", addresses[i], address, 128))
                    set_recent_address(i, address);
            const char *pings[] = {"bandwidthPingIp0", "bandwidthPingIp1", "bandwidthPingIp2"};
            char ping[500]{};
            for (int i = 0; i < 3; ++i)
                if (ini::get_string(&reader, "Settings", pings[i], ping, 500))
                    pending::set_bandwidth_probe_address(i, ping);
        }
    } catch (...) {
        ini::reader_clear(&reader);
        throw;
    }
    ini::reader_clear(&reader);
}

void release_network_settings() {
    char filename[260]{};
    std::sprintf(filename, "%s%s", graphics::startup::configuration_path(), "netprefs.ini");
    ini::Reader reader;
    ini::reader_construct(&reader, filename, 0);
    try {
        ini::set_int(&reader, "Settings", "guiMode",
                     static_cast<int>(services::pending::network_kind));
        const char *keys[] = {"showFps", "showDiagnostics", "showPlayerData", "showCarData",
                              "queryAnotherGame"};
        for (unsigned i = 0; i < 5; ++i)
            ini::set_int(&reader, "Settings", keys[i], preference_flags[i]);
        ini::set_int(&reader, "Settings", "showFEDiags", frontend_diagnostics);
        ini::set_int(&reader, "Settings", "maxConcUpdates", maximum_concurrent_updates);
        if (network::transport) {
            ini::set_int(&reader, "Settings", "tcpPort",
                         field<std::uint16_t>(network::transport, 36));
            ini::set_int(&reader, "Settings", "udpPort",
                         field<std::uint16_t>(network::transport, 38));
        }
        if (local_tcp_port)
            ini::set_int(&reader, "Settings", "localTCPPort", local_tcp_port);
        if (network::packets::pending::reconnect_port)
            ini::set_int(&reader, "Settings", "localUDPPort",
                         network::packets::pending::reconnect_port);
        ini::set_int(&reader, "Settings", "baseQueryPortInternet", internet_query_port);
        ini::set_int(&reader, "Settings", "baseQueryPortLan", lan_query_port);
        ini::set_int(&reader, "Settings", "bandwidthPort", bandwidth_port);
        auto set_string = [&](const char *name, const char *value) {
            set_ini_text(&reader, "Settings", name, value);
        };
        set_string("playerName", preferred_player_name);
        const char *addresses[] = {"lastUsedIP", "lastUsedIP1", "lastUsedIP2", "lastUsedIP3",
                                   "lastUsedIP4"};
        for (int i = 0; i < 5; ++i)
            set_string(addresses[i], recent_address(i));
        const char *pings[] = {"bandwidthPingIp0", "bandwidthPingIp1", "bandwidthPingIp2"};
        for (int i = 0; i < 3; ++i)
            set_string(pings[i], pending::bandwidth_probe_address(i));
    } catch (...) {
        ini::reader_clear(&reader);
        throw;
    }
    ini::reader_clear(&reader);
}

void load_server_settings(void *settings) {
    char filename[260]{};
    std::sprintf(filename, "%s%s", graphics::startup::configuration_path(), "server.ini");
    ini::Reader reader;
    ini::reader_construct(&reader, filename, 0);
    try {
        if (files::file::exists(filename)) {
            char text[256]{};
            int value = 0;
            const auto string_field = [&](const char *name, unsigned offset) {
                if (ini::get_string(&reader, "Settings", name, text, 256))
                    std::strcpy(static_cast<char *>(settings) + offset, text);
            };
            const auto integer_field = [&](const char *name, unsigned offset) {
                if (ini::get_int(&reader, "Settings", name, &value))
                    field<int>(settings, offset) = value;
            };
            string_field("hostName", 0);
            integer_field("trackId", 524);
            string_field("password", 632);
            integer_field("laps", 672);
            integer_field("gameType", 680);
            integer_field("direction", 684);
            if (ini::get_int(&reader, "Settings", "classRestriction", &value))
                field<int>(settings, 688) =
                    value == 6 || (value >= 38 && value <= 42) ? value : 6;
            if (ini::get_int(&reader, "Settings", "isDedicated", &value))
                field<Byte>(settings, 692) = value == 1;
            string_field("messageOfTheDay", 704);
            for (unsigned track = 1; track <= 16; ++track) {
                char name[12];
                std::sprintf(name, "Track%u", track);
                if (ini::get_int(&reader, "Settings", name, &value)) {
                    const Byte count = field<Byte>(settings, 848);
                    field<Byte>(settings, 832 + count) =
                        static_cast<Byte>(value > 11 ? 12 : value);
                    field<Byte>(settings, 848) = Byte(count + 1);
                }
            }
            field<Byte>(settings, 693) =
                *reinterpret_cast<const char *>(static_cast<const Byte *>(settings) + 632) != 0;
        }
    } catch (...) {
        ini::reader_clear(&reader);
        throw;
    }
    ini::reader_clear(&reader);
}
} // namespace nfs::services::dependency
namespace nfs::services::pending {
void load_network_settings() {
    dependency::load_network_settings();
}
void release_network_settings() {
    dependency::release_network_settings();
}
void load_server_settings(void *settings) {
    dependency::load_server_settings(settings);
}
} // namespace nfs::services::pending
