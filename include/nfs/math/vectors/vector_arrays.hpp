#pragma once
namespace nfs::game::runtime {
const float *add_vector_and_advance(const float *, const float *, float *);
const float *subtract_vector_and_advance(const float *, const float *, float *);
void add_constant_vector_array(int, const float *, const float *, float *);
void subtract_constant_vector_array(int, const float *, const float *, float *);
} // namespace nfs::game::runtime
