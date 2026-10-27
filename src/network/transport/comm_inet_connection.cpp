#include "comm_connection_runtime.hpp"
#include "nfs/core/memory/memory_primitives.hpp"
namespace nfs::network::session::pending {

static char *set_connection_host(void *owner, const char *host) {
    char *text = reinterpret_cast<char *>(bytes(owner) + 112);
    if (host)
        kernel::memory_copy(text, host, 256);
    else
        std::memset(text, 0, 256);
    return text;
}

Dword set_connection_destination(void *owner, const char *host, std::uint16_t port) {
    set_connection_host(owner, host);
    at<std::uint16_t>(owner, 72) = port;
    return 1;
}

Dword set_connection_endpoint(void *owner, const sockaddr *endpoint) {
    if (!endpoint)
        return 0;
    std::memcpy(bytes(owner) + 372, endpoint, 16);
    Dword address = 0;
    std::memmove(&address, endpoint->sa_data + 2, 4);
    std::strcpy(reinterpret_cast<char *>(bytes(owner) + 112), format_inet_address(address));
    return 1;
}

Dword set_connection_local_endpoint(void *owner, const sockaddr *endpoint) {
    if (!endpoint)
        return 0;
    std::memcpy(bytes(owner) + 388, endpoint, 16);
    at<Dword>(owner, 392) = local_inet_address();
    return 1;
}

std::uint16_t connection_remote_port(void *owner) {
    return ntohs(at<std::uint16_t>(owner, 374));
}

char *connection_remote_host(void *owner) {
    return format_inet_address(at<Dword>(owner, 376));
}

char *__fastcall connection_address_text(void *, void *, Dword address) {
    return format_inet_address(address);
}

Dword __fastcall connection_address_number(void *, void *, const char *address) {
    if (!comm_api) {
        comm::set_error(2);
        return 0;
    }
    if (!address)
        return 0;
    get_inet_driver();
    return inet_addr(address);
}

Dword __fastcall listen_connection(void *owner, void *, const char *host, std::uint16_t port) {
    if (at<SOCKET>(owner, 368) == INVALID_SOCKET)
        create_inet_socket(owner);
    if (!at<Dword>(owner, 20) && at<Dword>(owner, 68) == 4)
        return 1;
    at<Dword>(owner, 20) = 0;
    set_connection_destination(owner, host, port);
    if (at<Dword>(owner, 16) == 2 && at<Byte>(owner, 40)) {
        void *storage = comm::allocate_object(32);
        link_connection_filters(owner, storage ? construct_listener_filter(storage, owner, 64)
                                               : nullptr);
    }
    if (method<Byte>(at<void *>(owner, 84), 4, owner)) {
        swap_error(static_cast<Dword *>(owner), 4);
        return 1;
    }
    swap_error(static_cast<Dword *>(owner), 0);
    comm::set_error(13);
    return 0;
}

Dword __fastcall connect_connection(void *owner, void *, const char *host, std::uint16_t port,
                                   std::uint16_t local_port) {
    if (host) {
        if (at<SOCKET>(owner, 368) == INVALID_SOCKET)
            create_inet_socket(owner);
        at<Dword>(owner, 20) = 1;
        set_connection_destination(owner, host, port);
        if (local_port) {
            at<std::uint16_t>(owner, 390) = htons(local_port);
            at<Dword>(owner, 392) = local_inet_address();
        }
        if (method<Byte>(at<void *>(owner, 84), 4, owner)) {
            method(owner, 0, Dword(3), Dword(0), Dword(0), owner);
            return 1;
        }
        swap_error(static_cast<Dword *>(owner), 0);
        comm::set_error(14);
    }
    return 0;
}
namespace {
Dword accept_inet_connection(void *owner, Dword information, const char *file, unsigned line,
                            Dword role) {
    void *info = reinterpret_cast<void *>(information);
    if (!info)
        _assert("pInfo", file, line);
    const SOCKET socket_handle = at<SOCKET>(info, 0);
    if (socket_handle == INVALID_SOCKET || bytes(info) + 4 == nullptr)
        return 0;
    void *copy = method<void *>(owner, 4);
    at<Dword>(copy, 20) = role;
    swap_error(static_cast<Dword *>(copy), 4);
    at<SOCKET>(copy, 368) = socket_handle;
    set_connection_endpoint(copy, &at<sockaddr>(info, 4));
    set_connection_local_endpoint(copy, &at<sockaddr>(owner, 388));
    if (void *callback = at<void *>(owner, 76))
        method(callback, 0, Dword(2), reinterpret_cast<Dword>(copy), Dword(4), copy);
    return reinterpret_cast<Dword>(copy);
}
} // namespace

Dword __fastcall tcp_connection_event(void *owner, void *, Dword event, Dword first, Dword second,
                                     void *source) {
    if (event != 1001)
        return connection_event(owner, nullptr, event, first, second, source);
    return accept_inet_connection(owner, first, "cmn\\connection_tcp.cpp", 72, 1);
}

Dword __fastcall udp_connection_event(void *owner, void *, Dword event, Dword first, Dword second,
                                     void *source) {
    if (!at<Byte>(owner, 40) || event != 1001)
        return connection_event(owner, nullptr, event, first, second, source);
    return accept_inet_connection(owner, first, "cmn\\connection_udp.cpp", 179, 0);
}
const Dword inet_connection_methods[]{reinterpret_cast<Dword>(connection_event),
                                     reinterpret_cast<Dword>(delete_inet_connection),
                                     reinterpret_cast<Dword>(connection_address_text),
                                     reinterpret_cast<Dword>(connection_address_number),
                                     reinterpret_cast<Dword>(clone_inet_connection),
                                     reinterpret_cast<Dword>(close_connection_method),
                                     reinterpret_cast<Dword>(update_connection),
                                     reinterpret_cast<Dword>(send_connection),
                                     reinterpret_cast<Dword>(listen_connection),
                                     reinterpret_cast<Dword>(connect_connection)};
const Dword tcp_connection_methods[]{reinterpret_cast<Dword>(tcp_connection_event),
                                    reinterpret_cast<Dword>(delete_tcp_connection),
                                    reinterpret_cast<Dword>(connection_address_text),
                                    reinterpret_cast<Dword>(connection_address_number),
                                    reinterpret_cast<Dword>(clone_inet_connection),
                                    reinterpret_cast<Dword>(close_connection_method),
                                    reinterpret_cast<Dword>(update_connection),
                                    reinterpret_cast<Dword>(send_connection),
                                    reinterpret_cast<Dword>(listen_connection),
                                    reinterpret_cast<Dword>(connect_connection)};
const Dword udp_connection_methods[]{reinterpret_cast<Dword>(udp_connection_event),
                                    reinterpret_cast<Dword>(delete_udp_connection),
                                    reinterpret_cast<Dword>(connection_address_text),
                                    reinterpret_cast<Dword>(connection_address_number),
                                    reinterpret_cast<Dword>(clone_udp_connection),
                                    reinterpret_cast<Dword>(close_connection_method),
                                    reinterpret_cast<Dword>(update_connection),
                                    reinterpret_cast<Dword>(send_connection),
                                    reinterpret_cast<Dword>(listen_connection),
                                    reinterpret_cast<Dword>(connect_connection)};
} // namespace nfs::network::session::pending
