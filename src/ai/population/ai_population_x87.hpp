#pragma once
#include <cstdint>
namespace nfs::game::runtime::ai::lifecycle::pending::population_frame {
void bind_location_path(void *, void *, const float *, void *);
void anchor_paths(std::uint32_t *);
std::uint32_t reserve_traffic_location(void *, std::uint32_t);
void recount_entity_kinds(std::uint32_t *);
int nearest_track_point(void *, const float *);
std::uint8_t player_within_range(void *, const float *, float);
void anchor_path_graph(void *, const float *);
void build_path_arc_distances(void *);
std::uint8_t insert_path_section(void *, void *);
std::uint32_t chase_transition_ticks(float);
void initialize_helicopter_attachment_offset();
void initialize_helicopter_projectile_offset();
extern const float chase_transition_multiplier, helicopter_scale, helicopter_attachment;
extern const float helicopter_projectile_x, helicopter_projectile_y, helicopter_projectile_z;
std::uint32_t truncate_native80(const void *);
#if defined(__clang__) && __LDBL_MANT_DIG__ == 64
inline std::uint32_t truncate_retained(long double value) { return truncate_native80(&value); }
#endif
}
