#pragma once
#include "nfs/core/types/kernel.hpp"

namespace nfs::graphics::math {
using Vector = float[4];
using Matrix = float[16];
extern Matrix multiply_scratch;
void transform_row(const float *, const float *, float *);
void multiply(const float *, const float *, float *);
void transform_position(const float *, const float *, float *);
void cross(const float *, const float *, float *);
void append(float *, const float *);
void append_projection(float *, const float *);
void diagonal(float *, float, float, float, float);
void translation(float *, float, float, float);
void from_compact_transform(float *, const float *, const float *);
void append_translation(float *, float, float, float);
void append_axis_rotation(float *, float, float, float, float);
void copy(float *, const float *);
void perspective(float *, float, float, float, float);
void orthographic(float *, float, float, float, float);
void orthographic_bounds(float *, float, float, float, float, float, float);
} // namespace nfs::graphics::math
