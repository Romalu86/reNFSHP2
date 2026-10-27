#include "gamespy_http_runtime.hpp"
namespace nfs::network::session::pending::http {

bool initialize_buffer(void *owner, Buffer *target, int capacity, int growth) {
    if (!owner || !target || capacity < 1)
        return false;
    if (growth > 0) {
        target->owner = owner;
        target->data = nullptr;
        target->capacity = target->size = target->position = 0;
        target->growth = growth;
        target->fixed = target->borrowed = 0;
        char *data = static_cast<char *>(std::realloc(nullptr, capacity));
        if (data) {
            target->capacity = capacity;
            target->data = data;
            *data = 0;
            return true;
        }
    }
    return false;
}

void destroy_buffer(Buffer *target) {
    if (target && target->data) {
        if (!target->borrowed)
            std::free(target->data);
        std::memset(target, 0, 32);
    }
}

bool append_buffer(Buffer *target, const void *source, int count) {
    if (!target || !source || count < 0)
        return false;
    if (!count)
        count = int(std::strlen(static_cast<const char *>(source)));
    const int size = int(Dword(target->size) + Dword(count));
    while (size >= target->capacity) {
        if (target->fixed) {
            fail(target->owner, 2);
            return false;
        }
        if (target->growth < 1) {
            fail(target->owner, 1);
            return false;
        }
        const int capacity = int(Dword(target->capacity) + Dword(target->growth));
        auto *data = static_cast<char *>(std::realloc(target->data, capacity));
        if (!data) {
            fail(target->owner, 1);
            return false;
        }
        target->data = data;
        target->capacity = capacity;
    }
    std::memcpy(target->data + target->size, source, std::size_t(count));
    target->size = size;
    target->data[size] = 0;
    return true;
}

bool append_header(Buffer *target, const char *key, const char *value) {
    return append_buffer(target, key, 0) && append_buffer(target, ": ", 2) &&
           append_buffer(target, value, 0) && append_buffer(target, "\r\n", 2);
}

bool append_character(Buffer *target, char value) {
    if (!target)
        return false;
    const int size = int(Dword(target->size) + 1);
    if (size >= target->capacity) {
        if (target->fixed) {
            fail(target->owner, 2);
            return false;
        }
        if (target->growth < 1) {
            fail(target->owner, 1);
            return false;
        }
        const int capacity = int(Dword(target->capacity) + Dword(target->growth));
        auto *data = static_cast<char *>(std::realloc(target->data, capacity));
        if (!data) {
            fail(target->owner, 1);
            return false;
        }
        target->data = data;
        target->capacity = capacity;
    }
    target->data[target->size] = value;
    target->size = size;
    target->data[size] = 0;
    return true;
}

void append_integer(Buffer *target, Dword value) {
    char number[16];
    std::sprintf(number, "%d", value);
    append_buffer(target, number, 0);
}

void clear_buffer(Buffer *target) {
    target->size = target->position = 0;
    *target->data = 0;
}

bool poll_socket(SOCKET handle, int *readable, int *writable, int *exceptional) {
    fd_set reads, writes, exceptions;
    reads.fd_count = writes.fd_count = exceptions.fd_count = 1;
    reads.fd_array[0] = writes.fd_array[0] = exceptions.fd_array[0] = handle;
    timeval wait{0, 0};
    const int ready = select(64, readable ? &reads : nullptr, writable ? &writes : nullptr,
                             exceptional ? &exceptions : nullptr, &wait);
    if (ready == SOCKET_ERROR)
        return false;
    if (readable)
        *readable = ready > 0 && __WSAFDIsSet(handle, &reads);
    if (writable)
        *writable = ready > 0 && __WSAFDIsSet(handle, &writes);
    if (exceptional)
        *exceptional = ready > 0 && __WSAFDIsSet(handle, &exceptions);
    return true;
}

int receive_request(void *owner, char *data, int *capacity) {
    int limit = *capacity - 1;
    if (at<Dword>(owner, 244)) {
        const Dword now = GetTickCount();
        if (now < at<Dword>(owner, 248) + throttle_period)
            return 1;
        at<Dword>(owner, 248) = now;
        if (throttle_bytes <= limit)
            limit = throttle_bytes;
    }
    const int received = recv(at<SOCKET>(owner, 64), data, limit, 0);
    if (received == SOCKET_ERROR) {
        const int error = WSAGetLastError();
        if (error != 10035 && error != 10036) {
            at<int>(owner, 68) = error;
            fail(owner, 5);
            at<Dword>(owner, 240) = 1;
            return 3;
        }
        return 1;
    }
    if (received) {
        data[received] = 0;
        *capacity = received;
        return 0;
    }
    at<Dword>(owner, 240) = 1;
    return 2;
}

int send_request(void *owner, const char *data, int length) {
    const int sent = send(at<SOCKET>(owner, 64), data, length, 0);
    if (sent != SOCKET_ERROR) {
        if (at<Dword>(owner, 12) == 3)
            at<Dword>(owner, 264) += Dword(sent);
        return sent;
    }
    const int error = WSAGetLastError();
    if (error == 10035)
        return 0;
    at<int>(owner, 68) = error;
    fail(owner, 5);
    return -1;
}
// The function always drains owner+48.
bool flush_request(Buffer *, void *owner) {
    for (;;) {
        int writable, exceptional;
        if (!poll_socket(at<SOCKET>(owner, 64), nullptr, &writable, &exceptional) ||
            exceptional) {
            fail(owner, 5);
            at<int>(owner, 68) = WSAGetLastError();
            return false;
        }
        if (!writable)
            return true;
        const int sent = send_request(owner, at<char *>(owner, 76) + at<int>(owner, 88),
                                      at<int>(owner, 84) - at<int>(owner, 88));
        if (sent == -1)
            return false;
        at<Dword>(owner, 88) += Dword(sent);
        if (at<int>(owner, 84) <= at<int>(owner, 88))
            return true;
    }
}

Byte send_or_queue(void *owner, const char *data, int length) {
    int sent = 0;
    if (!at<Dword>(owner, 84)) {
        sent = send(at<SOCKET>(owner, 64), data, length, 0);
        if (sent == SOCKET_ERROR) {
            const int error = WSAGetLastError();
            if (error != 10035) {
                at<int>(owner, 68) = error;
                fail(owner, 5);
                return 0;
            }
            sent = 0;
        } else if (at<Dword>(owner, 12) == 3)
            at<Dword>(owner, 264) += Dword(sent);
        if (sent == length)
            return 1;
    }
    return append_buffer(buffer(owner, 72), data + sent, length - sent) ? 2 : 0;
}

bool set_blocking(SOCKET handle, Dword enabled) {
    u_long nonblocking = enabled == 0;
    return ioctlsocket(handle, FIONBIO, &nonblocking) == 0;
}

bool set_receive_buffer(SOCKET handle, int bytes) {
    return setsockopt(handle, SOL_SOCKET, SO_RCVBUF, reinterpret_cast<char *>(&bytes), 4) !=
           SOCKET_ERROR;
}
} // namespace nfs::network::session::pending::http
