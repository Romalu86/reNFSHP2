#pragma once
#include "nfs/network/session/network_gui.hpp"
namespace nfs::network::players {
struct Visitor {
    const Dword *methods;
};
extern Visitor ready, start, prepare, count, slot_comparer;
extern const Dword ready_methods[], start_methods[], prepare_methods[], count_methods[],
    slot_methods[];
Visitor *construct_ready(Visitor *);
Visitor *construct_start(Visitor *);
Visitor *construct_prepare(Visitor *);
Visitor *construct_count(Visitor *);
Visitor *construct_slot(Visitor *);
void destroy_ready(Visitor *);
void destroy_start(Visitor *);
void destroy_prepare(Visitor *);
void destroy_count(Visitor *);
void destroy_slot(Visitor *);
Dword __fastcall reset_slot(Visitor *, void *, ui::PlayerRecord *, Dword, Dword, Dword);
Dword __fastcall start_vehicle(Visitor *, void *, ui::PlayerRecord *, Dword, Dword, Dword);
Dword __fastcall prepare_slot(Visitor *, void *, ui::PlayerRecord *, Dword, Dword, Dword);
Dword __fastcall count_slot(Visitor *, void *, ui::PlayerRecord *, Dword, Dword, Dword *);
Dword __fastcall compare_slot(Visitor *, void *, const std::int16_t *, const ui::PlayerRecord *);
Dword selected_players(Dword *);
void reset_client_updates(Dword *);
void apply_state(Dword *, const void *);
void notify_game(Dword *, gui::String, int, int);
struct VehicleName {
    const char *name;
    int identifier;
};
namespace pending {
extern const VehicleName vehicle_names[];
}
const char *vehicle_name(int);
void initialize_ready();
void finalize_ready();
void initialize_start();
void finalize_start();
void initialize_prepare();
void finalize_prepare();
void initialize_count();
void finalize_count();
void initialize_slot();
void finalize_slot();
} // namespace nfs::network::players
