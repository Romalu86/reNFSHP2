#pragma once
#include "nfs/graphics/textures/pixel_codec.hpp"
namespace nfs::graphics::surfaces {
struct LinearSample {
    Dword first;
    float first_weight;
    Dword second;
    float second_weight;
};
struct Contribution {
    Dword index;
    float weight;
};
struct Accumulator {
    pixels::Colour *values;
    Accumulator *next;
    Dword references;
};
static_assert(sizeof(LinearSample) == 16 && sizeof(Contribution) == 8 && sizeof(Accumulator) == 12);
LinearSample *linear_samples(Dword, Dword, Dword);
Dword *triangle_weights(Dword, Dword, Dword);
int half_2d(Transfer *);
int half_3d(Transfer *);
int linear_2d(Transfer *);
int linear_3d(Transfer *);
int triangle_2d(Transfer *);
int triangle_3d(Transfer *);
int packed_half(Transfer *);
int mmx_available();
int average_argb(Dword *, const Dword *, Dword, Dword, Dword, Dword);
int average_xrgb(Dword *, const Dword *, Dword, Dword, Dword, Dword);
int average_mmx(Dword *, const Dword *, Dword, Dword, Dword, Dword);
} // namespace nfs::graphics::surfaces
