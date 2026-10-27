#include "../session/network_bandwidth_protocol.hpp"
#include <cstdio>
#include <cstring>
namespace nfs::network::bandwidth {
SOCKET datagram_socket = INVALID_SOCKET, echo_socket = INVALID_SOCKET;
Dword probes_sent{}, probes_timed_out{}, delays_in_band{}, delays_received{};
unsigned char measurement_succeeded{};
Dword probe_sequence{};
char address_text[128]{}, endpoint_text[128]{};

bool send_echo(const void *payload, int payload_size, SOCKET socket, const sockaddr *destination,
               Dword sequence) {
    if (!destination || payload_size > 550)
        return false;
    unsigned char packet[1024];
    packet[0] = 8;
    packet[1] = 0;
    *reinterpret_cast<std::uint16_t *>(packet + 2) = 0;
    *reinterpret_cast<std::uint16_t *>(packet + 4) =
        static_cast<std::uint16_t>(GetCurrentProcessId());
    *reinterpret_cast<std::uint16_t *>(packet + 6) = static_cast<std::uint16_t>(sequence);
    std::memcpy(packet + 8, payload, static_cast<Dword>(payload_size));
    const int size = static_cast<int>(static_cast<Dword>(payload_size) + 8u);
    Dword sum = 0;
    int remaining = size;
    auto *cursor = packet;
    while (remaining > 1) {
        sum += *reinterpret_cast<const std::uint16_t *>(cursor);
        cursor += 2;
        remaining -= 2;
    }
    if (remaining)
        sum += *cursor;
    sum = (sum >> 16) + (sum & 65535u);
    *reinterpret_cast<std::uint16_t *>(packet + 2) =
        static_cast<std::uint16_t>(~(sum + (sum >> 16)));
    const int sent =
        sendto(socket, reinterpret_cast<const char *>(packet), size, 0, destination, 16);
    if (sent == SOCKET_ERROR) {
        WSAGetLastError();
        return false;
    }
    return sent == size;
}

bool receive_echo(SOCKET socket, int *sequence, sockaddr *sender, Dword expected_destination,
                  Dword *received_destination) {
    u_long available;
    if (ioctlsocket(socket, FIONREAD, &available) || !available) {
        Sleep(0);
        return false;
    }
    sockaddr received_from;
    int address_size = 16;
    unsigned char packet[1024];
    const int received =
        recvfrom(socket, reinterpret_cast<char *>(packet), 1024, 0, &received_from, &address_size);
    if (received == SOCKET_ERROR) {
        WSAGetLastError();
        return false;
    }
    if (static_cast<Dword>(received) < 28)
        return false;
    const unsigned header_size = (packet[0] & 15u) * 4u;
    if (received < static_cast<int>(header_size + 8) || packet[9] != 1)
        return false;
    *received_destination = ntohl(*reinterpret_cast<const Dword *>(packet + 16));
    const auto *echo = packet + header_size;
    if (*echo == 0) {
        if (*reinterpret_cast<const std::uint16_t *>(echo + 4) !=
            static_cast<std::uint16_t>(GetCurrentProcessId()))
            return false;
        std::memcpy(sender, &received_from, static_cast<Dword>(address_size));
        *sequence = *reinterpret_cast<const std::uint16_t *>(echo + 6);
        return true;
    }
    if (*echo != 11 || ntohl(*reinterpret_cast<const Dword *>(echo + 24)) != expected_destination)
        return false;
    std::sprintf(address_text, "%d.%d.%d.%d", packet[12], packet[13], packet[14], packet[15]);
    std::memcpy(sender, &received_from, static_cast<Dword>(address_size));
    *sequence = 100000;
    return true;
}

const char *format_probe_address(const Dword *address) {
    const auto *octets = reinterpret_cast<const unsigned char *>(address);
    std::sprintf(address_text, "%d.%d.%d.%d", octets[0], octets[1], octets[2], octets[3]);
    return address_text;
}
} // namespace nfs::network::bandwidth
