#include "network_bandwidth_protocol.hpp"
#include <mmsystem.h>
#include <conio.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
namespace nfs::network::bandwidth {
namespace {
#include "network_bandwidth_graph.inc"
void close_probe_sockets() {
    if (datagram_socket != INVALID_SOCKET)
        closesocket(datagram_socket);
    if (echo_socket != INVALID_SOCKET)
        closesocket(echo_socket);
    WSACleanup();
}
void random_payload(unsigned char (&payload)[640]) {
    for (auto &byte : payload)
        byte = static_cast<unsigned char>(std::rand());
}
void describe_address(Dword address) {
    format_probe_address(&address);
}
void copy_endpoint(sockaddr_in &target, const sockaddr_in &source) {
    std::memcpy(&target, &source, sizeof(target));
}
int set_ttl(SOCKET socket, int ttl) {
    return setsockopt(socket, 0, 4, reinterpret_cast<const char *>(&ttl), 4);
}
} // namespace

Dword measure_rate(Dword remote, Dword local, std::uint16_t port, int retries, int *status,
                  Dword *upstream, Dword version, const Dword *options) {
    if (version != 257)
        return 11;
    timeBeginPeriod(1);
    probes_sent = probes_timed_out = delays_in_band = delays_received = 0;
    measurement_succeeded = 0;
    static constexpr Dword default_options = 993480;
    if (!options)
        options = &default_options;
    Dword measured = 0;
    const int attempts = static_cast<int>(static_cast<Dword>(retries) + 1);
    for (int attempt = 0; attempt < attempts; ++attempt) {
        measured = measure_route(remote, local, port, status, upstream, options);
        if (measured) {
            measurement_succeeded = 1;
            break;
        }
        if (*status > 0 && *status <= 4)
            break;
    }
    timeEndPeriod(1);
    return measured;
}

Dword measure_route(Dword remote, Dword local, std::uint16_t port, int *status, Dword *upstream,
                   const Dword *options) {
    Dword source = 0;
    timeGetTime();
    if (!port)
        port = 1230;
    std::srand(timeGetTime());
    unsigned char payload[640];
    random_payload(payload);
    WSADATA sockets;
    if (WSAStartup(258, &sockets)) {
        GetLastError();
        *status = 1;
        return 0;
    }
    datagram_socket = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    if (datagram_socket == INVALID_SOCKET) {
        WSAGetLastError();
        WSACleanup();
        *status = (WSAGetLastError() != 10013) + 2;
        return 0;
    }
    echo_socket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (echo_socket == INVALID_SOCKET) {
        WSAGetLastError();
        closesocket(datagram_socket);
        WSACleanup();
        *status = (WSAGetLastError() != 10013) + 2;
        return 0;
    }
    int timeout = 50;
    if (setsockopt(datagram_socket, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<char *>(&timeout), 4))
        WSAGetLastError();
    timeout = 1;
    if (setsockopt(echo_socket, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<char *>(&timeout), 4))
        WSAGetLastError();
    sockaddr_in bound;
    bound.sin_addr.s_addr = htonl(0);
    bound.sin_port = htons(port);
    bound.sin_family = AF_INET;
    if (bind(datagram_socket, reinterpret_cast<sockaddr *>(&bound), 16) == SOCKET_ERROR) {
        WSAGetLastError();
        *status = 4;
        close_probe_sockets();
        return 0;
    }
    sockaddr_in destination;
    destination.sin_addr.s_addr = htonl(remote);
    destination.sin_port = 80; // Host-order store, not htons(80).
    destination.sin_family = AF_INET;
    sockaddr_in burst_destination;
    copy_endpoint(burst_destination, destination);
    if (!local) {
        timeGetTime();
        send_echo(payload, 0, echo_socket, reinterpret_cast<sockaddr *>(&destination), 50);
        const Dword start = timeGetTime();
        int sequence = -1;
        for (;;) {
            receive_echo(echo_socket, &sequence, reinterpret_cast<sockaddr *>(&destination),
                         ntohl(burst_destination.sin_addr.s_addr), &source);
            if (timeGetTime() - start > 2500) {
                *status = 9;
                close_probe_sockets();
                return 0;
            }
            if (sequence != -1)
                break;
        }
        if (sequence != 50) {
            *status = 9;
            close_probe_sockets();
            return 0;
        }
        local = source;
        source = htonl(source);
        describe_address(source);
        timeGetTime();
    }
    timeGetTime();
    Dword local_network = htonl(local);
    sockaddr_in hop_destination;
    hop_destination.sin_addr.s_addr = htonl(remote);
    hop_destination.sin_port = 80;
    hop_destination.sin_family = AF_INET;
    sockaddr_in route_destination;
    copy_endpoint(route_destination, hop_destination);
    int ttl = 1, route_count = 0, sequence_to_send = 0;
    Dword route[256];
    timeGetTime();
    for (; ttl < 100; ++ttl, ++sequence_to_send) {
        copy_endpoint(hop_destination, route_destination);
        if (set_ttl(echo_socket, ttl) == SOCKET_ERROR) {
            WSAGetLastError();
            route_count = 0;
            break;
        }
        const Dword start = timeGetTime();
        unsigned char route_payload[240]; // Zero-length send; saved buffer is not initialized.
        send_echo(route_payload, 0, echo_socket, reinterpret_cast<sockaddr *>(&hop_destination),
                  sequence_to_send);
        int sequence = -1;
        for (;;) {
            receive_echo(echo_socket, &sequence, reinterpret_cast<sockaddr *>(&hop_destination),
                         ntohl(route_destination.sin_addr.s_addr), &local_network);
            if (timeGetTime() - start > 1500 || sequence != -1)
                break;
        }
        if (sequence != -1) {
            const Dword hop = ntohl(hop_destination.sin_addr.s_addr);
            route[route_count++] = hop;
            if (hop == remote || sequence == sequence_to_send)
                break;
        }
    }
    if (route_count <= 0 || route[route_count - 1] != remote)
        route_count = 0;
    timeGetTime();
    if (!route_count) {
        *status = 8;
        close_probe_sockets();
        return 0;
    }
    for (int index = 0; index < route_count; ++index)
        describe_address(htonl(route[index]));

    Dword delays[100];
    int selected_hop = route_count > 3 ? 2 : 0;
    int first_ttl = 0;
    sockaddr_in gateway;
    float baseline_delay;
    for (; selected_hop < route_count; ++selected_hop) {
        const Dword hop = route[selected_hop];
        if ((hop & 4294901760u) == (local & 4294901760u))
            continue;
        int count = sample_delays(hop, local, 1, 0, delays, 2000);
        if (!count)
            count = sample_delays(hop, local, 1, 0, delays, 2000);
        if (!count)
            continue;
        timeGetTime();
        first_ttl = selected_hop + 1;
        gateway.sin_addr.s_addr = htonl(hop);
        gateway.sin_port = 80;
        gateway.sin_family = AF_INET;
        int samples = 15;
        Dword wait = delays[0] * 3;
        if (delays[0] < 50) {
            samples = 50;
            wait = 100;
        } else if (delays[0] > 100)
            samples = 6;
        count = sample_delays(hop, local, samples, 0, delays, wait);
        if (!count) {
            *status = 6;
            close_probe_sockets();
            return 0;
        }
        baseline_delay = float(average_delay(count, delays, 1));
        timeGetTime();
        timeGetTime();
        break;
    }
    if (!first_ttl) {
        *status = 8;
        close_probe_sockets();
        return 0;
    }
    const Dword settings = *options;
    int burst_count = (settings >> 9) & 127;
    Dword burst_timeout = 8000;
    if (float((settings >> 16) & 31) > baseline_delay) {
        burst_count = (settings >> 2) & 127;
        burst_timeout = 4000;
    }
    const bool use_echo = (settings & 1) != 0;
    const SOCKET burst_socket = use_echo ? echo_socket : datagram_socket;
    ++first_ttl;
    if (set_ttl(burst_socket, first_ttl) == SOCKET_ERROR) {
        WSAGetLastError();
        *status = 5;
        close_probe_sockets();
        return 0;
    }
    int receive_buffer = 128000;
    if (setsockopt(burst_socket, SOL_SOCKET, SO_RCVBUF, reinterpret_cast<char *>(&receive_buffer),
                   4) == SOCKET_ERROR)
        WSAGetLastError();
    Dword burst_start = timeGetTime();
    const int maximum_ttl = route_count - 1 < first_ttl ? first_ttl : route_count - 1;
    int current_ttl = first_ttl;
    timeGetTime();
    for (int sent = 0; sent < burst_count; ++sent) {
        if (settings & 2) {
            if (++current_ttl > maximum_ttl)
                current_ttl = first_ttl;
            if (set_ttl(burst_socket, current_ttl) == SOCKET_ERROR) {
                WSAGetLastError();
                *status = 5;
            }
        }
        random_payload(payload);
        if (use_echo)
            send_echo(payload, 466, echo_socket, reinterpret_cast<sockaddr *>(&burst_destination),
                      0);
        else {
            unsigned char packet[474];
            *reinterpret_cast<std::uint16_t *>(packet) = 1234;
            *reinterpret_cast<std::uint16_t *>(packet + 2) = htons(4321);
            *reinterpret_cast<std::uint16_t *>(packet + 4) = 474;
            *reinterpret_cast<std::uint16_t *>(packet + 6) = 0;
            std::memcpy(packet + 8, payload, 466);
            Dword sum = 0;
            for (unsigned index = 0; index < 237; ++index)
                sum += reinterpret_cast<const std::uint16_t *>(packet)[index];
            sum = (sum >> 16) + (sum & 65535u);
            *reinterpret_cast<std::uint16_t *>(packet + 6) =
                static_cast<std::uint16_t>(~(sum + (sum >> 16)));
            burst_destination.sin_port = htons(4321);
            if (sendto(datagram_socket, reinterpret_cast<const char *>(packet), 474, 0,
                       reinterpret_cast<sockaddr *>(&burst_destination), 16) == SOCKET_ERROR)
                WSAGetLastError();
        }
    }
    timeGetTime();
    timeGetTime();
    if (set_ttl(echo_socket, 255) == SOCKET_ERROR)
        WSAGetLastError();
    const Dword marker_start = timeGetTime();
    send_echo(payload, 0, echo_socket, reinterpret_cast<sockaddr *>(&gateway), 5);
    send_echo(payload, 0, echo_socket, reinterpret_cast<sockaddr *>(&gateway), 6);
    int sequence = -1;
    while (sequence != 6) {
        receive_echo(echo_socket, &sequence, reinterpret_cast<sockaddr *>(&destination), 0,
                     &source);
        if (timeGetTime() - marker_start > burst_timeout || sequence == 5)
            break;
    }
    if (sequence != 5 && sequence != 6) {
        *status = 7;
        close_probe_sockets();
        return 0;
    }
    const Dword finished = timeGetTime();
    if (finished - marker_start == 4294967295u) {
        *status = 7;
        close_probe_sockets();
        return 0;
    }
    const auto format = kernel::numeric::current_format();
    const auto elapsed = Extended::integer(finished - burst_start, format) - baseline_delay;
    const Dword milliseconds = Dword(elapsed.truncate_to_integer());
    *status = 0;
    Dword downstream = milliseconds && elapsed < Extended(268435456.f, format)
                          ? (Dword(burst_count) * 4000000u) / milliseconds
                          : 4294967295u;
    timeGetTime();
    Dword upstream_value = downstream;
    timeGetTime();
    if (downstream < 576000 && downstream > 8) {
        int first_wait = int(Dword((Extended(baseline_delay, format) * 5.f).truncate_to_integer()));
        if (first_wait <= 200)
            first_wait = 200;
        sequence = -1;
        const Dword drain_start = timeGetTime();
        Dword last_packet = drain_start;
        for (;;) {
            receive_echo(echo_socket, &sequence, reinterpret_cast<sockaddr *>(&destination),
                         ntohl(burst_destination.sin_addr.s_addr), &source);
            if (sequence != -1) {
                sequence = -1;
                last_packet = timeGetTime();
            }
            if (timeGetTime() - drain_start > 3000 || timeGetTime() - last_packet > 500)
                break;
        }
        const float retained_delay = baseline_delay;
        if (set_ttl(echo_socket, 255) == SOCKET_ERROR) {
            WSAGetLastError();
            *status = 5;
            close_probe_sockets();
            return downstream;
        }
        const Dword hop = ntohl(gateway.sin_addr.s_addr);
        int count = sample_delays(hop, local, 1, 540, delays, Dword(first_wait));
        if (!count)
            count = sample_delays(hop, local, 1, 540, delays, 2000);
        if (!count)
            count = sample_delays(hop, local, 1, 540, delays, 2000);
        if (count) {
            int samples = 15;
            Dword wait = delays[0] * 3;
            if (delays[0] < 100) {
                samples = 50;
                wait = 200;
            } else if (delays[0] > 250)
                samples = 6;
            count = sample_delays(hop, local, samples, 540, delays, wait);
            if (!count) {
                *status = 6;
                close_probe_sockets();
                return downstream;
            }
            float minimum_delay = 1000000.f;
            for (int index = 0; index < samples; ++index) {
                const auto value = Extended::integer(delays[index], format);
                if (!(Extended(minimum_delay, format) < value))
                    minimum_delay = float(value);
            }
            const auto average = average_delay(count, delays, 1);
            if (!(downstream & 4294967288u))
                downstream = 56000;
            const auto corrected =
                average - Extended(540000.f, format) / Extended::integer(downstream >> 3, format) -
                retained_delay;
            if (Extended(0.f, format) < corrected)
                upstream_value =
                    Dword((Extended(1000.f, format) / corrected * 4320.f).truncate_to_integer());
            timeGetTime();
        }
    }
    if (settings & 2097152u) {
        unsigned char graph[2400];
        std::memcpy(graph, graph_template, sizeof(graph));
        if (set_ttl(echo_socket, 255) == SOCKET_ERROR)
            WSAGetLastError();
        float values[1024];
        int value_count = 0;
        for (int size = 0; size < 541; size += 10, ++value_count) {
            Dword samples_buffer[100];
            const Dword hop = ntohl(gateway.sin_addr.s_addr);
            int count = sample_delays(hop, local, 1, size, samples_buffer, 1000);
            if (!count)
                count = sample_delays(hop, local, 1, size, samples_buffer, 2000);
            if (!count)
                count = sample_delays(hop, local, 1, size, samples_buffer, 2000);
            if (!count)
                continue;
            int samples = 15;
            Dword wait = samples_buffer[0] * 3;
            if (samples_buffer[0] < 100) {
                samples = 30;
                wait = 200;
            } else if (samples_buffer[0] > 250)
                samples = 6;
            count = sample_delays(hop, local, samples, size, samples_buffer, wait);
            values[value_count] = count ? float(average_delay(count, samples_buffer, 1)) : 0.f;
        }
        float maximum = -1.f, minimum = 10000.f;
        for (int index = 0; index < value_count; ++index) {
            if (!(values[index] > minimum))
                minimum = values[index];
            if (!(maximum > values[index]))
                maximum = values[index];
        }
        char label[240];
        std::sprintf(label, "%3.1f", double(maximum));
        std::strncpy(reinterpret_cast<char *>(graph), label, std::strlen(label));
        std::sprintf(label, "%3.1f", double(minimum));
        std::strncpy(reinterpret_cast<char *>(graph + 2000), label, std::strlen(label));
        const float scale = float(Extended(25.f, format) / (Extended(maximum, format) - minimum));
        for (int index = 0; index < value_count; ++index) {
            const int row = int(
                Dword(((Extended(values[index], format) - minimum) * scale).truncate_to_integer()));
            graph[2009 + index - row * 80] = '*';
        }
        for (int row = 0; row < 30; ++row)
            _cprintf(reinterpret_cast<const char *>(graph + row * 80));
    }
    close_probe_sockets();
    *upstream = upstream_value < downstream ? downstream : upstream_value;
    timeGetTime();
    *status = 0;
    return downstream;
}
} // namespace nfs::network::bandwidth
