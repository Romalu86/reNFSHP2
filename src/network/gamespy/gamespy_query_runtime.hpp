#pragma once
#include "../session/network_lifecycle_runtime.hpp"
#include <winsock.h>
#include <cstdio>
#include <cstdlib>
namespace nfs::network::session::pending {
struct QueryLayout {
    SOCKET receive_socket, heartbeat_socket;
    char game[64], key[128];
    void *basic, *information, *rules, *players;
    Dword last_heartbeat, query_identifier, packet_identifier, port;
    Byte retry;
    Byte padding[3];
    sockaddr master;
    void(__cdecl *key_reply)(char *, Dword, sockaddr *);
    Dword context;
};
static_assert(sizeof(QueryLayout) == 260 && offsetof(QueryLayout, basic) == 200 &&
              offsetof(QueryLayout, master) == 236 && offsetof(QueryLayout, context) == 256);
extern Dword default_query_context[65];
extern Dword *query_context;
void initialize_query_context(Dword *, Dword, SOCKET, SOCKET, const char *, const char *, void *,
                              void *, void *, void *, Dword);
void poll_query(void *);
void dispatch_query(void *, char *, Dword, sockaddr *);
void append_query(void *, sockaddr *, char *, char *);
void flush_query(void *, sockaddr *, char *);
void query_information(void *, sockaddr *, char *);
void query_rules(void *, sockaddr *, char *);
void query_players(void *, sockaddr *, char *);
void query_echo(void *, sockaddr *, char *, const char *);
void finish_query(void *, sockaddr *, char *, const char *);
void encode_query_validation(const Byte *, int, char *);
void encrypt_query_validation(const Byte *, int, Byte *, int);
inline char *query_text(void *owner, unsigned offset) {
    return reinterpret_cast<char *>(bytes(owner) + offset);
}
} // namespace nfs::network::session::pending
