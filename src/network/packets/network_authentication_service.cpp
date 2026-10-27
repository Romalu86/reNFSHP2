#include "nfs/network/packets/network_authentication.hpp"
#include "nfs/frontend/network/frontend_network.hpp"
#include <winsock.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include "../session/network_lifecycle_runtime.hpp"
namespace nfs::network::authentication::pending {
namespace {
struct Request {
    Dword identifier;
    char key[36];
    Dword salt, ip, started, retries, status;
    void *context;
    void(__cdecl *callback)(Dword, int, const char *, void *);
    char *error;
    void *datagram;
    Dword bytes;
};
struct Node {
    Request *request;
    Node *next, *previous;
};
static_assert(sizeof(Request) == 80 && sizeof(Node) == 12);
} // namespace
// Saved globals: INVALID_SOCKET in .data; remaining fields are zero BSS.
namespace unresolved {
SOCKET &authentication_socket = frontend::tracker_socket;
sockaddr &authentication_endpoint = *reinterpret_cast<sockaddr *>(frontend::state + 2408);
char (&xor_key)[12] = *reinterpret_cast<char (*)[12]>(frontend::state + 2424);
Node &authentication_request_sentinel = *reinterpret_cast<Node *>(frontend::state + 2436);
Dword &authentication_product_id = *reinterpret_cast<Dword *>(frontend::state + 2448);
Dword &socket_initializations = frontend::socket_references;

void initialize_sockets() {
    WSADATA data;
    WSAStartup(257, &data);
    ++socket_initializations;
}

Dword resolve_endpoint(const char *host, Dword port, sockaddr *output, void *resolved) {
    auto *address = reinterpret_cast<sockaddr_in *>(output);
    std::memset(output, 0, 16);
    address->sin_family = AF_INET;
    address->sin_port = htons(static_cast<u_short>(port));
    address->sin_addr.s_addr = host ? inet_addr(host) : 0;
    void *result = output;
    if (address->sin_addr.s_addr == INADDR_NONE && std::strcmp(host, "255.255.255.255")) {
        auto *entry = gethostbyname(host);
        if (!entry)
            return 0;
        std::memcpy(&address->sin_addr.s_addr, entry->h_addr_list[0], 4);
        result = entry;
    }
    if (resolved)
        *static_cast<void **>(resolved) = result;
    return 1;
}
} // namespace unresolved
char authentication_host[64]{};
std::uint16_t bound_port{};
 
Dword initialize_service(Dword product) {
    unresolved::initialize_sockets();
    unresolved::authentication_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (unresolved::authentication_socket == INVALID_SOCKET)
        return 4294967295u;
    sockaddr local;
    unresolved::resolve_endpoint(nullptr, 0, &local, nullptr);
    if (bind(unresolved::authentication_socket, &local, 16) == SOCKET_ERROR)
        return 4294967295u;
    int bytes = 16;
    if (getsockname(unresolved::authentication_socket, &local, &bytes) == SOCKET_ERROR)
        return 4294967295u;
    std::memcpy(&bound_port, local.sa_data, 2);
    if (!authentication_host[0])
        std::strcpy(authentication_host, "key.gamespy.com");
    unresolved::resolve_endpoint(authentication_host, 29910, &unresolved::authentication_endpoint,
                                 nullptr);
    unresolved::authentication_product_id = product;
    unresolved::authentication_request_sentinel = {};
    std::srand(GetTickCount());
    std::strcpy(unresolved::xor_key, "gamespy");
    return 0;
}

void xor_datagram(void *data, int bytes) {
    auto *output = static_cast<unsigned char *>(data);
    const char *mask = unresolved::xor_key;
    for (int index = 0; index < bytes; ++index) {
        output[index] ^= *mask++;
        if (!*mask)
            mask = unresolved::xor_key;
    }
}

char *duplicate(const char *text) {
    if (!text)
        return nullptr;
    auto *copy = static_cast<char *>(std::malloc(std::strlen(text) + 1));
    if (copy)
        std::strcpy(copy, text);
    return copy;
}
// Linked request allocation and UDP datagram encoding.
void submit_authentication_challenge(Dword identifier, Dword ip, const char *challenge, const char *response,
                     void(__cdecl *callback)(Dword, int, const char *, void *), void *context) {
    char key[33];
    std::strncpy(key, response, 32);
    key[32] = 0;
    char *error = std::strlen(response) < 72 ? duplicate("Bad CD Key") : nullptr;
    for (Node *node = unresolved::authentication_request_sentinel.next; node; node = node->next)
        if (!std::strcmp(key, node->request->key)) {
            error = duplicate("CD Key in use");
            break;
        }
    auto *request = static_cast<Request *>(std::malloc(80));
    request->identifier = identifier;
    request->ip = ip;
    request->callback = callback;
    request->context = context;
    request->error = nullptr;
    request->datagram = nullptr;
    std::strcpy(request->key, key);
    auto *node = static_cast<Node *>(std::malloc(12));
    node->request = request;
    Node *tail = &unresolved::authentication_request_sentinel;
    while (tail->next)
        tail = tail->next;
    tail->next = node;
    node->previous = tail;
    node->next = nullptr;
    if (error) {
        request->status = 2;
        request->error = error;
        return;
    }
    request->status = 0;
    request->salt = (GetTickCount() ^ Dword(std::rand())) & 16383;
    request->started = GetTickCount();
    request->retries = 1;
    char packet[512];
    const int count =
        std::sprintf(packet, "\\auth\\\\pid\\%d\\ch\\%s\\resp\\%s\\ip\\%d\\skey\\%d",
                     unresolved::authentication_product_id, challenge, response, request->ip, request->salt);
    xor_datagram(packet, count);
    sendto(unresolved::authentication_socket, packet, count, 0, &unresolved::authentication_endpoint, 16);
    request->datagram = std::malloc(count);
    std::memmove(request->datagram, packet, count);
    request->bytes = count;
}
} // namespace nfs::network::authentication::pending

