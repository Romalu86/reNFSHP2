#include "gamespy_http_runtime.hpp"
namespace nfs::network::authentication::pending {
char *duplicate(const char *);
}
namespace nfs::network::session::pending::http {

void read_headers(void *owner) {
    char incoming[4096];
    int capacity = 4096;
    const int state = receive_request(owner, incoming, &capacity);
    if (state == 3)
        return;
    if (state == 1) {
        if (at<int>(owner, 120) == at<int>(owner, 116))
            return;
    } else if (state == 0 && !append_buffer(buffer(owner, 104), incoming, capacity))
        return;
    char *base = at<char *>(owner, 108);
    char *headers = base + at<int>(owner, 120);
    char *end = std::strstr(headers, "\r\n\r\n");
    if (!end)
        end = std::strstr(headers, "\n\n");
    if (!end) {
        if (state == 2) {
            fail(owner, 7);
            at<int>(owner, 68) = WSAGetLastError();
        }
        return;
    }
    const int previous_size = at<int>(owner, 116);
    char *body = end + 4;
    end[2] = 0;
    at<int>(owner, 116) = int(end + 2 - base);
    const int remaining = previous_size - int(body - base);
    const int category = at<int>(owner, 180) / 100;
    if (category == 1) {
        if (!remaining)
            clear_buffer(buffer(owner, 104));
        else {
            std::memmove(base, body, std::size_t(Dword(remaining) + 1));
            at<int>(owner, 116) = remaining;
            at<Dword>(owner, 120) = 0;
        }
        at<Dword>(owner, 12) = 5;
        notify_progress(owner, 0, 0);
        return;
    }
    if (category == 3) {
        if (at<int>(owner, 204) > 10) {
            fail(owner, 11);
            return;
        }
        if (char *location = std::strstr(headers, location_header)) {
            location += 9;
            while (std::isspace(static_cast<unsigned char>(*location)))
                ++location;
            char *tail = location;
            while (*tail && !std::isspace(static_cast<unsigned char>(*tail)))
                ++tail;
            *tail = 0;
            if (*location == '/') {
                const char *host = at<char *>(owner, 20);
                auto *redirect = static_cast<char *>(
                    std::malloc(std::strlen(host) + std::strlen(location) + 14));
                at<char *>(owner, 200) = redirect;
                if (!redirect)
                    fail(owner, 1);
                std::sprintf(redirect, "http://%s:%d%s", host, at<std::uint16_t>(owner, 28),
                             location);
                return;
            }
            at<char *>(owner, 200) = authentication::pending::duplicate(location);
            if (!at<char *>(owner, 200))
                fail(owner, 1);
            return;
        }
    }
    char *length = std::strstr(headers, content_length_header);
    if (length)
        at<int>(owner, 196) = std::atoi(length + 16);
    const bool chunked = std::strstr(headers, chunked_header) != nullptr;
    at<Dword>(owner, 208) = Dword(chunked);
    if (chunked) {
        at<Byte>(owner, 212) = 0;
        at<Dword>(owner, 224) = 0;
        at<Dword>(owner, 228) = 0;
        at<Dword>(owner, 232) = 0;
    }
    const Dword kind = at<Dword>(owner, 8);
    if (kind == 3 || kind == 4) {
        at<Dword>(owner, 188) = 1;
        return;
    }
    at<Dword>(owner, 12) = 7;
    if (length && !at<Dword>(owner, 196)) {
        at<Dword>(owner, 188) = 1;
        return;
    }
    if (remaining > 0)
        consume_chunked(owner, body, remaining);
}

Dword consume_body(void *owner, const char *data, int count) {
    at<Dword>(owner, 192) += Dword(count);
    if (at<Dword>(owner, 192) == at<Dword>(owner, 196) || at<Dword>(owner, 240))
        at<Dword>(owner, 188) = 1;
    Dword callback_size = Dword(count);
    switch (at<Dword>(owner, 8)) {
    case 0:
        if (!append_buffer(buffer(owner, 136), data, count))
            return 0;
        data = at<char *>(owner, 140);
        callback_size = at<Dword>(owner, 148);
        break;
    case 1:
        if (std::fwrite(data, 1, std::size_t(count), at<FILE *>(owner, 40)) !=
            std::size_t(count)) {
            fail(owner, 13);
            return 0;
        }
        break;
    case 2:
        break;
    default:
        callback_size = reinterpret_cast<Dword>(data);
        break;
    }
    notify_progress(owner, reinterpret_cast<Dword>(data), callback_size);
    return 1;
}

Dword consume_chunked(void *owner, const char *data, int count) {
    if (!at<Dword>(owner, 208))
        return consume_body(owner, data, count);
    while (count > 0) {
        const Dword state = at<Dword>(owner, 232);
        if (state == 0) {
            const char *newline = std::strchr(data, '\n');
            int &used = at<int>(owner, 224);
            if (!newline) {
                if (used < 10) {
                    const int copied = count <= 10 - used ? count : 10 - used;
                    std::memcpy(bytes(owner) + 212 + used, data, std::size_t(copied));
                    used += copied;
                    text(owner, 212)[used] = 0;
                }
                return 1;
            }
            int copied = int(newline - data) - 1;
            if (used < 10) {
                if (10 - used < copied)
                    copied = 10 - used;
                std::memcpy(bytes(owner) + 212 + used, data, std::size_t(copied));
                used += copied;
                text(owner, 212)[used] = 0;
            }
            unsigned size;
            const int fields = std::sscanf(text(owner, 212), "%x", &size);
            at<Dword>(owner, 228) = fields == 1 ? size : Dword(-1);
            if (at<Dword>(owner, 228) == Dword(-1)) {
                fail(owner, 7);
                return 0;
            }
            const int consumed = int(newline + 1 - data);
            data = newline + 1;
            count -= consumed;
            at<Dword>(owner, 232) = at<Dword>(owner, 228) == 0 ? 3 : 1;
        } else if (state == 1) {
            const int remaining = at<int>(owner, 228);
            const int consumed = count <= remaining ? count : remaining;
            if (!consume_body(owner, data, consumed))
                return 0;
            data += consumed;
            count -= consumed;
            at<Dword>(owner, 228) -= Dword(consumed);
            if (!at<Dword>(owner, 228))
                at<Dword>(owner, 232) = 2;
        } else if (state == 2) {
            const char *newline = std::strchr(data, '\n');
            if (!newline)
                return 1;
            at<Byte>(owner, 212) = 0;
            at<Dword>(owner, 224) = 0;
            at<Dword>(owner, 228) = 0;
            const int consumed = int(newline + 1 - data);
            data = newline + 1;
            count -= consumed;
            at<Dword>(owner, 232) = 0;
        } else if (state == 3) {
            at<Dword>(owner, 188) = 1;
            return 1;
        } else
            return 0;
    }
    return 1;
}

void read_body(void *owner) {
    char incoming[8192];
    while (!at<Dword>(owner, 188)) {
        int capacity = 8192;
        const int state = receive_request(owner, incoming, &capacity);
        if (state == 3 || state == 1)
            return;
        if (state == 2) {
            at<Dword>(owner, 188) = 1;
            return;
        }
        if (!consume_chunked(owner, incoming, capacity))
            return;
    }
}
} // namespace nfs::network::session::pending::http
