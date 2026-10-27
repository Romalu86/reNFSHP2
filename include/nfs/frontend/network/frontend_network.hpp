#pragma once
#include "nfs/network/session/network_runtime.hpp"
namespace nfs::network::frontend {
struct TrackerNode {
    Dword *connection;
    TrackerNode *next, *previous;
};
extern std::uint8_t state[2452];
extern Dword (&default_query)[65];
extern Dword *&current_query;
extern Dword tracker_socket, query_identifier, socket_references;
void disconnect_tracker(Dword *);
void clear_trackers();
void clear_trackers_entry();
void heartbeat(std::uint8_t *);
void heartbeat_exit(std::uint8_t *);
void close_query(std::uint8_t *);
void release_sockets();
void exit();
} // namespace nfs::network::frontend
