#include "comm_connection_runtime.hpp"
namespace nfs::network::session::pending {
namespace {
char receive_buffer[8192]{};
} // namespace

int receive_tcp_socket(void *driver, void *connection) {
    if (!connection)
        return 0;
    if (!at<Dword>(connection, 20)) {
        alignas(4) Byte information[24]{};
        at<SOCKET>(information, 0) = INVALID_SOCKET;
        int length = 16;
        SOCKET accepted =
            accept(at<SOCKET>(connection, 368), &at<sockaddr>(information, 4), &length);
        at<SOCKET>(information, 0) = accepted;
        if (accepted != INVALID_SOCKET)
            method(connection, 0, Dword(1001), reinterpret_cast<Dword>(information), Dword(24),
                   driver);
        return 0;
    }
    const int size = recv(at<SOCKET>(connection, 368), receive_buffer, 8192, 0);
    if (size > 0)
        method(connection, 0, Dword(1), reinterpret_cast<Dword>(receive_buffer), Dword(size),
               connection);
    else if (!size) {
        swap_error(static_cast<Dword *>(connection), 1);
        method(connection, 0, Dword(5), reinterpret_cast<Dword>(connection), Dword(4), connection);
    } else
        socket_error(connection);
    return size;
}

int receive_udp_socket(void *connection) {
    if (!connection)
        return 0;
    sockaddr endpoint;
    int length = 16;
    const int size =
        recvfrom(at<SOCKET>(connection, 368), receive_buffer, 8192, 0, &endpoint, &length);
    if (size > 0) {
        set_connection_endpoint(connection, &endpoint);
        method(connection, 0, Dword(1), reinterpret_cast<Dword>(receive_buffer), Dword(size),
               connection);
    } else if (!size) {
        swap_error(static_cast<Dword *>(connection), 1);
        method(connection, 0, Dword(5), reinterpret_cast<Dword>(connection), Dword(4), connection);
    } else
        socket_error(connection);
    return size;
}

Byte poll_inet_sockets(void *driver, fd_set *reads, void **connections_to_poll, int count,
                       timeval *wait) {
    Byte received = 0;
    if (!reads || !connections_to_poll || count <= 0)
        return 0;
    fd_set exceptions;
    exceptions.fd_count = 0;
    if (select(count, reads, nullptr, &exceptions, wait))
        for (int i = 0; i < count; ++i) {
            void *connection = connections_to_poll[i];
            if (__WSAFDIsSet(at<SOCKET>(connection, 368), reads)) {
                received = 1;
                if (at<Dword>(connection, 16) == 1)
                    receive_tcp_socket(driver, connection);
                else
                    receive_udp_socket(connection);
            }
        }
    return received;
}

void __fastcall poll_inet_set(void *driver, void *, void *set) {
    if (!set) {
        comm::set_error(3);
        return;
    }
    auto **batch = reinterpret_cast<void **>(bytes(driver) + 32);
    fd_set reads;
    reads.fd_count = 0;
    timeval wait{0, 0};
    int count = 0;
    std::memset(batch, 0, 256);
    auto **items = at<void **>(set, 8);
    int last = at<int>(set, 12) - 1;
    for (int i = 0; i <= last; ++i) {
        if (at<Dword>(driver, 16) != 2)
            break;
        void *connection = items[i];
        if (!connection || at<SOCKET>(connection, 368) == INVALID_SOCKET)
            continue;
        switch (at<Dword>(connection, 68)) {
        case 0:
        case 1:
        case 2:
            continue;
        case 3:
        case 4:
        case 5:
            break;
        default:
            _assert("0", "cmn\\driver_inet.cpp", 1055);
            continue;
        }
        const SOCKET handle = at<SOCKET>(connection, 368);
        Dword unique = 0;
        while (unique < reads.fd_count && reads.fd_array[unique] != handle)
            ++unique;
        if (unique == reads.fd_count && reads.fd_count < 64)
            reads.fd_array[reads.fd_count++] = handle;
        if (at<Byte>(connection, 48)) {
            if (wait.tv_sec < at<int>(connection, 52))
                wait.tv_sec = at<int>(connection, 52);
            if (wait.tv_usec < at<int>(connection, 56))
                wait.tv_usec = at<int>(connection, 56);
        }
        batch[count++] = connection;
        if (count == 64) {
            poll_inet_sockets(driver, &reads, batch, 64, &wait);
            count = 0;
            reads.fd_count = 0;
            std::memset(batch, 0, 256);
            // 97 clears the scan index and final index here.
            i = last = 0;
        }
    }
    if (count > 0)
        poll_inet_sockets(driver, &reads, batch, count, &wait);
}
} // namespace nfs::network::session::pending