namespace nfs::network::session::pending {
namespace keys = authentication::pending;
extern Dword default_query_context[];
void __cdecl receive_key_reply(char *, Dword, sockaddr *);
namespace {
char (&key_reply_buffer)[1024] =
    *reinterpret_cast<char (*)[1024]>(frontend::state + 1384);
} // namespace

void initialize_key_service(Dword query, Dword product) {
    auto *context = query ? reinterpret_cast<Dword *>(query) : default_query_context;
    keys::unresolved::authentication_socket = context[0];
    std::memset(&keys::unresolved::authentication_endpoint, 0, 16);
    keys::bound_port = 65535;
    context[63] = reinterpret_cast<Dword>(receive_key_reply);
    auto *endpoint = reinterpret_cast<sockaddr_in *>(&keys::unresolved::authentication_endpoint);
    endpoint->sin_family = 2;
    endpoint->sin_port = htons(29910);
    endpoint->sin_addr.s_addr = context[60];
    keys::unresolved::authentication_product_id = product;
    keys::unresolved::authentication_request_sentinel = {};
    std::srand(GetTickCount());
    std::strcpy(keys::unresolved::xor_key, "gamespy");
}

static void send_key_disconnect(keys::Request *request) {
    char datagram[512];
    const int size = std::sprintf(datagram, "\\disc\\\\pid\\%d\\cd\\%s\\ip\\%d",
                                  keys::unresolved::authentication_product_id, request->key, request->ip);
    keys::xor_datagram(datagram, size);
    sendto(keys::unresolved::authentication_socket, datagram, size, 0,
           &keys::unresolved::authentication_endpoint, 16);
}

void remove_key_client(Dword identifier) {
    auto *node = keys::unresolved::authentication_request_sentinel.next;
    while (node && node->request->identifier != identifier)
        node = node->next;
    if (!node)
        return;
    auto *request = node->request;
    send_key_disconnect(request);
    node->previous->next = node->next;
    if (node->next)
        node->next->previous = node->previous;
    if (request->datagram)
        std::free(request->datagram);
    std::free(request);
    std::free(node);
}

void update_key_service() {
    timeval wait{0, 0};
    int endpoint_size = 16;
    sockaddr endpoint;
    fd_set reads;
    reads.fd_count = 1;
    reads.fd_array[0] = keys::unresolved::authentication_socket;
    if (keys::bound_port != Dword(-1)) {
        for (;;) {
            const int ready = select(64, &reads, nullptr, nullptr, &wait);
            if (ready == SOCKET_ERROR || !ready)
                break;
            const int size = recvfrom(keys::unresolved::authentication_socket, key_reply_buffer, 1023, 0,
                                      &endpoint, &endpoint_size);
            if (size != SOCKET_ERROR) {
                key_reply_buffer[size] = 0;
                receive_key_reply(key_reply_buffer, Dword(size), &endpoint);
            }
        }
    }
    for (auto *node = keys::unresolved::authentication_request_sentinel.next; node; node = node->next) {
        auto *request = node->request;
        const Dword status = request->status;
        bool accepted = status == 1;
        if (!status) {
            if (request->started + 2000 > GetTickCount())
                continue;
            if (static_cast<int>(request->retries) > 2)
                accepted = true;
            else {
                request->started = GetTickCount();
                ++request->retries;
                sendto(keys::unresolved::authentication_socket, static_cast<char *>(request->datagram),
                       int(request->bytes), 0, &keys::unresolved::authentication_endpoint, 16);
                continue;
            }
        }
        if (accepted) {
            request->callback(request->identifier, 1,
                              status == 1 ? "Validated" : "Validation Timeout", request->context);
            request->status = 3;
            std::free(request->datagram);
            request->datagram = nullptr;
        } else if (status == 2) {
            auto *previous = node->previous;
            node->previous->next = node->next;
            if (node->next)
                node->next->previous = node->previous;
            char *error = request->error;
            request->callback(request->identifier, 0, error ? error : "", request->context);
            std::free(request->datagram);
            if (error)
                std::free(error);
            std::free(request);
            std::free(node);
            node = previous;
        }
    }
}
namespace {
char (&key_values)[2][256] =
    *reinterpret_cast<char (*)[2][256]>(frontend::state + 864);
Dword &key_value_slot = *reinterpret_cast<Dword *>(frontend::state + 1376);
} // namespace

static const char *key_value(const char *packet, const char *key) {
    key_value_slot ^= 1;
    char pattern[258] = "\\";
    std::strcat(pattern, key);
    std::strcat(pattern, "\\");
    const char *value = std::strstr(packet, pattern);
    if (!value)
        return "";
    value += std::strlen(pattern);
    char *output = key_values[key_value_slot];
    unsigned size = 0;
    while (*value && *value != '\\' && size < 200)
        output[size++] = *value++;
    output[size] = 0;
    return output;
}
// The saved stack bytes overwrite "skey" before its first use.
static void accept_key_reply(const char *packet, Dword accepted) {
    char error_key[8] = "errmsg";
    const int salt = std::atoi(key_value(packet, error_key));
    std::memcpy(error_key, &salt, sizeof(salt));
    char key[33];
    std::strncpy(key, key_value(packet, "cd"), 32);
    key[32] = 0;
    auto *node = keys::unresolved::authentication_request_sentinel.next;
    while (node && std::strcmp(key, node->request->key))
        node = node->next;
    if (!node || !node->request)
        return;
    auto *request = node->request;
    if (request->salt != Dword(salt) || request->status == 3)
        return;
    if (accepted) {
        request->status = 1;
        return;
    }
    request->status = 2;
    request->error = keys::duplicate(key_value(packet, error_key));
}

void __cdecl receive_key_reply(char *packet, Dword bytes_received, sockaddr *sender) {
    keys::xor_datagram(packet, int(bytes_received));
    char command[32];
    command[0] = 0;
    if (*packet != '\\')
        return;
    const int delimiter =
        int(reinterpret_cast<Dword>(std::strchr(packet + 1, '\\')) - reinterpret_cast<Dword>(packet));
    if (delimiter < 33) {
        std::strncpy(command, packet + 1, std::size_t(delimiter - 1));
        command[delimiter - 1] = 0;
    }
    if (!*command)
        return;
    if (!std::strcmp(command, "uok")) {
        accept_key_reply(packet, 1);
        return;
    }
    if (!std::strcmp(command, "unok")) {
        accept_key_reply(packet, 0);
        return;
    }
    if (!std::strcmp(command, "ison")) {
        const int salt = std::atoi(key_value(packet, "skey"));
        const char *key = key_value(packet, "cd");
        auto *node = keys::unresolved::authentication_request_sentinel.next;
        while (node && std::strcmp(key, node->request->key))
            node = node->next;
        const bool admitted = node && node->request && node->request->status == 3;
        char reply[64];
        const int length =
            std::sprintf(reply, admitted ? "\\uon\\\\skey\\%d" : "\\uoff\\\\skey\\%d", salt);
        keys::xor_datagram(reply, length);
        sendto(keys::unresolved::authentication_socket, reply, length, 0, sender, 16);
        return;
    }
    if (!std::strcmp(command, "ucount")) {
        int count = 0;
        for (auto *node = keys::unresolved::authentication_request_sentinel.next; node; node = node->next)
            ++count;
        char reply[64];
        const int length = std::sprintf(reply, "\\ucount\\%d", count);
        keys::xor_datagram(reply, length);
        sendto(keys::unresolved::authentication_socket, reply, length, 0, sender, 16);
    }
}
} // namespace nfs::network::session::pending
