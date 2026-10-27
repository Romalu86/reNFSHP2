#pragma once

namespace nfs::game::runtime::hardware {
void store_dot_product(const float *first, const float *second, void *output);
void store_vector_length(const float *value, void *output);
void store_point_distance(const float *first, const float *second, void *output);
void store_distance_squared(const float *first, const float *second, void *output);
}
