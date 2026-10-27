#pragma once
#include "../session/network_lifecycle_runtime.hpp"
#include "nfs/network/transport/comm_packet.hpp"
#include <winsock.h>
#include <cstdlib>
extern "C" void __cdecl _assert(const char *, const char *, unsigned);
namespace nfs::network::session::pending {
struct ConnectionCore {
    const Dword *methods;
    Dword kind, references, type, protocol_kind, role;
    ConnectionOptions options;
    Dword state;
    std::uint16_t port, padding;
    void *callback;
    comm::LockObject *lock;
    void *driver, *filter;
    comm::Mailbox *outgoing;
    comm::LockObject *outgoing_lock;
    Dword unwritten_64;
    ConnectionCore *previous, *next;
};
struct InetConnection {
    ConnectionCore core;
    char host[256];
    SOCKET socket;
    sockaddr remote, local;
};
struct InetDriver {
    const Dword *methods;
    Dword kind, references, identifier, state;
    void *sets, *worker;
    comm::LockObject *lock;
    void *receive_batch[64];
    comm::LockObject *socket_lock;
};
static_assert(sizeof(ConnectionCore) == 112 && offsetof(ConnectionCore, options) == 24 &&
              offsetof(ConnectionCore, state) == 68 &&
              offsetof(ConnectionCore, outgoing) == 92 &&
              offsetof(ConnectionCore, next) == 108 && sizeof(InetConnection) == 404 &&
              offsetof(InetConnection, socket) == 368 &&
              offsetof(InetConnection, remote) == 372 && sizeof(InetDriver) == 292 &&
              offsetof(InetDriver, receive_batch) == 32);
extern Dword *connections;
extern const Dword inet_connection_methods[], tcp_connection_methods[], udp_connection_methods[];
extern const Dword connection_methods[], comm_object_methods[];
extern const Dword inet_driver_methods[], connection_set_methods[], comm_worker_methods[];
comm::LockObject *allocate_comm_lock();
Dword *registered_driver(int);
Dword register_driver(void *);
Dword unregister_driver(void *);
Dword register_connection(void *);
Dword unregister_connection(void *);
bool contains_connection(void *, void *, Dword *);
void *construct_connection_set(void *);
void link_connection_set(void *, void *);
bool add_connection_to_set(void *, void *);
bool remove_connection_from_set(void *, void *);
bool driver_add_connection(void *, void *);
Byte driver_remove_connection(void *, void *);
void *get_inet_driver();
SOCKET create_connection_socket(void *);
Dword create_inet_socket(void *);
Dword close_inet_socket(SOCKET);
int socket_error(void *);
char *format_inet_address(Dword);
Dword local_inet_address();
Dword configure_inet_address(void *);
Dword *construct_inet_connection(Dword *, Dword, const ConnectionOptions *, Dword *);
Dword *copy_inet_connection(Dword *, void *);
Dword reset_connection(void *, void *);
void destroy_inet_connection(void *);
void destroy_connection(void *);
Dword close_connection(void *);
Dword link_connection_filters(void *, void *);
void *bottom_connection_filter(void *);
Dword link_filter_modules(void *, void *);
Dword unlink_connection_filter(void *, void *);
Dword clear_connection_filters(void *);
void *construct_packetizer_filter(void *);
void *construct_reliable_filter(void *, Dword, std::uint16_t);
void *construct_listener_filter(void *, void *, Dword);
void *__fastcall clone_inet_connection(void *, void *);
void *__fastcall clone_udp_connection(void *, void *);
void *__fastcall delete_inet_connection(void *, void *, Dword);
void *__fastcall delete_tcp_connection(void *, void *, Dword);
void *__fastcall delete_udp_connection(void *, void *, Dword);
Dword __fastcall connection_event(void *, void *, Dword, Dword, Dword, void *);
Dword __fastcall tcp_connection_event(void *, void *, Dword, Dword, Dword, void *);
Dword __fastcall udp_connection_event(void *, void *, Dword, Dword, Dword, void *);
char *__fastcall connection_address_text(void *, void *, Dword);
Dword __fastcall connection_address_number(void *, void *, const char *);
Dword __fastcall close_connection_method(void *, void *);
Dword __fastcall update_connection(void *, void *);
Dword __fastcall send_connection(void *, void *, const void *, Dword);
Dword __fastcall listen_connection(void *, void *, const char *, std::uint16_t);
Dword __fastcall connect_connection(void *, void *, const char *, std::uint16_t, std::uint16_t);
Dword send_connection_data(void *, const void *, Dword, const char *, std::uint16_t);
Dword deliver_connection_packet(void *, comm::Packet *);
int drain_connection_queue(void *);
Dword set_connection_endpoint(void *, const sockaddr *);
Dword set_connection_destination(void *, const char *, std::uint16_t);
Dword set_connection_local_endpoint(void *, const sockaddr *);
std::uint16_t connection_remote_port(void *);
char *connection_remote_host(void *);
void construct_driver(void *, Dword);
void destroy_driver(void *);
void *construct_comm_worker(void *, void(__cdecl *)(void *), void *, Dword, Dword);
Dword start_driver_worker(void *, void(__cdecl *)(void *), void *, Dword, Dword);
void __cdecl update_driver_worker(void *);
void __cdecl comm_worker_entry(Dword);
Dword __fastcall initialize_driver(void *, void *);
Dword __fastcall stop_driver(void *, void *);
void __fastcall update_driver(void *, void *, Dword);
Dword __fastcall open_inet_connection(void *, void *, void *);
Dword __fastcall close_inet_connection(void *, void *, void *);
Dword __fastcall driver_option_leaf(void *, void *);
Byte __fastcall can_send_inet(void *, void *, void *);
int __fastcall send_inet_packet(void *, void *, void *, comm::Packet *);
void __fastcall poll_inet_set(void *, void *, void *);
void __fastcall update_connection_set(void *, void *, void *);
Dword __fastcall cleanup_inet_driver(void *, void *);
void *__fastcall delete_inet_driver(void *, void *, Dword);
Byte poll_inet_sockets(void *, fd_set *, void **, int, timeval *);
int receive_tcp_socket(void *, void *);
int receive_udp_socket(void *);
inline void enter_comm_lock(void *lock) {
    kernel::lock_enter(&at<comm::LockObject>(lock, 0).lock);
}
inline void leave_comm_lock(void *lock) {
    kernel::lock_leave(&at<comm::LockObject>(lock, 0).lock);
}
} // namespace nfs::network::session::pending
