#include "nfs/network/session/network_runtime.hpp"
#include <cstring>
namespace nfs::network {

void reset_server(void *server) {
    std::memset(static_cast<char *>(server) + 56, 0, 164);
    std::strncpy(static_cast<char *>(server) + 172, "Unnamed NFS-HP2 Server", 24);
    field<Dword>(server, 232) = 0;
    field<Dword>(server, 88) = 0;
    field<Dword>(server, 68) = 0;
    field<Dword>(server, 80) = 0;
    field<Dword>(server, 72) = 1;
    field<Dword>(server, 76) = 3;
    field<Dword>(server, 64) = 0;
    field<Dword>(server, 116) = 0;
    field<Dword>(server, 120) = 0;
    field<Dword>(server, 100) = 0;
    field<Dword>(server, 84) = 0;
    field<Dword>(server, 60) = 6;
    field<Dword>(server, 92) = 1000000;
    field<std::uint8_t>(server, 96) = 8;
    field<Dword>(server, 104) = 20000;
    field<Dword>(server, 108) = 1;
    field<Dword>(server, 136) = 0;
    field<Dword>(server, 164) = 30000;
    field<Dword>(server, 160) = 0;
    field<Dword>(server, 128) = 900000;
    field<Dword>(server, 132) = 90000;
    field<Dword>(server, 168) = 1;
    field<Dword>(server, 112) = 15000;
    field<Dword>(server, 124) = 0;
    field<Dword>(server, 140) = 1;
    field<Dword>(server, 48) = 0;
    field<Dword>(server, 28) = 0;
    field<Dword>(server, 32) = 0;
    field<Dword>(server, 36) = 0;
    field<Dword>(server, 220) = 1;
    field<std::uint16_t>(server, 224) = 1;
    field<std::uint16_t>(server, 52) = 8;
    field<Dword>(server, 24) = 0;
    field<Dword>(server, 56) = 0;
}
} // namespace nfs::network
