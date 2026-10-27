#pragma once
#include "nfs/network/packets/network_packet_handlers.hpp"
namespace nfs::network::rules {
Dword random();
extern Dword random_state[6];
void choice_values(Dword *, Dword, Dword, void *);
void choice_label(Dword *, Dword, const char *);
void choice_enabled(Dword *, Dword, std::uint8_t);
void choice_selected(Dword *, Dword, Dword);
void set_mode(Dword *, int);
void *profile(Dword *, Dword);
void *car_settings(Dword *, Dword);
int catalogue_index(int);
void copy_vehicle(void *, void *);
namespace pending {
void clear_language();
void load_language(Dword);
void reload_lod(Dword *);
} // namespace pending
} // namespace nfs::network::rules
