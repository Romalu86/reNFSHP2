#include "../../frontend/flow/frontend_multiplayer.hpp"
#include "server_runtime.hpp"
#include "nfs/core/services/service_dependencies.hpp"
namespace nfs::network::lobby {
using game::runtime::at;
using game::runtime::bytes;

void set_server_race(void *record, const char *name) {
    if (!std::strcmp(name, "Single Race"))
        at<Dword>(record, 680) = 0;
    else if (!std::strcmp(name, "Lap Knockout"))
        at<Dword>(record, 680) = 1;
}

const char *race_name(int race) {
    return race == 0 ? "Single Race" : race == 1 ? "Lap Knockout" : "";
}

Dword server_reversed(void *record) {
    const Dword direction = at<Dword>(record, 684);
    return direction == 1 || direction == 3;
}

Dword server_mirrored(void *record) {
    const Dword direction = at<Dword>(record, 684);
    return direction == 3 || direction == 2;
}

void read_server_basic(void *record, sdk::Server **entry) {
    sdk::Server *server = *entry;
    char name[500]{};
    std::strcpy(name, sdk::server_text(server, "hostname", ""));
    if (test_master)
        name[23] = 0;
    std::strcpy(reinterpret_cast<char *>(record), name);
    std::strcpy(reinterpret_cast<char *>(bytes(record) + 324),
                sdk::server_text(server, "mapname", ""));
    at<Dword>(record, 528) = sdk::server_integer(server, "ctid", 0);
    char type[500]{};
    std::strcpy(type, sdk::server_text(server, "gametype", ""));
    if (test_master)
        std::strcpy(type, race_name(0));
    set_server_race(record, type);
    at<Dword>(record, 664) = sdk::server_integer(server, "numplayers", 0);
    at<Dword>(record, 668) = sdk::server_integer(server, "maxplayers", 0);
    std::strcpy(reinterpret_cast<char *>(bytes(record) + 532),
                sdk::server_text(server, "gamemode", ""));
    at<Byte>(record, 693) = sdk::server_integer(server, "password", 0) == 1;
    at<Dword>(record, 676) = sdk::server_ping(server);
}

void read_server_rules(void *record, sdk::Server **entry) {
    sdk::Server *server = *entry;
    application::pending::set_server_car_class(record, sdk::server_integer(server, "res", 0));
    application::pending::set_server_direction(record, sdk::server_integer(server, "dir", 0));
    at<Dword>(record, 672) = sdk::server_integer(server, "laps", 0);
    at<Byte>(record, 692) = sdk::server_integer(server, "ded", 0) == 1;
    at<Byte>(record, 694) = sdk::server_integer(server, "bwor", 0) == 1;
}

const char *recent_address(int index) {
    return services::dependency::recent_address(index);
}

void set_preferred_mode(Dword mode) {
    services::pending::network_kind = mode;
}

void *client_owner() {
    return game::runtime::system(112);
}

void apply_client_name(void *owner, gui::String name) {
    struct Consumed {
        gui::String *value;
        ~Consumed() {
            gui::string_destruct(value);
        }
    } consumed{&name};
    if (gui::string_equal(&name, &lifecycle::construction::unresolved::shared_string))
        return;
    gui::string_copy(reinterpret_cast<gui::String *>(bytes(owner) + 4), &name);
    application::pending::LocalFrontendString narrow(&name);
    gui::string_make_narrow(&narrow.value);
    packets::set_local_name(gui::string_ascii(&narrow.value));
    void *rules = game::runtime::system(72);
    application::pending::LocalFrontendString saved(&narrow.value);
    if (gui::string_make_narrow(&saved.value) &&
        gui::engine::pending::character_at(&saved.value, 0)) {
        std::strncpy(reinterpret_cast<char *>(bytes(rules) + 2514),
                     gui::string_ascii(&saved.value), 25);
        at<Byte>(rules, 2513) = 0;
    } else
        at<Byte>(rules, 2514) = 0;
}
} // namespace nfs::network::lobby
