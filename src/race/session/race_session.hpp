#pragma once
#include "race_runtime.hpp"
#include "nfs/network/session/network_rules.hpp"
#include "nfs/core/services/service_lifecycle.hpp"
#include "../../math/precision/extended_float.hpp"
namespace nfs::application::pending {
struct GridCarPool {
    int cars[8];
    Dword totals[8], humans[8], racers[8], cops[8], special_cops[8];
};
static_assert(sizeof(GridCarPool) == 192);
extern Dword event_car_order[24], event_car_order_count;
inline void *grid_entry(void *rules, Dword slot) {
    return slot < 24 ? bytes(rules) + 248 + slot * 84 : nullptr;
}
int last_configured_lap(void *);
inline int final_lap() {
    return last_configured_lap(system(72));
}
void reset_event_progress(void *);
void advance_event_progress(void *);
Dword event_stage(void *);
bool event_complete_progress(void *);
Dword event_active(void *);
Dword event_points(void *, Dword);
void read_event_track(void *, Dword, Dword *);
Byte event_humans(void *);
void write_event_humans(void *, int);
Dword event_racers(void *);
void write_event_racers(void *, Dword);
Dword event_cops(void *);
void write_event_cops(void *, Dword);
Dword event_difficulty(void *);
void write_event_difficulty(void *, Dword);
Byte event_car_class(void *);
void write_event_car_class(void *, Dword);
Byte event_traffic(void *);
void write_event_traffic(void *, Dword);
Dword event_helicopters(void *);
void write_event_helicopters(void *, Dword);
void save_event_grid(void *);
void restore_event_grid(void *);
void read_event_participant(void *, Dword, void *);
void write_event_participant(void *, Dword, const void *);
void record_event_position(void *, Dword, Dword, Byte);
Byte event_track(void *, Dword);
Dword grid_free_slot(const GridCarPool *);
Dword grid_car_slot(const GridCarPool *, int);
int catalogue_car_in_class(void *, int, int);
Byte catalogue_car_is_traffic(void *, int);
void collect_pursuit_cars(int, int, int *, int *);
void __fastcall prepare_grid_car_pool(void *, void *, GridCarPool *, Dword, int, int);
void __fastcall configure_race_grid(void *, void *, Dword);
int __fastcall evaluate_race_medal(void *, void *, const int *, void *, Byte);
void __fastcall update_race_results(void *, void *);
void clear_pursuit_finish_flags();
void capture_participant_finish(void *, Dword);
void record_participant_speeds(void *, Dword, Dword);
void update_finish_rewards(void *);
void save_race_snapshot(void *, void *);
void rank_race_finishers(void *);
void rank_race_racers(void *);
void rank_race_cops(void *);
void rank_quota_players(void *);
void rank_elimination_finishers(void *);
void __fastcall finish_race_run(void *, void *);
void announce_race_results(void *, int, int);
void request_network_finish();
} // namespace nfs::application::pending
