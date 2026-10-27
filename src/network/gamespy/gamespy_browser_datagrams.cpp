#include "gamespy_browser.hpp"
namespace nfs::network::session::pending::browser {

Dword pump_lan(Browser *owner) {
    const SOCKET connection = owner->socket;
    timeval timeout{0, 0};
    int endpoint_size = 16;
    for (;;) {
        fd_set readable;
        readable.fd_count = 1;
        readable.fd_array[0] = connection;
        const int selected = select(64, &readable, nullptr, nullptr, &timeout);
        if (selected == SOCKET_ERROR || !selected)
            break;
        char packet[1500];
        sockaddr_in sender;
        const int received = recvfrom(connection, packet, 1499, 0,
                                      reinterpret_cast<sockaddr *>(&sender), &endpoint_size);
        if (received == SOCKET_ERROR)
            continue;
        packet[1499] = 0; // Fixed terminator, not packet[received].
        if (!std::strstr(packet, "\\final\\"))
            continue;
        const int count = int(http::dynamic_array_count(owner->servers));
        if (find_server(owner, sender.sin_addr.s_addr, ntohs(sender.sin_port), count) == -1) {
            Server *server =
                create_server(sender.sin_addr.s_addr, ntohs(sender.sin_port), 5, owner->strings);
            http::append_dynamic_array(owner->servers, &server);
        }
    }
    if (ticks() - owner->broadcast_started > 3000) {
        closesocket(connection);
        owner->socket = INVALID_SOCKET;
        change_phase(owner, 3);
    }
    return 0;
}

Dword pump_queries(Browser *owner) {
    int active = 0;
    fd_set readable;
    readable.fd_count = 0;
    timeval timeout{0, 0};
    const int initial_count = owner->query_count;
    Query *const initial_queries = owner->queries;
    for (int index = 0; index < initial_count; ++index) {
        Query &query = initial_queries[index];
        if (query.server) {
            ++active;
            FD_SET(query.socket, &readable);
        }
    }
    if (active) {
        const int selected = select(64, &readable, nullptr, nullptr, &timeout);
        if (selected != SOCKET_ERROR && selected) {
            for (int index = 0; index < owner->query_count; ++index) {
                Query *query = &owner->queries[index];
                if (!query->server || !FD_ISSET(query->socket, &readable))
                    continue;
                char packet[1500];
                sockaddr_in sender;
                int endpoint_size = 16;
                const int received =
                    recvfrom(query->socket, packet, 1499, 0, reinterpret_cast<sockaddr *>(&sender),
                             &endpoint_size);
                const auto *expected = reinterpret_cast<const sockaddr_in *>(&query->endpoint);
                if (sender.sin_addr.s_addr != expected->sin_addr.s_addr ||
                    sender.sin_port != expected->sin_port)
                    continue;
                if (received == SOCKET_ERROR) {
                    query->server = nullptr;
                    continue;
                }
                packet[received] = 0;
                const bool final = std::strstr(packet, "\\final\\") != nullptr;
                Server *server = query->server;
                if (server->ping == 9999)
                    server->ping =
                        std::int16_t(std::uint16_t(ticks()) - std::uint16_t(query->started));
                parse_server_properties(server, packet);
                if (final) {
                    const int count = int(http::dynamic_array_count(owner->servers));
                    owner->callback(owner, 2, owner->context, server,
                                    int(Dword(owner->next_server) * 100) / count);
                    owner->queries[index].server = nullptr;
                }
            }
        }
    }
    const int query_count = owner->query_count;
    Query *const timeout_queries = owner->queries;
    for (int index = 0; index < query_count; ++index) {
        Query &query = timeout_queries[index];
        if (query.server && ticks() - query.started > 3000)
            query.server = nullptr;
    }
    if (!owner->cancel_requested) {
        void *servers = owner->servers;
        if (owner->next_server < int(http::dynamic_array_count(servers)) || active) {
            static constexpr const char *packets[]{"\\basic\\",         "\\info\\",
                                                   "\\rules\\",         "\\players\\",
                                                   "\\info\\\\rules\\", "\\status\\"};
            static constexpr int lengths[]{7, 6, 7, 9, 13, 8};
            for (int index = 0; index < query_count; ++index) {
                const int next = owner->next_server;
                if (int(http::dynamic_array_count(servers)) <= next)
                    return 0;
                Query &query = owner->queries[index];
                if (query.server)
                    continue;
                Server *server = *static_cast<Server **>(http::dynamic_array_item(servers, next));
                owner->next_server = int(Dword(next) + 1);
                query.server = server;
                auto *endpoint = reinterpret_cast<sockaddr_in *>(&query.endpoint);
                endpoint->sin_family = AF_INET;
                endpoint->sin_addr.s_addr = inet_addr(server_address_text(server));
                endpoint->sin_port = htons(server_port(server));
                sendto(query.socket, packets[server->query_type], lengths[server->query_type], 0,
                       &query.endpoint, 16);
                query.started = ticks();
            }
            return 0;
        }
    }
    Query *const closing_queries = owner->queries;
    for (int index = 0; index < query_count; ++index)
        closesocket(closing_queries[index].socket);
    change_phase(owner, 0);
    return 0;
}
} // namespace nfs::network::session::pending::browser
