#pragma once
#include "nfs/network/packets/network_packet_handlers.hpp"
namespace nfs::network::snapshots {
void *allocate(void *);
void insert(void *, const void *, void *, Dword);
void append(void *, const void *, Dword);
Dword pop(void *, void *);
void unpack_position(const void *, float *);
void unpack_velocity(const void *, float *);
void *clear_vector3(void *);
} // namespace nfs::network::snapshots
