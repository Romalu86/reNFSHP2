#pragma once
#include "comm_connection_runtime.hpp"
#include "nfs/core/timing/timer.hpp"
namespace nfs::network::session::pending {
extern const Dword filter_methods[], listener_filter_methods[], packetizer_filter_methods[],
    reliable_filter_methods[];
void *construct_filter(void *, Dword);
void destroy_filter(void *);
Dword __fastcall filter_kind(void *, void *);
Dword __fastcall filter_mode(void *, void *);
Dword __fastcall set_filter_mode(void *, void *, Dword);
void __fastcall enable_filter(void *, void *);
void __fastcall disable_filter(void *, void *);
Byte __fastcall clear_filter(void *, void *);
void *__fastcall filter_connection(void *, void *);
void *__fastcall set_filter_connection(void *, void *, void *);
void *__fastcall filter_previous(void *, void *);
void *__fastcall set_filter_previous(void *, void *, void *);
void *__fastcall filter_next(void *, void *);
void *__fastcall set_filter_next(void *, void *, void *);
Dword __fastcall filter_header_bytes(void *, void *);
Dword __fastcall filter_total_header_bytes(void *, void *);
Byte __fastcall filter_send(void *, void *, comm::Packet *);
Dword __fastcall filter_receive(void *, void *, comm::Packet *);
void encode_comm_header(void *, const char *);
void decode_comm_header(void *, const char *);
Dword comm_milliseconds();
Dword comm_timestamp();
struct ReliableEntry {
    Dword kind;
    comm::Packet packet;
    ReliableEntry *previous, *next;
    Dword identifier, sent_at, delay, interval, slot, retries;
    Byte available, padding[3];
};
struct ReliablePool {
    const Dword *methods;
    Dword kind, capacity;
    ReliableEntry *entries;
    Dword stride;
    Byte *data;
    comm::LockObject *lock;
    Dword epoch, write, read;
};
static_assert(sizeof(ReliableEntry) == 360 && offsetof(ReliableEntry, packet) == 4 &&
              offsetof(ReliableEntry, identifier) == 332 && sizeof(ReliablePool) == 40);
ReliablePool *construct_reliable_pool(ReliablePool *, Dword, Dword);
Dword reset_reliable_pool(ReliablePool *, Dword, Dword);
ReliableEntry *enqueue_reliable_packet(ReliablePool *, comm::Packet *);
ReliableEntry *store_reliable_packet(ReliablePool *, Dword, const void *, Dword);
Dword acknowledge_reliable_packets(ReliablePool *, Dword);
Byte reliable_pool_empty(ReliablePool *);
void clear_reliable_entry(ReliableEntry *);
} // namespace nfs::network::session::pending
