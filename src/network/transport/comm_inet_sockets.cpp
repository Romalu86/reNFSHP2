#include "comm_connection_runtime.hpp"
namespace nfs::network::session::pending {
namespace {
char hostname_buffer[256]{};
} // namespace

static Dword resolve_inet_host(const char *name) {
    if (!comm_api) {
        comm::set_error(2);
        return 0;
    }
    if (!name) {
        comm::set_error(3);
        return 0;
    }
    get_inet_driver();
    Dword result = 0;
    if (hostent *entry = gethostbyname(name))
        std::memcpy(&result, entry->h_addr_list[0], 4);
    return result;
}

static char *local_inet_hostname() {
    if (!comm_api) {
        comm::set_error(2);
        return nullptr;
    }
    std::memset(hostname_buffer, 0, 256);
    get_inet_driver();
    return gethostname(hostname_buffer, 256) == SOCKET_ERROR ? nullptr : hostname_buffer;
}

Dword local_inet_address() {
    if (!comm_api) {
        comm::set_error(2);
        return 0;
    }
    get_inet_driver();
    return resolve_inet_host(local_inet_hostname());
}

char *format_inet_address(Dword address) {
    if (!comm_api) {
        comm::set_error(2);
        return nullptr;
    }
    get_inet_driver();
    in_addr value;
    value.s_addr = address;
    return inet_ntoa(value);
}

int socket_error(void *connection) {
    int error = WSAGetLastError();
    switch (error) {
    case 10048:
        return 59;
    case 10050:
        return 56;
    case 10054:
        if (connection && at<Dword>(connection, 16) != 2) {
            swap_error(static_cast<Dword *>(connection), 1);
            method(connection, 0, Dword(5), reinterpret_cast<Dword>(connection), Dword(4), connection);
        }
        break;
    case 10061:
        return 17;
    case 10093:
        return 55;
    }
    return error;
}

SOCKET create_connection_socket(void *connection) {
    if (!connection)
        return INVALID_SOCKET;
    int enabled = 1;
    const Dword protocol_kind = at<Dword>(connection, 16);
    SOCKET handle;
    if (protocol_kind == 1) {
        handle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (handle == INVALID_SOCKET) {
            socket_error(nullptr);
            return INVALID_SOCKET;
        }
        setsockopt(handle, SOL_SOCKET, SO_KEEPALIVE, reinterpret_cast<char *>(&enabled), 4);
        setsockopt(handle, IPPROTO_TCP, 1, reinterpret_cast<char *>(&enabled), 4);
    } else {
        if (protocol_kind != 2) {
            _assert("0", "cmn\\driver_inet.cpp", 254);
            return INVALID_SOCKET;
        }
        handle = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (handle == INVALID_SOCKET) {
            socket_error(nullptr);
            return INVALID_SOCKET;
        }
    }
    return handle;
}

Dword close_inet_socket(SOCKET handle) {
    if (handle == INVALID_SOCKET)
        return 0;
    shutdown(handle, 1);
    if (closesocket(handle) == SOCKET_ERROR) {
        socket_error(nullptr);
        return 0;
    }
    return 1;
}

static Dword set_inet_broadcast(SOCKET handle, Dword protocol_kind, Dword enabled) {
    if (protocol_kind != 2) {
        _assert("0", "cmn\\driver_inet.cpp", 323);
        return 1;
    }
    enabled = Byte(enabled) != 0;
    setsockopt(handle, SOL_SOCKET, SO_BROADCAST, reinterpret_cast<char *>(&enabled), 4);
    return 1;
}

Dword configure_inet_address(void *connection) {
    if (!connection)
        return 0;
    const char *host = reinterpret_cast<char *>(bytes(connection) + 112);
    const Dword protocol_kind = at<Dword>(connection, 16);
    bool broadcast = false;
    if (protocol_kind == 2) {
        broadcast = host && !std::strcmp(host, "255.255.255.255");
        set_inet_broadcast(at<SOCKET>(connection, 368), protocol_kind, Dword(broadcast));
    }
    auto &endpoint = at<sockaddr_in>(connection, 372);
    std::memset(&endpoint, 0, 16);
    endpoint.sin_family = AF_INET;
    endpoint.sin_port = htons(at<std::uint16_t>(connection, 72));
    endpoint.sin_addr.s_addr = INADDR_NONE;
    if (host && *host) {
        endpoint.sin_addr.s_addr = inet_addr(host);
        if (endpoint.sin_addr.s_addr == INADDR_NONE && !broadcast) {
            hostent *entry = gethostbyname(host);
            if (!entry) {
                comm::set_error(57);
                return 0;
            }
            std::memmove(&endpoint.sin_addr.s_addr, entry->h_addr_list[0], entry->h_length);
        }
    }
    return 1;
}

Dword __fastcall open_inet_connection(void *driver, void *, void *connection) {
    if (!connection) {
        comm::set_error(3);
        return 0;
    }
    if (at<Dword>(connection, 12) != 1) {
        comm::set_error(52);
        return 0;
    }
    SOCKET handle = at<SOCKET>(connection, 368);
    if (handle == INVALID_SOCKET) {
        comm::set_error(58);
        return 0;
    }
    configure_inet_address(connection);
    auto &remote = at<sockaddr_in>(connection, 372);
    auto &local = at<sockaddr_in>(connection, 388);
    local.sin_family = AF_INET;
    const Dword role = at<Dword>(connection, 20);
    if (!role) {
        if (remote.sin_addr.s_addr == INADDR_NONE)
            remote.sin_addr.s_addr = 0;
        std::memcpy(&local, &remote, 16);
        if (bind(handle, reinterpret_cast<sockaddr *>(&remote), 16) == SOCKET_ERROR) {
            socket_error(nullptr);
            return 0;
        }
        if (at<Dword>(connection, 16) == 1 && listen(handle, 2147483647) == SOCKET_ERROR) {
            socket_error(nullptr);
            return 0;
        }
    } else if (role == 1) {
        if (local.sin_port &&
            bind(handle, reinterpret_cast<sockaddr *>(&local), 16) == SOCKET_ERROR) {
            socket_error(nullptr);
            return 0;
        }
        if (at<Dword>(connection, 16) == 1) {
            if (remote.sin_addr.s_addr == INADDR_NONE) {
                comm::set_error(57);
                return 0;
            }
            if (connect(handle, reinterpret_cast<sockaddr *>(&remote), 16) == SOCKET_ERROR) {
                socket_error(nullptr);
                return 0;
            }
        }
    }
    driver_add_connection(driver, connection);
    return 1;
}

Dword __fastcall close_inet_connection(void *driver, void *, void *connection) {
    if (!connection) {
        comm::set_error(3);
        return 0;
    }
    driver_remove_connection(driver, connection);
    close_inet_socket(at<SOCKET>(connection, 368));
    at<SOCKET>(connection, 368) = INVALID_SOCKET;
    return 1;
}

Byte __fastcall can_send_inet(void *, void *, void *connection) {
    if (!connection || at<Dword>(connection, 12) != 1)
        return 0;
    SOCKET handle = at<SOCKET>(connection, 368);
    if (handle == INVALID_SOCKET)
        return 0;
    fd_set writes, exceptions;
    writes.fd_count = 1;
    writes.fd_array[0] = handle;
    exceptions.fd_count = 0;
    timeval wait{0, 0};
    return select(64, nullptr, &writes, &exceptions, &wait) != 0;
}

int __fastcall send_inet_packet(void *, void *, void *connection, comm::Packet *packet) {
    if (!packet || !connection || !at<Dword>(connection, 68) || at<Dword>(connection, 68) == 1)
        return 0;
    SOCKET handle = at<SOCKET>(connection, 368);
    if (handle == INVALID_SOCKET)
        return 0;
    if (at<Dword>(connection, 16) == 2 && at<Byte>(packet->endpoint, 4)) {
        set_connection_destination(connection, reinterpret_cast<char *>(packet->endpoint) + 4,
                                   at<std::uint16_t>(packet->endpoint, 260));
        configure_inet_address(connection);
    }
    int remaining = comm::available_bytes(packet);
    char *data = static_cast<char *>(comm::packet_data(packet));
    const int flags = at<int>(packet->endpoint, 264);
    int retries = 0, result = 0;
    while (remaining > 0 && retries < 5) {
        result = at<Dword>(connection, 16) == 1
                     ? send(handle, data, remaining, flags)
                     : sendto(handle, data, remaining, flags, &at<sockaddr>(connection, 372), 16);
        if (result < 1) {
            if (result == SOCKET_ERROR)
                socket_error(connection);
            ++retries;
        } else {
            remaining -= result;
            data += result;
            retries = 0;
        }
    }
    return result;
}
} // namespace nfs::network::session::pending
