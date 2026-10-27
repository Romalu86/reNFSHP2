#pragma once

namespace nfs::game::world::camera::route_math {
void lateral_native80(void *entity, void *output);
void heading_native80(void *entity, void *output);
bool heading_below_half(void *entity);
bool heading_negative(void *entity);
bool heading_at_least_half(void *entity);
bool lateral_beyond_three_quarters(void *entity);
bool lateral_negative_for_target(void *entity, void *target);
bool lateral_above_three_quarters(void *entity);
float lateral_f32(void *entity);
} // namespace nfs::game::world::camera::route_math
