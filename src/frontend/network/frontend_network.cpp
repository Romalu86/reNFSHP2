#include <winsock2.h>
#include "nfs/frontend/network/frontend_network.hpp"
#include "../../network/gamespy/gamespy_query_runtime.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
namespace nfs::network::frontend {

// the other SDK writers remain open; offsets preserve their shared storage.
alignas(4) std::uint8_t state[2452]{};
Dword (&default_query)[65] = session::pending::default_query_context;
Dword *&current_query = session::pending::query_context;
Dword tracker_socket = 4294967295u, query_identifier = 4294967295u,
     socket_references = 0;
namespace {
TrackerNode *&head() {
    return field<TrackerNode *>(state, 2440);
}
void announce(std::uint8_t *query, int phase) {
    if (!query)
        query = reinterpret_cast<std::uint8_t *>(current_query);
    char packet[256];
    std::sprintf(packet, "\\heartbeat\\%d\\gamename\\%s", field<int>(query, 228),
                 reinterpret_cast<char *>(query + 8));
    std::sprintf(packet + std::strlen(packet), "\\statechanged\\%d", phase);
    sendto(field<SOCKET>(query, 4), packet, static_cast<int>(std::strlen(packet)), 0,
           reinterpret_cast<const sockaddr *>(query + 236), 16);
    field<Dword>(query, 216) = GetTickCount();
}
} // namespace
// Compile only: no network datagram is sent by tests.
void disconnect_tracker(Dword *connection) {
    std::uint8_t packet[512];
    const int bytes =
        std::sprintf(reinterpret_cast<char *>(packet), "\\disc\\\\pid\\%d\\cd\\%s\\ip\\%d",
                     field<Dword>(state, 2448), reinterpret_cast<char *>(connection) + 4,
                     field<Dword>(connection, 44));
    auto *key = state + 2424;
    for (int index = 0; index < bytes; ++index) {
        const std::uint8_t value = *key++;
        packet[index] ^= value;
        if (!*key)
            key = state + 2424;
    }
    sendto(tracker_socket, reinterpret_cast<const char *>(packet), bytes, 0,
           reinterpret_cast<const sockaddr *>(state + 2408), 16);
}

void clear_trackers() {
    while (TrackerNode *node = head()) {
        Dword *connection = node->connection;
        disconnect_tracker(connection);
        node->previous->next = node->next;
        if (node->next)
            node->next->previous = node->previous;
        if (field<Dword>(connection, 72))
            std::free(field<void *>(connection, 72));
        std::free(connection);
        std::free(node);
    }
}
// Tail entry to the same cleanup body.
void clear_trackers_entry() {
    clear_trackers();
}

void heartbeat(std::uint8_t *query) {
    announce(query, 1);
}

void heartbeat_exit(std::uint8_t *query) {
    announce(query, 2);
}

void release_sockets() {
    WSACleanup();
    --socket_references;
}

void close_query(std::uint8_t *query) {
    if (!query)
        query = reinterpret_cast<std::uint8_t *>(current_query);
    const SOCKET first = field<SOCKET>(query, 0);
    if (first != INVALID_SOCKET && field<Dword>(query, 228))
        closesocket(first);
    const SOCKET second = field<SOCKET>(query, 4);
    if (second != INVALID_SOCKET && second != first)
        closesocket(second);
    field<SOCKET>(query, 4) = INVALID_SOCKET;
    field<SOCKET>(query, 0) = INVALID_SOCKET;
    field<Dword>(query, 216) = 0;
    if (query != reinterpret_cast<std::uint8_t *>(default_query))
        std::free(query);
    release_sockets();
}

void exit() {
    std::strcpy(reinterpret_cast<char *>(state) + 540, "exiting");
    heartbeat(nullptr);
    heartbeat_exit(nullptr);
    close_query(nullptr);
    query_identifier = 4294967295u;
    field<std::uint8_t>(state, 0) = 0;
}
} // namespace nfs::network::frontend
