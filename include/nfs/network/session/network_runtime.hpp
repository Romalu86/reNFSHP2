#pragma once
#include "nfs/network/transport/thread_queue.hpp"
namespace nfs::network {
using kernel::Dword;
template <class T> T &field(void *object, unsigned offset) {
    return *reinterpret_cast<T *>(static_cast<std::uint8_t *>(object) + offset);
}
extern Dword *transport;
Dword *server();
Dword *client();
Dword *network_game();
Dword *server_queue(Dword *);
Dword session_active();
Dword render_mode();
Dword transport_time();
Dword protocol(void *);
void *address(void *);
Dword pending_packets(void *);
Dword swap_error(void *, Dword);
Dword read_error(void *);
Dword transport_matches_route(void *, void *);
void reset_server(void *);
void set_game_phase(Dword *, std::int16_t);
Dword __fastcall remove_server_client(Dword *, void *, Dword);
void release_player_slot(Dword *, std::uint16_t);
void mark_player_left(Dword *, int, std::uint8_t);
void reset_race_progress(Dword *);
void broadcast(Dword *, const void *, Dword, Dword, const void *);
void leave_game(Dword *);
void close_route(Dword *, void *);
extern Dword *comm_api;
Dword destroy_comm_packet(Dword *, Dword *);
void destroy_packet(Dword *);
extern Dword &graphics_enabled;
extern Dword saved_graphics_enabled;
extern HANDLE &instance_mutex;
bool has_instance_mutex();
bool acquire_instance_mutex();
void disconnect_route(void *);
void disconnect_client(Dword *, Dword);
void send_packet(Dword, Dword, Dword, Dword);
void forward(Dword *, Dword, Dword, Dword);
Dword send_route(Dword *, void *, const std::uint8_t *, Dword, std::uint8_t);
void __fastcall dispatch_server_packet(Dword *, void *, Dword, Dword, Dword);
void __fastcall dispatch_client_packet(Dword *, void *, Dword, Dword, Dword);
} // namespace nfs::network
