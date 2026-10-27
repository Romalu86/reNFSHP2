#include "gamespy_http_runtime.hpp"
namespace nfs::network::authentication::pending {
char *duplicate(const char *);
}
namespace nfs::network::authentication::pending::unresolved {
void initialize_sockets();
}
namespace nfs::network::session::pending::http {

void resolve_request(void *owner) {
    notify_progress(owner, 0, 0);
    authentication::pending::unresolved::initialize_sockets();
    char *url = owner ? at<char *>(owner, 16) : nullptr;
    if (!url || std::strncmp(url, "http://", 7)) {
        fail(owner, 3);
        return;
    }
    char *host = url + 7;
    char *path = host + std::strcspn(host, ":/");
    const char delimiter = *path;
    *path = 0;
    char *copy = authentication::pending::duplicate(host);
    at<char *>(owner, 20) = copy;
    if (!copy) {
        fail(owner, 3);
        return;
    }
    *path = delimiter;
    if (*path == ':') {
        ++path;
        at<std::uint16_t>(owner, 28) = std::uint16_t(std::atoi(path));
        if (!at<std::uint16_t>(owner, 28)) {
            fail(owner, 3);
            return;
        }
        do {
            ++path;
        } while (*path && *path != '/');
    } else
        at<std::uint16_t>(owner, 28) = 80;
    if (!*path)
        path = const_cast<char *>("/");
    char *resource = authentication::pending::duplicate(path);
    at<char *>(owner, 32) = resource;
    for (char *space = std::strchr(resource, ' '); space; space = std::strchr(resource, ' '))
        *space = '+';
    if (!resource) {
        fail(owner, 3);
        return;
    }
    const char *server = proxy_host ? proxy_host : copy;
    at<Dword>(owner, 24) = inet_addr(server);
    if (at<Dword>(owner, 24) == INADDR_NONE) {
        hostent *entry = gethostbyname(server);
        if (!entry) {
            fail(owner, 4);
            return;
        }
        std::memcpy(bytes(owner) + 24, entry->h_addr_list[0], 4);
    }
    at<Dword>(owner, 12) = 1;
    notify_progress(owner, 0, 0);
}

void connect_request(void *owner) {
    if (at<SOCKET>(owner, 64) == INVALID_SOCKET) {
        SOCKET handle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        at<SOCKET>(owner, 64) = handle;
        if (handle == INVALID_SOCKET || !set_blocking(handle, 0)) {
            fail(owner, 5);
            at<int>(owner, 68) = WSAGetLastError();
            return;
        }
        if (at<Dword>(owner, 244))
            set_receive_buffer(handle, throttle_bytes);
        sockaddr_in endpoint{};
        endpoint.sin_family = AF_INET;
        endpoint.sin_port = htons(proxy_host ? proxy_port : at<std::uint16_t>(owner, 28));
        endpoint.sin_addr.s_addr = at<Dword>(owner, 24);
        if (connect(handle, reinterpret_cast<sockaddr *>(&endpoint), 16) == SOCKET_ERROR) {
            const int error = WSAGetLastError();
            if (error != 10035 && error != 10036) {
                fail(owner, 6);
                at<int>(owner, 68) = error;
                return;
            }
        }
    }
    int writable, exceptional;
    const bool polled = poll_socket(at<SOCKET>(owner, 64), nullptr, &writable, &exceptional);
    if (!polled || exceptional) {
        fail(owner, 6);
        if (!polled)
            at<int>(owner, 68) = WSAGetLastError();
    } else if (writable) {
        at<Dword>(owner, 12) = 2;
        notify_progress(owner, 0, 0);
    }
}

const char *post_content_type(void *owner) {
    void *post = at<void *>(owner, 252);
    if (!post)
        return "";
    return at<Dword>(post, 12) ? multipart_content_type : urlencoded_content_type;
}

void send_headers(void *owner) {
    auto *out = buffer(owner, 72);
    if (!at<Dword>(owner, 84)) {
        const char *verb = at<void *>(owner, 252)   ? "POST "
                           : at<Dword>(owner, 8) == 3 ? "HEAD "
                                                     : "GET ";
        append_buffer(out, verb, 0);
        append_buffer(out, at<char *>(owner, proxy_host ? 16 : 32), 0);
        append_buffer(out, request_version, 0);
        if (at<std::uint16_t>(owner, 28) == 80)
            append_header(out, "Host", at<char *>(owner, 20));
        else {
            append_buffer(out, "Host: ", 0);
            append_buffer(out, at<char *>(owner, 20), 0);
            append_character(out, ':');
            append_integer(out, at<std::uint16_t>(owner, 28));
            append_buffer(out, "\r\n", 2);
        }
        append_header(out, "User-Agent", "GameSpyHTTP/1.0");
        if (at<void *>(owner, 252)) {
            char length[16];
            std::sprintf(length, "%d", at<Dword>(owner, 268));
            append_header(out, "Content-Length", length);
            append_header(out, "Content-Type", post_content_type(owner));
        }
        if (at<char *>(owner, 36))
            append_buffer(out, at<char *>(owner, 36), 0);
        append_buffer(out, "\r\n", 2);
    }
    if (flush_request(out, owner) && at<int>(owner, 84) <= at<int>(owner, 88)) {
        clear_buffer(out);
        at<Dword>(owner, 12) = at<void *>(owner, 252) ? 3 : 4;
        notify_progress(owner, 0, 0);
    }
}

void send_post_body(void *owner) {
    const Dword previous = at<Dword>(owner, 264);
    const Byte state = pump_post_body(owner);
    if (!state) {
        destroy_post_body(owner);
        return;
    }
    if (previous != at<Dword>(owner, 264))
        notify_upload(owner);
    if (state == 1) {
        destroy_post_body(owner);
        at<Dword>(owner, 12) = 4;
        notify_progress(owner, 0, 0);
    }
}

void wait_for_response(void *owner) {
    int readable;
    if (!poll_socket(at<SOCKET>(owner, 64), &readable, nullptr, nullptr)) {
        fail(owner, 5);
        at<int>(owner, 68) = WSAGetLastError();
        return;
    }
    if (readable) {
        at<Dword>(owner, 12) = 5;
        notify_progress(owner, 0, 0);
    }
}

void read_status(void *owner) {
    char incoming[1024];
    int capacity = 1024;
    const int state = receive_request(owner, incoming, &capacity);
    if (state == 3)
        return;
    if (state == 2) {
        fail(owner, 7);
        at<int>(owner, 68) = WSAGetLastError();
        return;
    }
    if (state == 0 && !append_buffer(buffer(owner, 104), incoming, capacity))
        return;
    char *line = at<char *>(owner, 108);
    char *end = std::strstr(line, "\r\n");
    if (!end)
        return;
    *end = 0;
    int major, minor, status, position;
    const int fields = std::sscanf(line, status_format, &major, &minor, &status, &position);
    while (line[position] && std::isspace(static_cast<unsigned char>(line[position])))
        ++position;
    if (fields != 3 || major < 1 || status < 100 || status > 599) {
        fail(owner, 7);
        return;
    }
    at<int>(owner, 172) = major;
    at<int>(owner, 180) = status;
    at<int>(owner, 176) = minor;
    at<int>(owner, 184) = position;
    at<int>(owner, 120) = int(end - line) + 2;
    at<Dword>(owner, 12) = 6;
    notify_progress(owner, 0, 0);
}
} // namespace nfs::network::session::pending::http
