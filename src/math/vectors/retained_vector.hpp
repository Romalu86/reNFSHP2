#pragma once
#include "../precision/extended_float.hpp"

namespace nfs::game::runtime {
// Callers which consume ST0 without an intervening binary64 store.
kernel::numeric::Extended retained_vector_length(const float *);
kernel::numeric::Extended retained_dot_product(const float *, const float *);
kernel::numeric::Extended retained_point_distance(const float *, const float *);
kernel::numeric::Extended retained_distance_squared(const float *, const float *);
} // namespace nfs::game::runtime

namespace nfs::game::runtime::track {
kernel::numeric::Extended retained_vehicle_direction(void *);
} // namespace nfs::game::runtime::track
