#pragma once
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
namespace nfs::game::runtime {
const char *level_file(const char *);
const char *track_file(const char *);
void *level_marker(Dword *, int, int);
void normalize_vector(const float *, float *);
long double vector_length(const float *);
void scale_vectors(int, const float *, float, float *);
long double dot_product(const float *, const float *);
long double dot_product_entry(const float *, const float *);
namespace pending {
extern char level_path[];
extern char track_path[];  
}
void reset_car_particle_pool(Dword *);
void update_car_particle_pool(Dword *);
void reset_particle_buffers();
long double sample_emitter_period(Dword *);
void reset_world_particle_pool(Dword *);
void release_vehicle_asset(Dword *);
void release_vehicle_assets(Dword *);
namespace pending {
void reset_particle_emitter(Dword *);
}
} // namespace nfs::game::runtime
