#include "gamespy_browser.hpp"
namespace nfs::network::session::pending::browser {
namespace {
char *token_cursor{};
Dword hash_text(const void *entry, Dword buckets) {
    const auto *text =
        reinterpret_cast<const unsigned char *>(*static_cast<const char *const *>(entry));
    Dword value = 0;
    while (*text)
        value = Dword(std::tolower(*text++)) - value * 1664117991u;
    return value % buckets;
}
} // namespace

Dword __cdecl hash_interned_string(const void *entry, Dword buckets) {
    return hash_text(entry, buckets);
}

Dword __cdecl hash_property(const void *entry, Dword buckets) {
    return hash_text(entry, buckets);
}

int __cdecl compare_strings(const void *first, const void *second) {
    return _stricmp(*static_cast<const char *const *>(first),
                    *static_cast<const char *const *>(second));
}

int __cdecl compare_property_keys(const void *first, const void *second) {
    return int(*static_cast<const Dword *>(first) - *static_cast<const Dword *>(second));
}

void __cdecl free_interned_string(void *entry) {
    std::free(*static_cast<char **>(entry));
}

void __cdecl free_server_slot(void *entry) {
    Server *server = *static_cast<Server **>(entry);
    free_hash(server->properties);
    std::free(server);
}

char *duplicate_string(const char *source) {
    if (!source)
        return nullptr;
    auto *copy = static_cast<char *>(std::malloc(std::strlen(source) + 1));
    if (copy)
        std::strcpy(copy, source);
    return copy;
}

char *next_token(char *input, char separator) {
    if (input)
        token_cursor = input;
    char *start = token_cursor;
    while (*token_cursor && *token_cursor != separator)
        ++token_cursor;
    char *result = token_cursor != start ? start : nullptr;
    if (*token_cursor)
        *token_cursor++ = 0;
    return result;
}

char *intern_string(Server *server, const char *text) {
    if (auto *found = static_cast<char **>(find_hash(server->strings, &text)))
        return *found;
    char *copy = duplicate_string(text);
    enter_hash(server->strings, &copy);
    return copy;
}

Dword property_is_retained(const char *name) {
    return std::strcmp(name, "queryid") && std::strcmp(name, "final") ? 1 : 0;
}

Server *create_server(Dword address, std::uint16_t port, Dword type, HashTable *strings) {
    auto *server = static_cast<Server *>(std::malloc(20));
    server->address = address;
    server->port = port;
    server->ping = 9999;
    server->query_type = type;
    const int buckets = type == 0 ? 4 : type == 1 ? 6 : 8;
    server->properties =
        create_hash(8, buckets, type <= 3 ? 2 : 4, hash_property, compare_property_keys, nullptr);
    server->strings = strings;
    return server;
}

Server *parse_server_row(char **columns, int count, char *row, Dword type, HashTable *strings) {
    Server *server = create_server(0, 0, type, strings);
    char *value = next_token(row + 1, '\\');
    for (int index = 0; index < count; ++index) {
        char *name = columns[index];
        if (!value)
            value = const_cast<char *>("");
        if (!std::strcmp(name, "ip")) {
            if (char *port = std::strchr(value, ':')) {
                *port = 0;
                server->address = inet_addr(value);
                server->port = std::uint16_t(std::atoi(port + 1));
            }
        } else if (type == 6 && !std::strcmp(name, "other")) {
            for (char *character = value; *character; ++character)
                if (*character == 1)
                    *character = '\\';
            char *key = next_token(value + 1, '\\');
            while (key) {
                char *item = next_token(nullptr, '\\');
                if (!item)
                    item = const_cast<char *>("");
                if (property_is_retained(key)) {
                    char *entry[]{intern_string(server, key), intern_string(server, item)};
                    enter_hash(server->properties, entry);
                }
                key = next_token(nullptr, '\\');
            }
        } else {
            char *entry[]{intern_string(server, name), intern_string(server, value)};
            enter_hash(server->properties, entry);
        }
        if (index + 1 < count)
            value = next_token(nullptr, '\\');
    }
    return server;
}

void parse_server_properties(Server *server, char *packet) {
    char *name = next_token(packet + 1, '\\');
    while (name) {
        char *value = next_token(nullptr, '\\');
        if (!value)
            value = const_cast<char *>("");
        if (property_is_retained(name)) {
            char *entry[]{intern_string(server, name), intern_string(server, value)};
            enter_hash(server->properties, entry);
        }
        name = next_token(nullptr, '\\');
    }
}

int server_ping(Server *server) {
    return server->ping;
}

char *server_address_text(Server *server) {
    in_addr address;
    address.s_addr = server->address;
    return inet_ntoa(address);
}

Dword server_address(Server *server) {
    return server->address;
}

std::uint16_t server_port(Server *server) {
    return server->port;
}

const char *server_text(Server *server, const char *key, const char *fallback) {
    if (!std::strcmp(key, "hostaddr")) {
        in_addr address;
        address.s_addr = server->address;
        return inet_ntoa(address);
    }
    if (auto *name = static_cast<char **>(find_hash(server->strings, &key)))
        if (auto *entry = static_cast<char **>(find_hash(server->properties, name)))
            return entry[1];
    return fallback;
}

int server_integer(Server *server, const char *key, int fallback) {
    if (!std::strcmp(key, "ping"))
        return server->ping;
    if (auto *name = static_cast<char **>(find_hash(server->strings, &key)))
        if (auto *entry = static_cast<char **>(find_hash(server->properties, name)))
            return std::atoi(entry[1]);
    return fallback;
}
} // namespace nfs::network::session::pending::browser
