#pragma once
#include <cstdint>

namespace nfs::game::world::actor_resources::pending {

void apply_actor_matrix_coefficients(float *matrix, const float *coefficients);
void add_actor_ground_offset(float *position, const void *actor);
float actor_heading_degrees(const void *actor);
float actor_heading_radians(std::uint32_t x_bits, std::uint32_t z_bits);

} // namespace nfs::game::world::actor_resources::pending
