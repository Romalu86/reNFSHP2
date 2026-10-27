#include "nfs/network/packets/network_authentication.hpp"
#include <cstring>
#include <cstdio>
namespace nfs::network::authentication {
char client_key[20]{};
// Preserve admissible dash positions and forbidden 0/1/O/I.
bool valid_key(const char *key) {
    if (std::strlen(key) != 19)
        return false;
    char upper[100]{};
    std::strcpy(upper, key);
    _strupr(upper);
    for (unsigned index = 0; index < std::strlen(key); ++index) {
        const unsigned char value = upper[index];
        const bool alphanumeric = (value >= 'A' && value <= 'Z') || (value >= '0' && value <= '9');
        const bool forbidden = value == '0' || value == '1' || value == 'O' || value == 'I';
        const bool dash = value == '-' && (index == 4 || index == 9 || index == 14);
        if (forbidden || (!alphanumeric && !dash))
            return false;
    }
    return true;
}

void set_client_key(const char *key) {
    const auto length = std::strlen(key);
    if (length == 19) {
        std::strcpy(client_key, key);
        return;
    }
    if (length == 16) {
        char parts[4][5]{};
        for (unsigned index = 0; index < 4; ++index)
            std::strncpy(parts[index], key + index * 4, 4);
        std::sprintf(client_key, "%s-%s-%s-%s", parts[0], parts[1], parts[2], parts[3]);
    }
}

void set_error(Dword *record, const char *message) {
    std::strcpy(reinterpret_cast<char *>(record) + 17, message);
}

void set_key(Dword *record, const char *key) {
    std::strcpy(reinterpret_cast<char *>(record) + 325, key);
}

} // namespace nfs::network::authentication
