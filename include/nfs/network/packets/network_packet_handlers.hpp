#pragma once
#include "nfs/network/packets/network_bitstream.hpp"
#include "nfs/network/session/network_player_state.hpp"
#include "nfs/resources/archives/archive_cache.hpp"
namespace nfs::network {
void __fastcall dispatch_client_packet(Dword *, void *, Dword, Dword, Dword);
void __fastcall dispatch_server_packet(Dword *, void *, Dword, Dword, Dword);
namespace packets {
extern char local_player_name[80];
void set_local_name(const char *);
const char *get_local_name();
struct ScopedString {
    gui::String value;
    ScopedString() {
        gui::string_construct(&value);
    }
    ~ScopedString() {
        gui::string_destruct(&value);
    }
    ScopedString(const ScopedString &) = delete;
    ScopedString &operator=(const ScopedString &) = delete;
};
using Byte = std::uint8_t;
namespace queue = lifecycle::construction::network::queue;
template <class T> T &at(void *object, unsigned offset) {
    return field<T>(object, offset);
}
inline Dword address_of(const void *object) {
    return reinterpret_cast<Dword>(object);
}
inline Byte *bytes(void *object) {
    return static_cast<Byte *>(object);
}
inline Dword *system(unsigned offset) {
    return reinterpret_cast<Dword *>(lifecycle::manager->slots[offset / 4]);
}
inline Dword *player_queue() {
    return server_queue(network_game());
}
inline Dword *route_queue(Dword *owner) {
    return server_queue(owner);
}
inline Dword *visitor(Dword *owner) {
    return owner
               ? reinterpret_cast<Dword *>(bytes(owner) + reinterpret_cast<Dword *>(owner[1])[2] + 4)
               : nullptr;
}
inline Dword *race() {
    return at<Dword *>(system(92), 4);
}
inline Dword *vehicle(int slot) {
    return at<Dword *>(system(68), 20 + Dword(slot) * 4);
}
inline void send(void *route, void *packet, Dword size, Dword flags = 0) {
    send_packet(address_of(route), address_of(packet), size, flags);
}
inline bool copy_player(const void *key, ui::PlayerRecord *player, Dword *comparer = nullptr) {
    return queue::copy_matching(player_queue(), address_of(key), address_of(player), comparer) != 0;
}
inline void replace_player(const void *key, ui::PlayerRecord *player) {
    queue::replace(player_queue(), address_of(key), address_of(player), nullptr);
}
inline void notice(const gui::String *text, int sender = -1, int prefix = 0) {
    if (auto *frontend = at<Dword *>(network_game(), 92))
        ui::notice(frontend, nullptr, text, std::int16_t(sender), prefix, 0, 0);
}
void set_server_name(Dword *, const char *);
void set_local_name(const char *);
Dword string_character(const gui::String *, int);
const char *vehicle_resource(Dword *, int);
int vehicle_class(Dword *, int);
Dword rule_value(Dword *, int);
void clear_server_settings(void *);
void export_server_settings(Dword *, void *);
void import_server_settings(Dword *, void *);
void add_player(Dword *, std::uint16_t, const char *, std::uint16_t);
void rename_player(Dword *, void *);
void formatted_notice(void *);
void player_update(void *);
Dword track_reference(int);
Dword player_left(Dword *, int);
bool find_player_route(Dword *, Dword, void *);
void broadcast_chat(Dword *, const char *, void *);
void dispatch_vote(Dword *, void *);
namespace pending {
void send_join(Dword *);
void receive_car_state(Dword *, void *);
void receive_position(Dword *, void *);
void receive_client_car(Dword *, void *, void *);
void answer_authentication_challenge(void *);
void receive_authentication(int, void *);
bool validate_key(const char *, Dword *, char *);
Dword *authentication_record(int);
void *find_race_vehicle(int);
void read_race_player(void *, void *);
void write_race_player(void *, void *);
void send_race_players(Dword *, void *);
void reconnect(Dword *, Dword, std::uint16_t, Dword);
bool is_booted(const char *);
void record_boot(const char *, const char *, const char *);
extern Dword name_comparer;
void set_game_rule(Dword *, int, Dword);
void set_rule_variant(Dword *, int, Dword);
void select_track(Dword *, Dword);
void generate_client_key(char *, int);
void relay_car_route(void *, void *);
extern std::uint16_t reconnect_port;
} // namespace pending
} // namespace packets
} // namespace nfs::network
