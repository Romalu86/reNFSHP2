#include "network_lifecycle_runtime.hpp"
#include <cstdio>
namespace nfs::network::session {
char service_key[8]{};
char version_text[36]{};

void load_registry_key() {
    HKEY key;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                      "Software\\Electronic Arts\\EA Games\\Need For Speed Hot Pursuit 2\\ergc", 0,
                      KEY_READ, &key) &&
        RegOpenKeyExA(
            HKEY_LOCAL_MACHINE,
            "Software\\Electronic Arts\\EA Games\\Need For Speed poursuite infernale 2\\ergc", 0,
            KEY_READ, &key))
        return;
    Byte value[500]{};
    DWORD type = 1, size = 500;
    if (!RegQueryValueExA(key, "", nullptr, &type, value, &size))
        authentication::set_client_key(reinterpret_cast<char *>(value));
    // No RegCloseKey appears on this path.
}
} // namespace nfs::network::session
namespace nfs::services::pending {

void initialize_network_identity() {
    auto *key = network::session::service_key;
    key[0] = 'Z';
    key[1] = 'I';
    key[2] = 'r';
    key[3] = '1';
    key[4] = 'w';
    key[5] = 'X';
    key[6] = 0;
    network::session::load_registry_key();
    char text[200]{};
    std::sprintf(text, "%d", 242);
    std::strcpy(network::session::version_text, text);
}
} // namespace nfs::services::pending
