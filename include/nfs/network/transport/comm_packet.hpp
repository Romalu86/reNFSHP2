#pragma once
#include "nfs/network/transport/network_mailbox.hpp"
namespace nfs::network::comm {
struct Packet;
struct Buffer {
    std::uint8_t *data;
    Packet *owner;
    int capacity, header_bytes, payload_bytes, cursor;
};
struct Packet {
    const gui::Destroy *methods;
    Dword kind;
    Buffer buffer;
    std::uint8_t endpoint[276];
    int references, mode;
    Dword status;
};
static_assert(sizeof(Buffer) == 24 && sizeof(Packet) == 320 &&
              offsetof(Packet, endpoint) == 32 && offsetof(Packet, status) == 316);
extern const gui::Destroy packet_methods[];
Dword initialize_packet(Packet *);
Packet *construct_packet(Packet *, const void *, int, int);
Packet *construct_packet_copy(Packet *, Packet *);
Packet *copy_packet(Packet *, Packet *);
Packet *share_packet(Packet *, Packet *);
void destroy_packet(Packet *);
Dword release_buffer(Packet *);
Dword allocate_buffer(Packet *, int, int);
Buffer *packet_buffer(Packet *);
bool copy_buffer(Packet *, const Buffer *);
Dword bind_buffer(Packet *, void *, int, int, Packet *);
void set_mode(Packet *, int);
int mode(Packet *);
void clear_packet(Packet *);
void reset_cursor(Packet *);
void *packet_data(Packet *);
int available_bytes(Packet *);
int seek_packet(Packet *, int, int);
bool set_extent(Packet *, int);
int write_packet(Packet *, const void *, int);
int read_packet(Packet *, void *, int);
void *packet_endpoint(Packet *);
bool set_endpoint(Packet *, const void *);
int reference_count(Packet *);
int add_reference(Packet *);
int release_reference(Packet *);
void set_status(Packet *, Dword);
Dword status(Packet *);
Dword send_via_filter(Dword *, Packet *);
Dword send_via_driver(Dword *, Packet *);
Dword header_bytes(Dword *);
std::uint8_t send_immediate(Dword *, Dword, const void *, Dword, const char *, std::uint16_t);
} // namespace nfs::network::comm
