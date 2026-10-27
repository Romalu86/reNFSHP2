#include "nfs/network/packets/network_packet_handlers.hpp"
#include "nfs/graphics/configuration/render_settings.hpp"
#include <cstdio>
#include <cstring>
#include <ctime>
namespace nfs::network::packets::pending {
Dword *authentication_records[20]{};
// There is no index check in this entry.
Dword *authentication_at(int index) {
    return authentication_records[index];
}

Dword *authentication_record(int identifier) {
    for (auto *record : authentication_records)
        if (record && static_cast<int>(record[0]) == identifier)
            return record;
    return nullptr;
}
namespace {
bool contains_key(const char *filename, const char *key) {
    char path[260]{};
    std::sprintf(path, "%s%s", graphics::startup::configuration_path(), filename);
    FILE *input = std::fopen(path, "r");
    if (!input)
        return false;
    char line[1024]{};
    bool found = false;
    while (std::fgets(line, 1024, input))
        if (std::strlen(line) > 31 && !std::strncmp(line, key, 32)) {
            found = true;
            break;
        }
    std::fclose(input);
    return found;
}
} // namespace

bool is_banned(const char *key) {
    return contains_key("banlist.txt", key);
}

void record_ban(const char *key, const char *name, const char *address) {
    char path[260]{};
    std::sprintf(path, "%s%s", graphics::startup::configuration_path(), "banlist.txt");
    if (FILE *output = std::fopen(path, "at")) {
        __time32_t timestamp;
        _time32(&timestamp);
        const char *date = std::asctime(_localtime32(&timestamp));
        std::fprintf(output, "%s : %s @ %s @ %s", key, name, address, date);
        std::fclose(output);
    }
}

bool is_booted(const char *key) {
    return contains_key("bootlist.txt", key);
}
// Legacy 32-bit time and the exact saved append format.
void record_boot(const char *key, const char *name, const char *address) {
    char path[260]{};
    std::sprintf(path, "%s%s", graphics::startup::configuration_path(), "bootlist.txt");
    if (FILE *output = std::fopen(path, "at")) {
        __time32_t timestamp;
        _time32(&timestamp);
        const char *date = std::asctime(_localtime32(&timestamp));
        std::fprintf(output, "%s : %s @ %s @ %s", key, name, address, date);
        std::fclose(output);
    }
}
// Key admission checks are retained without a bypass.
bool validate_key(const char *key, Dword *reason, char *description) {
    for (int index = 0; index < 20; ++index) {
        auto *record = authentication_at(index);
        if (record && !std::strncmp(reinterpret_cast<char *>(record) + 325, key, 32)) {
            *reason = 18;
            if (description)
                std::strcpy(description, "InUse");
            return false;
        }
    }
    if (is_booted(key)) {
        *reason = 5;
        if (description)
            std::strcpy(description, "Booted");
        return false;
    }
    if (is_banned(key)) {
        *reason = 6;
        if (description)
            std::strcpy(description, "Banned");
        return false;
    }
    return true;
}
} // namespace nfs::network::packets::pending
