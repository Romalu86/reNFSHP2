#pragma once
#include "nfs/core/services/service_lifecycle.hpp"
#include "nfs/network/packets/network_packet_handlers.hpp"
#include "nfs/network/packets/network_authentication.hpp"
#include "nfs/network/packets/network_cipher.hpp"
#include "nfs/core/frame/game_step.hpp"
#include "nfs/core/timing/frame_clock.hpp"
#include "nfs/core/timing/timer.hpp"
#include "nfs/gui/events/gui_events.hpp"
#include "../../math/precision/extended_float.hpp"
#include <cstring>
namespace nfs::network::packets::pending {
Dword compression_enabled();
void reset_lobby();
} // namespace nfs::network::packets::pending
namespace nfs::network::session {
using packets::at;
using packets::Byte;
using packets::bytes;
using packets::system;
namespace queue = lifecycle::construction::network::queue;
template <class Result = void, class... Args>
Result method(void *owner, unsigned slot, Args... args) {
    using Call = Result(__thiscall *)(void *, Args...);
    return reinterpret_cast<Call>((*reinterpret_cast<const Dword **>(owner))[slot])(owner, args...);
}
inline Dword *player_queue() {
    return server_queue(network_game());
}
inline Dword *route_queue() {
    return server_queue(server());
}
struct ConnectionOptions {
    alignas(4) Byte data[44];
};
struct TransportLayout {
    Dword methods, active, words_08[7];
    std::uint16_t tcp_port, udp_port;
    Dword tcp, udp, udp_listener, sequence, messages;
    kernel::Thread worker;
    kernel::Signal wake;
    volatile Dword stop;
};
static_assert(sizeof(TransportLayout) == 84 && offsetof(TransportLayout, worker) == 60 &&
              offsetof(TransportLayout, wake) == 72 && offsetof(TransportLayout, stop) == 80);
extern char service_key[8];
extern char version_text[36];
extern Byte internet_query_active;
extern Dword &browser_started;
ConnectionOptions *construct_connection_options(ConnectionOptions *);
void create_transport_connections(void *);
void release_transport_connections(void *);
void __cdecl transport_worker(Dword);
void listen_transport(void *);
bool connect_transport(void *, Dword, std::uint16_t, Dword);
Dword __fastcall transport_event(void *, void *, Dword, void *, int, Dword *);
void copy_client_route(void *, const void *);
Dword begin_client_connection(void *, Dword, std::uint16_t);
void client_route_closed(void *);
void reset_network_session(void *);
void show_connection_error(void *, void *, Dword);
void __cdecl wait_for_connection(void *);
Dword select_spectator_vehicle(void *);
void send_client_car(void *, const void *);
void pack_velocity(const float *, void *);
void pack_position(const float *, void *);
void pack_client_car(const void *, void *);
void send_client_snapshot(void *);
Dword all_players_in_phase(Dword *, std::uint16_t);
void boot_route(void *, Dword);
void begin_server_load(void *);
void schedule_server_start(void *);
void finish_server_start(void *);
void update_server_cars(void *);
Dword has_loading_player(const std::int16_t *);
void initialize_authentication();
void update_authentication();
void update_internet_queries();
void initialize_server_query();
void update_server_query();
void load_registry_key();
void __fastcall receive_transport_packet(void *, void *, Dword *, Byte *, Dword);
void __fastcall connection_opened(void *, void *, Dword *);
void __fastcall connection_accepted(void *, void *, Dword *);
void __fastcall connection_closed(void *, void *, Dword *);
namespace pending {
Dword *comm_options();
bool initialize_comm(const Dword *);
Byte shutdown_comm();
void update_comm();
Dword find_pending_connection(void *, void *);
void *construct_tcp_connection(void *, const ConnectionOptions *, Dword *);
void *construct_udp_connection(void *, const ConnectionOptions *, Dword *);
void initialize_key_service(Dword, Dword);
void update_key_service();
void send_authentication_challenge(Dword *);
void mark_authentication_players(Dword *);
void release_unmarked_authentication();
void browser_think();
Dword initialize_query(Dword, Dword, Dword, const char *, const char *, void *, void *, void *, void *,
                      Dword);
void query_heartbeat(void *);
void query_lan_think(void *);
void __cdecl server_query_parameters(char *, int, Dword);
void __cdecl server_query_players(char *, int, Dword);
void __cdecl server_query_rules(char *, int, Dword);
void __cdecl server_query_echo(char *, int, Dword);
extern char master_host[];
void transport_statistics(Byte, Dword, Byte);
} // namespace pending
} // namespace nfs::network::session
