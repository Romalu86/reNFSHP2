#include "gamespy_browser.hpp"
#include "nfs/frontend/network/frontend_network.hpp"
namespace nfs::network::session::pending::browser {
Byte master_buffer[2048]{};
Dword master_bytes{};
char *master_columns[20]{};
int master_column_count{};

Browser *create(const char *game, const char *query_game, const char *key, int queries,
                Callback callback, Dword unused, Dword context) {
    (void)unused;
    auto *owner = static_cast<Browser *>(std::malloc(1464));
    owner->phase = 0;
    owner->servers = http::allocate_dynamic_array(4, 64, free_server_slot);
    owner->strings =
        create_hash(4, 500, 4, hash_interned_string, compare_strings, free_interned_string);
    owner->query_count = queries;
    owner->queries = static_cast<Query *>(std::malloc(Dword(queries) * 28));
    std::memset(owner->queries, 0, Dword(queries) * 28);
    std::strcpy(owner->game, game);
    std::strcpy(owner->key, key);
    std::strcpy(owner->query_game, query_game);
    owner->callback = callback;
    owner->context = context;
    owner->error = "";
    owner->previous_count = 0;
    authentication::pending::unresolved::initialize_sockets();
    return owner;
}

void destroy(Browser *owner) {
    http::free_dynamic_array(owner->servers);
    free_hash(owner->strings);
    std::free(owner->queries);
    std::free(owner);
    frontend::release_sockets();
}

void change_phase(Browser *owner, Dword phase) {
    owner->phase = phase;
    owner->callback(owner, 1, owner->context, nullptr, 0);
}

Server *append_server(Browser *owner, Dword address, std::uint16_t port, Dword type) {
    if (!owner)
        return nullptr;
    Server *server = create_server(address, port, type, owner->strings);
    http::append_dynamic_array(owner->servers, &server);
    return server;
}

int find_server(Browser *owner, Dword address, int port, int count) {
    for (int index = 0; index < count; ++index) {
        auto *server = *static_cast<Server **>(http::dynamic_array_item(owner->servers, index));
        if (server_port(server) == port && server_address(server) == address)
            return index;
    }
    return -1;
}

Dword cancel(Browser *owner) {
    if (owner->phase)
        owner->cancel_requested = 1;
    return 0;
}

Dword clear(Browser *owner) {
    if (owner->phase)
        return 4;
    http::free_dynamic_array(owner->servers);
    owner->servers = http::allocate_dynamic_array(4, 64, free_server_slot);
    free_hash(owner->strings);
    owner->strings =
        create_hash(4, 500, 4, hash_interned_string, compare_strings, free_interned_string);
    owner->previous_count = 0;
    return 0;
}

Dword phase(Browser *owner) {
    return owner->phase;
}

Dword ticks() {
    return GetTickCount();
}

void sleep(int milliseconds) {
    Sleep(milliseconds);
}
} // namespace nfs::network::session::pending::browser
