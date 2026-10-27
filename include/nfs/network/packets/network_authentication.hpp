#pragma once
#include "nfs/network/packets/network_packet_handlers.hpp"
namespace nfs::network::authentication {
extern char client_key[20];
bool valid_key(const char *);
void set_client_key(const char *);
void set_status(Dword *, Dword);
void set_error(Dword *, const char *);
void set_key(Dword *, const char *);
void send_response(Dword *, const char *);
Dword player_address(Dword *, int);
void reject_player(Dword *, Dword, Dword);
void make_response(const char *, const char *, char *);
void __cdecl complete(Dword, int, const char *, void *);
void append_record(Dword *);
void release_record(int);
void unmark_records();
namespace pending {
void submit_authentication_challenge(Dword, Dword, const char *, const char *,
                     void(__cdecl *)(Dword, int, const char *, void *), void *);
}
} // namespace nfs::network::authentication
namespace nfs::network::packets::pending {
extern Dword *authentication_records[20];
extern int session_kind;
} // namespace nfs::network::packets::pending
