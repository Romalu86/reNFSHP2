#pragma once
#include "../profiles/ai_profile_runtime.hpp"
#include "../lifecycle/ai_chase_tuning.hpp"
#include "../../math/precision/extended_float.hpp"
#include "nfs/graphics/resources/eagl_graphics.hpp"
namespace nfs::game::runtime::ai::lifecycle::pending {
extern Dword roadblock_minimum_cars, roadblock_maximum_cars;
void reset_police_chase(void *);
void release_population_location(void *);
void initialize_population_locations(void *);
void load_population_locations(void *);
void clear_population_locations(void *);
void release_population_locations(void *);
void bind_population_location_paths(void *);
void bind_population_location_path(void *);
void reset_helicopter_population(Dword *);
void reset_helicopter_projectile(void *);
void load_spike_population(Dword *);
void reset_spike_population(Dword *);
void release_spike_population(Dword *);
void deactivate_spike_entity(void *);
Dword *construct_spike_entity(Dword *, void *, int);
Dword *construct_helicopter_entity(Dword *, void *, int);
Dword *construct_helicopter_projectile(Dword *, void *, int);
Dword bind_shape_body(void *, Dword);
void anchor_path_graph(void *, const float *);
int nearest_track_point(void *, const float *);
void *construct_population_path(void *);
void destroy_population_path(void *);
int load_helicopter_population(Dword *, int);
extern const Dword projectile_profile_methods[];
Dword *construct_path_controller(Dword *);
void destroy_path_controller(Dword *);
void *construct_location(void *);
void *construct_traffic_locations(void *);
void destroy_traffic_locations(void *);
void *construct_roadblock_locations(void *);
void destroy_roadblock_locations(void *);
Dword *construct_spike_slots(Dword *);
void destroy_spike_slots(Dword *);
extern const Dword location_base_methods[], population_location_methods[];
void __fastcall bind_location(void *, void *, Dword, const Dword *);
int track_point_count(void *);
Byte location_available(void *);
const float *path_position(void *, int);
void load_traffic_location_records(void *);
void __fastcall initialize_traffic_locations(void *, void *);
Dword __fastcall reserve_traffic_location(void *, void *, Dword);
Dword __fastcall reserve_nearest_traffic_location(void *, void *, void *);
Dword __fastcall reserve_roadblock_location(void *, void *, void *);
} // namespace nfs::game::runtime::ai::lifecycle::pending
