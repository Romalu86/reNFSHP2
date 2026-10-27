#pragma once
#include "nfs/graphics/textures/pixel_blocks.hpp"

namespace nfs::graphics::surfaces::pixels {

// Pixel compression interfaces.
void fit_alpha_endpoints(float *lower, float *upper, const float *samples, Dword levels);
void fit_colour_endpoints(float *lower, float *upper, const Colour *samples, Dword levels);
Dword pack_565(const float *colour);
int encode_colour_block(void *destination, const Colour *pixels, Dword allow_transparency);
int encode_dxt5(void *destination, const Colour *pixels);

} // namespace nfs::graphics::surfaces::pixels
