#pragma once
#include "nfs/graphics/textures/pixel_codec.hpp"
namespace nfs::graphics::surfaces::pixels {
void unpack_565(Colour *, Dword);
int unpremultiply(Colour *);
int premultiply(Colour *, const Colour *);
void interpolate(Colour *, const Colour *, const Colour *, float);
int decode_dxt1(Colour *, const void *);
int decode_dxt2(Colour *, const void *);
int decode_dxt3(Colour *, const void *);
int decode_dxt4(Colour *, const void *);
int decode_dxt5(Colour *, const void *);
int encode_dxt1(void *, const Colour *);
int encode_dxt2(void *, const Colour *);
int encode_dxt3(void *, const Colour *);
int encode_dxt4(void *, const Colour *);
Codec *construct_dxt(Codec *, const Image *);
void destroy_dxt(Codec *);
void read_dxt(Codec *, Dword, Dword, Colour *);
void write_dxt(Codec *, Dword, Dword, const Colour *);
void quantize_dxt_key(Codec *);
Codec *construct_dxt1(Codec *, const Image *);
Codec *construct_dxt2(Codec *, const Image *);
Codec *construct_dxt3(Codec *, const Image *);
Codec *construct_dxt4(Codec *, const Image *);
Codec *construct_dxt5(Codec *, const Image *);

} // namespace nfs::graphics::surfaces::pixels
