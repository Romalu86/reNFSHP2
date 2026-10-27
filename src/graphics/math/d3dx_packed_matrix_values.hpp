#pragma once
#include "d3dx_dispatch_kernels.hpp"
namespace nfs::graphics::d3dx::packed {
void multiply_quaternion_values(float *, const float *, const float *);
void inverse_quaternion_values(float *, const float *);
void logarithm_quaternion_values(float *, const float *);
void exponential_quaternion_values(float *, const float *);
void yaw_pitch_roll_values(float *, float, float, float);
void quaternion_matrix_extended_values(float *, const float *);
void quaternion_matrix_values(float *, const float *);
float *__stdcall invert_cofactor_pairs(float *, float *, const float *);
float *__stdcall multiply_matrix_rows(float *, const float *, const float *);
} // namespace nfs::graphics::d3dx::packed
