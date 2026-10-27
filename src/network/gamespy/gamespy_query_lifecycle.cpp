#include "gamespy_query_runtime.hpp"
namespace nfs::network::authentication::pending::unresolved {
void initialize_sockets();
}
namespace nfs::network::session::pending {
Dword default_query_context[65]{Dword(-1), Dword(-1)};
Dword *query_context = default_query_context;
char master_host[64] = "master.gamespy.com";
namespace {
char query_datagram[256]{};
u_long resolve_query_host(const char *host) {
    u_long address = inet_addr(host);
    if (address == INADDR_NONE && std::strcmp(host, "255.255.255.255"))
        if (hostent *entry = gethostbyname(host))
            std::memcpy(&address, entry->h_addr_list[0], sizeof(address));
    return address;
}
} // namespace

void initialize_query_context(Dword *output, Dword port, SOCKET heartbeat_socket,
                              SOCKET receive_socket, const char *game, const char *key, void *basic,
                              void *info, void *rules, void *players, Dword context) {
    void *owner = default_query_context;
    if (output) {
        owner = std::malloc(260);
        *output = reinterpret_cast<Dword>(owner);
    }
    std::strcpy(query_text(owner, 8), game);
    std::strcpy(query_text(owner, 72), key);
    at<Dword>(owner, 228) = port;
    at<SOCKET>(owner, 4) = heartbeat_socket;
    at<SOCKET>(owner, 0) = receive_socket;
    at<Dword>(owner, 220) = 1;
    at<Byte>(owner, 232) = 1;
    at<Dword>(owner, 256) = context;
    at<void *>(owner, 200) = basic;
    at<void *>(owner, 204) = info;
    at<Dword>(owner, 216) = 0;
    at<Dword>(owner, 224) = 0;
    at<void *>(owner, 212) = players;
    at<void *>(owner, 208) = rules;
    at<Dword>(owner, 252) = 0;
}

Dword initialize_query(Dword output_address, Dword bind_host, Dword port, const char *game,
                      const char *key, void *basic, void *info, void *rules, void *players,
                      Dword context) {
    auto *output = reinterpret_cast<Dword *>(output_address);
    if (output)
        *output = 0;
    authentication::pending::unresolved::initialize_sockets();
    SOCKET socket_handle = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (socket_handle == INVALID_SOCKET)
        return 1;
    const Dword limit = port + 10;
    Dword result = port;
    sockaddr_in endpoint;
    for (; static_cast<int>(port) < static_cast<int>(limit); ++port) {
        endpoint.sin_family = AF_INET;
        endpoint.sin_port = htons(static_cast<u_short>(port));
        endpoint.sin_addr.s_addr =
            bind_host ? resolve_query_host(reinterpret_cast<char *>(bind_host)) : 0;
        if (endpoint.sin_addr.s_addr == htonl(2130706433))
            endpoint.sin_addr.s_addr = 0;
        result = Dword(bind(socket_handle, reinterpret_cast<sockaddr *>(&endpoint), 16));
        if (!result)
            break;
    }
    if (result)
        return 2;
    if (!port) {
        int length = 16;
        if (getsockname(socket_handle, reinterpret_cast<sockaddr *>(&endpoint), &length))
            return 2;
        port = ntohs(endpoint.sin_port);
    }
    initialize_query_context(output, port, socket_handle, socket_handle, game, key, basic, info,
                             rules, players, context);
    void *owner = output ? reinterpret_cast<void *>(*output) : query_context;
    auto &master = at<sockaddr_in>(owner, 236);
    master.sin_family = AF_INET;
    master.sin_port = htons(27900);
    master.sin_addr.s_addr = resolve_query_host(master_host);
    return 0;
}
// The socket is supplied by the caller and the port word is zero.
Dword initialize_query_on_socket(Dword *output, SOCKET socket_handle, const char *game,
                                 const char *key, void *basic, void *info, void *rules,
                                 void *players, Dword context) {
    if (output)
        *output = 0;
    authentication::pending::unresolved::initialize_sockets();
    initialize_query_context(output, 0, socket_handle, socket_handle, game, key, basic, info,
                             rules, players, context);
    void *owner = output ? reinterpret_cast<void *>(*output) : query_context;
    auto &master = at<sockaddr_in>(owner, 236);
    master.sin_family = AF_INET;
    master.sin_port = htons(27900);
    master.sin_addr.s_addr = resolve_query_host(master_host);
    return 0;
}

void poll_query(void *owner) {
    int endpoint_size = 16;
    timeval wait{0, 0};
    sockaddr endpoint;
    if (at<Dword>(owner, 228)) {
        fd_set reads;
        reads.fd_count = 1;
        reads.fd_array[0] = at<SOCKET>(owner, 0);
        for (;;) {
            const int ready = select(64, &reads, nullptr, nullptr, &wait);
            if (ready == SOCKET_ERROR || !ready)
                break;
            int size =
                recvfrom(at<SOCKET>(owner, 0), query_datagram, 255, 0, &endpoint, &endpoint_size);
            if (size != SOCKET_ERROR) {
                query_datagram[size] = 0;
                dispatch_query(owner, query_datagram, Dword(size), &endpoint);
            }
        }
    }
}

void query_heartbeat(void *owner) {
    if (!owner)
        owner = query_context;
    const Dword now = GetTickCount();
    if (at<SOCKET>(owner, 4) != INVALID_SOCKET) {
        const Dword last = at<Dword>(owner, 216);
        const bool regular = now - last < 300001 && last && last <= now;
        if (!regular || (at<Byte>(owner, 232) && now - last > 30000)) {
            char packet[256];
            std::sprintf(packet, "\\heartbeat\\%d\\gamename\\%s", at<Dword>(owner, 228),
                         query_text(owner, 8));
            sendto(at<SOCKET>(owner, 4), packet, int(std::strlen(packet)), 0,
                   &at<sockaddr>(owner, 236), 16);
            at<Dword>(owner, 216) = GetTickCount();
            if (regular && ++at<Byte>(owner, 232) > 10)
                at<Byte>(owner, 232) = 0;
        }
    }
    poll_query(owner);
}
// Independent heartbeat body; this entry does not poll datagrams.
void send_query_heartbeat(void *owner) {
    const Dword now = GetTickCount();
    if (at<SOCKET>(owner, 4) == INVALID_SOCKET)
        return;
    const Dword last = at<Dword>(owner, 216);
    const bool regular = now - last < 300001 && last && last <= now;
    if (regular && (!at<Byte>(owner, 232) || now - last <= 30000))
        return;
    char packet[256];
    std::sprintf(packet, "\\heartbeat\\%d\\gamename\\%s", at<Dword>(owner, 228),
                 query_text(owner, 8));
    sendto(at<SOCKET>(owner, 4), packet, int(std::strlen(packet)), 0,
           &at<sockaddr>(owner, 236), 16);
    at<Dword>(owner, 216) = GetTickCount();
    if (regular && ++at<Byte>(owner, 232) > 10)
        at<Byte>(owner, 232) = 0;
}

void query_lan_think(void *owner) {
    poll_query(owner ? owner : query_context);
}
} // namespace nfs::network::session::pending
