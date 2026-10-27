#include <winsock2.h>
#include "network_lifecycle_runtime.hpp"
namespace nfs::network::session {
namespace {
Dword *route_connection_comparer(Dword *owner) {
    return owner ? reinterpret_cast<Dword *>(bytes(owner) + reinterpret_cast<Dword *>(owner[1])[1] +
                                            72)
                 : nullptr;
}
void dispatch_route(void *route, Byte *packet, Dword size) {
    if (!at<Dword>(route, 4))
        send_route(transport, route, packet, size, 0);
    else if (at<Dword>(route, 8))
        dispatch_server_packet(server(), nullptr, reinterpret_cast<Dword>(route),
                               reinterpret_cast<Dword>(packet), size);
    else
        dispatch_client_packet(client(), nullptr, reinterpret_cast<Dword>(route),
                               reinterpret_cast<Dword>(packet), size);
}
} // namespace

void __fastcall receive_transport_packet(void *, void *, Dword *connection, Byte *packet,
                                         Dword size) {
    Dword *server_owner = server();
    if (size > 65536)
        return;
    alignas(4) Byte route[304];
    if (!at<Dword>(server_owner, 44)) {
        Dword *owner = client();
        if (at<Dword *>(owner, 108) != connection && at<Dword *>(owner, 112) != connection) {
            pending::transport_statistics(0, size, protocol(connection) == 2 ? 4 : 0);
            return;
        }
        const Dword now = transport_time();
        const Dword client_route = at<volatile Dword>(owner, 80);
        at<volatile Dword>(owner, 92) = now;
        at<volatile Byte>(owner, 104) = packet[0];
        if (client_route)
            dispatch_client_packet(client(), nullptr, reinterpret_cast<Dword>(bytes(owner) + 72),
                                   reinterpret_cast<Dword>(packet), size);
        else
            dispatch_server_packet(server(), nullptr, reinterpret_cast<Dword>(bytes(owner) + 72),
                                   reinterpret_cast<Dword>(packet), size);
        return;
    }
    if (packet[0] != 27) {
        if (queue::copy_matching(server_queue(server_owner), reinterpret_cast<Dword>(connection),
                                 reinterpret_cast<Dword>(route),
                                 route_connection_comparer(server_owner))) {
            at<Byte>(route, 24) = packet[0];
            if (at<Dword>(route, 8))
                dispatch_client_packet(client(), nullptr, reinterpret_cast<Dword>(route),
                                       reinterpret_cast<Dword>(packet), size);
            else
                dispatch_server_packet(server(), nullptr, reinterpret_cast<Dword>(route),
                                       reinterpret_cast<Dword>(packet), size);
            at<Dword>(route, 20) = transport_time();
            // The dispatch may replace the global server; this route still belongs to its owner.
            queue::replace(server_queue(server_owner), reinterpret_cast<Dword>(route),
                           reinterpret_cast<Dword>(route), nullptr);
        } else
            pending::transport_statistics(0, size, protocol(connection) == 2 ? 4 : 0);
        return;
    }
    if (!queue::copy_matching(server_queue(server_owner), reinterpret_cast<Dword>(connection), 0,
                              route_connection_comparer(server_owner)) &&
        queue::copy_matching(server_queue(server_owner), reinterpret_cast<Dword>(packet + 2),
                             reinterpret_cast<Dword>(route), nullptr)) {
        at<Dword>(route, 20) = transport_time();
        if (protocol(connection) == 1)
            at<Dword *>(route, 36) = connection;
        else {
            at<Dword *>(route, 40) = connection;
            std::memcpy(route + 44, bytes(connection) + 112, 256);
            at<std::uint16_t>(route, 300) =
                ntohs(at<std::uint16_t>(address(connection), 2));
        }
        queue::replace(server_queue(server_owner), reinterpret_cast<Dword>(route), reinterpret_cast<Dword>(route),
                       nullptr);
    }
}

void __fastcall connection_opened(void *, void *, Dword *connection) {
    if (at<Dword>(server(), 44))
        return;
    Dword *owner = client();
    if (protocol(connection) == 1) {
        at<Dword>(owner, 84) = 1;
        at<Dword *>(owner, 108) = connection;
        at<Dword>(owner, 76) = 0;
        at<Dword>(owner, 92) = transport_time();
    }
    if (protocol(connection) == 2) {
        at<Dword>(owner, 84) = 1;
        at<Dword *>(owner, 112) = connection;
        at<Dword>(owner, 76) = 0;
        at<Dword>(owner, 92) = transport_time();
        alignas(4) Byte packet[6];
        packet[0] = 27;
        at<std::uint16_t>(packet, 2) = at<std::uint16_t>(owner, 72);
        send_route(transport, bytes(owner) + 72, packet, 6, 4);
    }
}

void __fastcall connection_accepted(void *, void *, Dword *connection) {
    alignas(4) Byte route[304]{};
    if (queue::copy_matching(route_queue(), reinterpret_cast<Dword>(connection), 0,
                             route_connection_comparer(server())))
        return;
    if (protocol(connection) != 1)
        return;
    const Dword identifier = at<Dword>(server(), 220)++;
    at<std::uint16_t>(route, 0) = std::uint16_t(identifier);
    at<Dword>(route, 12) = 1;
    at<Dword *>(route, 36) = connection;
    at<Dword>(route, 4) = 0;
    at<Dword>(route, 16) = 0;
    at<Dword>(route, 20) = transport_time();
    queue::append(route_queue(), reinterpret_cast<Dword>(route), reinterpret_cast<Dword>("listnode"));
    alignas(4) Byte packet[6];
    packet[0] = 8;
    at<std::uint16_t>(packet, 2) = std::uint16_t(identifier);
    at<std::uint16_t>(packet, 4) = at<std::uint16_t>(transport, 38);
    dispatch_route(route, packet, sizeof(packet));
}

void __fastcall connection_closed(void *, void *, Dword *connection) {
    if (!connection)
        return;
    if (!at<Dword>(server(), 44)) {
        Dword *owner = client();
        if (protocol(connection) == 1)
            at<Dword>(owner, 108) = 0;
        else
            at<Dword>(owner, 112) = 0;
        if (!at<Dword>(owner, 108)) {
            at<Dword>(owner, 112) = 0;
            client_route_closed(client());
        }
        if (read_error(connection))
            method(connection, 5);
    } else {
        alignas(4) Byte route[304];
        if (queue::copy_matching(route_queue(), reinterpret_cast<Dword>(connection),
                                 reinterpret_cast<Dword>(route),
                                 route_connection_comparer(server()))) {
            if (protocol(connection) == 1) {
                at<Dword>(route, 36) = 0;
                destroy_packet(connection);
            }
            at<Dword>(route, 40) = 0;
            if (!at<Dword>(route, 36)) {
                while (remove_server_client(server(), nullptr, at<Dword>(route, 0))) {
                }
                queue::erase(route_queue(), reinterpret_cast<Dword>(route), nullptr);
            } else
                queue::replace(route_queue(), reinterpret_cast<Dword>(route),
                               reinterpret_cast<Dword>(route), nullptr);
        }
    }
    if (protocol(connection) == 1)
        cipher::remove_connection(reinterpret_cast<Dword>(connection));
}
} // namespace nfs::network::session
