#include "gamespy_browser.hpp"
namespace nfs::network::session::pending::browser {
bool allocate_query_sockets(Browser *owner) {
    for (int index = 0; index < owner->query_count; ++index) {
        Query &query = owner->queries[index];
        query.socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (query.socket == INVALID_SOCKET) {
            if (!index)
                return false;
            owner->query_count = index;
            break;
        }
        query.server = nullptr;
        query.started = 0;
    }
    return true;
}
void pump_synchronously(Browser *owner) {
    while (owner->phase) {
        switch (owner->phase) {
        case 1:
            pump_master(owner);
            break;
        case 2:
            pump_lan(owner);
            break;
        case 3:
            pump_queries(owner);
            break;
        }
        sleep(10);
    }
}

Dword refresh_master(Browser *owner, Dword asynchronous, const char *filter, Dword type) {
    if (owner->phase)
        return 4;
    owner->query_type = type;
    sockaddr_in master;
    master.sin_family = AF_INET;
    master.sin_port = htons(28900);
    master.sin_addr.s_addr = inet_addr("master.gamespy.com");
    if (master.sin_addr.s_addr == INADDR_NONE) {
        hostent *entry = gethostbyname("master.gamespy.com");
        if (!entry)
            return 2;
        master.sin_addr.s_addr = *reinterpret_cast<const Dword *>(entry->h_addr_list[0]);
    }
    const SOCKET connection = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    owner->socket = connection;
    if (connection == INVALID_SOCKET)
        return 1;
    if (connect(connection, reinterpret_cast<sockaddr *>(&master), 16)) {
        closesocket(connection);
        return 3;
    }
    char packet[256];
    const int received = recv(connection, packet, 255, 0);
    if (received == SOCKET_ERROR)
        return 3;
    packet[received] = 0;
    char *challenge = std::strstr(packet, "\\secure\\");
    if (!challenge)
        return 5;
    challenge += std::strlen("\\secure\\");
    encrypt_validation(reinterpret_cast<const Byte *>(owner->key), 6,
                       reinterpret_cast<Byte *>(challenge), 6);
    for (unsigned index = 0; index < 6; ++index)
        challenge[index] ^= owner->key[index];
    char validation[64];
    encode_validation(reinterpret_cast<const Byte *>(challenge), 6, validation);
    std::sprintf(packet,
                 "\\gamename\\%s\\gamever\\%s\\location\\0\\validate\\%"
                 "s\\enctype\\2\\final\\\\queryid\\1.1\\",
                 owner->query_game, "1.8", validation);
    int sent = send(owner->socket, packet, int(std::strlen(packet)), 0);
    if (sent == SOCKET_ERROR || !sent)
        return 3;
    const char *list = "cmp";
    if (owner->query_type == 6) {
        list = "groups";
        owner->row_phase = 0;
    } else if (owner->query_type == 7) {
        list = "info2";
        owner->row_phase = 0;
    }
    if (!filter)
        std::sprintf(packet, "\\list\\%s\\gamename\\%s\\final\\", list, owner->game);
    else
        std::sprintf(packet, "\\list\\%s\\gamename\\%s\\where\\%s\\final\\", list, owner->game,
                     filter);
    sent = send(owner->socket, packet, int(std::strlen(packet)), 0);
    if (sent == SOCKET_ERROR || !sent)
        return 3;
    change_phase(owner, 1);
    if (type != 6 && type != 7)
        if (!allocate_query_sockets(owner))
            return 1;
    owner->next_server = 0;
    owner->cancel_requested = 0;
    owner->previous_count = int(http::dynamic_array_count(owner->servers));
    owner->cipher.depth = -1;
    if (!asynchronous)
        pump_synchronously(owner);
    return 0;
}

Dword refresh_lan(Browser *owner, Dword asynchronous, std::uint16_t first, int last, int stride) {
    if (owner->phase)
        return 4;
    if (!allocate_query_sockets(owner))
        return 1;
    const SOCKET connection = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    owner->socket = connection;
    const int enabled = 1;
    if (connection == INVALID_SOCKET || setsockopt(connection, SOL_SOCKET, SO_BROADCAST,
                                                   reinterpret_cast<const char *>(&enabled), 4))
        return 1;
    sockaddr_in endpoint;
    endpoint.sin_family = AF_INET;
    endpoint.sin_addr.s_addr = INADDR_BROADCAST;
    for (Dword port = first; int(port & 65535) <= last; port += Dword(stride)) {
        endpoint.sin_port = htons(static_cast<std::uint16_t>(port));
        sendto(connection, "\\status\\", 8, 0, reinterpret_cast<sockaddr *>(&endpoint), 16);
    }
    change_phase(owner, 2);
    owner->broadcast_started = ticks();
    owner->next_server = 0;
    owner->cancel_requested = 0;
    if (!asynchronous)
        pump_synchronously(owner);
    return 0;
}

Dword refresh_server(Browser *owner, const char *host, int port, Dword asynchronous, Dword type) {
    const Dword address = inet_addr(host);
    const int count = int(http::dynamic_array_count(owner->servers));
    const int selected = find_server(owner, address, port, count);
    const Dword current_phase = owner->phase;
    if (!current_phase) {
        if (!allocate_query_sockets(owner))
            return 1;
        Server *server;
        if (selected == -1)
            server = create_server(address, std::uint16_t(port), type, owner->strings);
        else {
            server = *static_cast<Server **>(http::dynamic_array_item(owner->servers, selected));
            server->query_type = type;
            server->ping = 9999;
            remove_array_without_destroy(owner->servers, selected);
        }
        http::append_dynamic_array(owner->servers, &server);
        owner->next_server = int(http::dynamic_array_count(owner->servers) - 1);
        owner->cancel_requested = 0;
        change_phase(owner, 3);
        if (!asynchronous)
            pump_synchronously(owner);
    } else if (selected == -1) {
        Server *server = create_server(address, std::uint16_t(port), type, owner->strings);
        insert_array(owner->servers, &server, owner->next_server);
        if (current_phase == 1 || current_phase == 2)
            ++owner->previous_count;
    } else {
        Server *server =
            *static_cast<Server **>(http::dynamic_array_item(owner->servers, selected));
        if (selected < owner->next_server) {
            server->query_type = type;
            server->ping = 9999;
            for (int index = 0; index < owner->query_count; ++index)
                if (owner->queries[index].server == server) {
                    owner->queries[index].server = nullptr;
                    break;
                }
            const int next = owner->next_server;
            insert_array(owner->servers, &server, next);
            remove_array_without_destroy(owner->servers, selected);
            owner->next_server = int(Dword(next) - 1);
        }
    }
    return 0;
}
} // namespace nfs::network::session::pending::browser
