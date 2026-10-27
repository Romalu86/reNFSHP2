#include "network_bandwidth_protocol.hpp"
#include <mmsystem.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <malloc.h>
namespace nfs::network::bandwidth {

int sample_delays(Dword remote, Dword local, int count, int payload_size, Dword *delays,
                  Dword timeout) {
    Dword received_count = 0;
    Dword source = htonl(local);
    int ttl = 255;
    if (setsockopt(echo_socket, 0, 4, reinterpret_cast<const char *>(&ttl), 4) == SOCKET_ERROR) {
        WSAGetLastError();
        return 0;
    }
    sockaddr_in destination;
    destination.sin_family = AF_INET;
    destination.sin_port = 80;
    destination.sin_addr.s_addr = htonl(remote);
    timeGetTime();
    for (int attempt = 0; attempt < count; ++attempt) {
        unsigned char payload[640];
        for (auto &byte : payload)
            byte = static_cast<unsigned char>(std::rand());
        const Dword start = timeGetTime();
        send_echo(payload, payload_size, echo_socket, reinterpret_cast<sockaddr *>(&destination),
                  probe_sequence);
        ++probes_sent;
        int sequence = -1;
        if (probe_sequence != 4294967295u) {
            for (;;) {
                sockaddr sender;
                receive_echo(echo_socket, &sequence, &sender, ntohl(destination.sin_addr.s_addr),
                             &source);
                if (timeGetTime() - start > timeout) {
                    ++probes_timed_out;
                    break;
                }
                if (static_cast<Dword>(sequence) == probe_sequence)
                    break;
            }
        }
        if (static_cast<Dword>(sequence) == probe_sequence) {
            const Dword elapsed = timeGetTime() - start;
            const auto *octets =
                reinterpret_cast<const unsigned char *>(&destination.sin_addr.s_addr);
            std::sprintf(endpoint_text, "%d.%d.%d.%d ; %d", octets[0], octets[1], octets[2],
                         octets[3], static_cast<unsigned>(destination.sin_port));
            delays[received_count++] = elapsed;
        }
        ++probe_sequence;
    }
    if (received_count > 5) {
        delays_received += received_count;

        auto *copy = static_cast<Dword *>(_alloca(received_count * sizeof(Dword)));
        std::memcpy(copy, delays, received_count * sizeof(Dword));
        const float mean = float(average_delay(static_cast<int>(received_count), copy, 1));
        float factor = .25f;
        if (mean < 100.f)
            factor = 0.3499999940395355224609375f;
        if (mean < 30.f)
            factor = .5f;
        const auto format = kernel::numeric::current_format();
        const auto threshold = Extended(factor, format) * mean;
        for (Dword index = 0; index < received_count; ++index) {
            auto difference = Extended::integer(delays[index], format) - mean;
            if (difference < Extended(0.f, format))
                difference = -difference;
            if (difference < threshold)
                ++delays_in_band; // Counter counts samples INSIDE the band.
        }
    }
    return static_cast<int>(received_count);
}
} // namespace nfs::network::bandwidth
