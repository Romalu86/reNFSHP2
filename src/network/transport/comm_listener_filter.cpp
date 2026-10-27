#include "comm_filter_runtime.hpp"
namespace nfs::network::session::pending {

static void disable_listener_modules(void *owner) {
    void *filter = at<void *>(at<void *>(owner, 28), 88);
    while (filter)
        if (method<Dword>(filter, 1) != 3) {
            method(filter, 3, Dword(2));
            filter = method<void *>(filter, 12);
        }
}

void *construct_listener_filter(void *owner, void *connection, Dword disable_modules) {
    construct_filter(owner, 3);
    at<void *>(owner, 28) = nullptr;
    at<const Dword *>(owner, 0) = listener_filter_methods;
    if (at<Dword>(connection, 16) != 1) {
        at<void *>(owner, 28) = connection;
        if (disable_modules)
            disable_listener_modules(owner);
    }
    return owner;
}

static void destroy_listener_filter(void *owner) {
    at<const Dword *>(owner, 0) = listener_filter_methods;
    void *current = connections;
    if (filter_connection(owner, nullptr) == at<void *>(owner, 28))
        while (current)
            if (at<Dword>(current, 12) == 1 && at<Dword>(current, 16) == 2 &&
                !at<void *>(current, 84)) {
                void *next = at<void *>(current, 108);
                at<void *>(next, 104) = at<void *>(current, 104);
                method(current, 1, Dword(1));
                current = next;
            } else
                current = at<void *>(current, 108);
    destroy_filter(owner);
}

static void *__fastcall delete_listener_filter(void *owner, void *, Dword flags) {
    destroy_listener_filter(owner);
    if (flags & 1)
        kernel::memory_free(owner);
    return owner;
}

static void *__fastcall clone_listener_filter(void *owner, void *) {
    void *storage = comm::allocate_object(32);
    return storage ? construct_listener_filter(storage, at<void *>(owner, 28), 0) : nullptr;
}

static void *find_listener_peer(void *owner) {
    void *listener = at<void *>(owner, 28);
    const char *host = connection_remote_host(listener);
    const std::uint16_t port = connection_remote_port(listener);
    for (void *peer = connections; peer; peer = at<void *>(peer, 108))
        if (at<Dword>(peer, 12) == 1 && at<Dword>(peer, 16) == 2 && !at<void *>(peer, 84) &&
            connection_remote_port(peer) == port &&
            !std::strcmp(connection_remote_host(peer), host))
            return peer;
    return nullptr;
}

static void *create_listener_peer(void *owner) {
    alignas(4) Byte information[24]{};
    at<SOCKET>(information, 0) = INVALID_SOCKET;
    void *listener = at<void *>(owner, 28);
    if (!listener)
        _assert("m_pListenerConnection", "cmn\\module_multiplexor.cpp", 156);
    std::memcpy(information + 4, bytes(listener) + 372, 16);
    at<SOCKET>(information, 0) = at<SOCKET>(listener, 368);
    void *peer = method<void *>(listener, 0, Dword(1001), reinterpret_cast<Dword>(information),
                                Dword(24), owner);
    if (peer)
        at<void *>(peer, 84) = nullptr;
    return peer;
}

static void update_listener_peers() {
    for (void *peer = connections; peer; peer = at<void *>(peer, 108))
        if (at<Dword>(peer, 12) == 1 && at<Dword>(peer, 16) == 2 && !at<void *>(peer, 84)) {
            const Dword state = at<Dword>(peer, 68);
            if (!state) {
                swap_error(static_cast<Dword *>(peer), 1);
                if (void *callback = at<void *>(peer, 76))
                    method(callback, 0, Dword(5), Dword(0), Dword(0), peer);
            } else if (state > 2 && state < 6)
                method(peer, 6);
        }
}

static Byte __fastcall send_listener_packet(void *owner, void *, comm::Packet *packet) {
    if (at<Dword>(owner, 24) == 2)
        return method<Byte>(owner, 16, packet);
    if (!packet) {
        if (method<void *>(owner, 8) == at<void *>(owner, 28)) {
            update_listener_peers();
            return method<Byte>(owner, 16, packet);
        }
        return 0;
    }
    void *listener = at<void *>(owner, 28);
    if (!listener)
        return method<Byte>(owner, 16, packet);
    void *connection = method<void *>(owner, 8);
    if (connection != listener && at<Dword>(listener, 16) == 2) {
        connection = method<void *>(owner, 8);
        std::strcpy(reinterpret_cast<char *>(packet->endpoint) + 4,
                    connection_remote_host(connection));
        at<std::uint16_t>(packet->endpoint, 260) = connection_remote_port(connection);
    }
    connection = method<void *>(owner, 8);
    if (at<void *>(owner, 20)) {
        method(owner, 9, at<void *>(owner, 28));
        const Byte sent = method<Byte>(at<void *>(owner, 20), 14, packet);
        method(owner, 9, connection);
        return sent;
    }
    return comm::send_via_driver(at<Dword *>(owner, 28), packet) != 0;
}

static Dword __fastcall receive_listener_packet(void *owner, void *, comm::Packet *packet) {
    if (at<Dword>(owner, 24) == 2 || !packet || !at<void *>(owner, 28) ||
        method<void *>(owner, 8) != at<void *>(owner, 28))
        return method<Dword>(owner, 17, packet);
    void *peer = find_listener_peer(owner);
    if (!peer)
        peer = create_listener_peer(owner);
    if (!peer)
        return 0;
    const int size = comm::available_bytes(packet);
    void *data = comm::packet_data(packet);
    return method<Dword>(peer, 0, Dword(1), reinterpret_cast<Dword>(data), Dword(size), peer) != 0;
}
const Dword listener_filter_methods[]{reinterpret_cast<Dword>(delete_listener_filter),
                                     reinterpret_cast<Dword>(filter_kind),
                                     reinterpret_cast<Dword>(filter_mode),
                                     reinterpret_cast<Dword>(set_filter_mode),
                                     reinterpret_cast<Dword>(enable_filter),
                                     reinterpret_cast<Dword>(disable_filter),
                                     reinterpret_cast<Dword>(clear_filter),
                                     reinterpret_cast<Dword>(clone_listener_filter),
                                     reinterpret_cast<Dword>(filter_connection),
                                     reinterpret_cast<Dword>(set_filter_connection),
                                     reinterpret_cast<Dword>(filter_previous),
                                     reinterpret_cast<Dword>(set_filter_previous),
                                     reinterpret_cast<Dword>(filter_next),
                                     reinterpret_cast<Dword>(set_filter_next),
                                     reinterpret_cast<Dword>(send_listener_packet),
                                     reinterpret_cast<Dword>(receive_listener_packet),
                                     reinterpret_cast<Dword>(filter_send),
                                     reinterpret_cast<Dword>(filter_receive),
                                     reinterpret_cast<Dword>(filter_header_bytes),
                                     reinterpret_cast<Dword>(filter_total_header_bytes)};
} // namespace nfs::network::session::pending
