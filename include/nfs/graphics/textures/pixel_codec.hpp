#pragma once
#include "nfs/graphics/textures/surface_conversion.hpp"
namespace nfs::graphics::surfaces::pixels {
struct Colour {float red,green,blue,alpha;};
struct Codec {Dword words[1051];};
static_assert(sizeof(Codec)==4204&&sizeof(Colour)==16);
extern const Dword base_methods[4];extern const float flat_dither[32],ordered_dither[32];
Codec* construct(Codec*,const Image*,Dword,Dword);void destroy(Codec*);Colour* convert(Codec*,const Colour*);int source(Codec*,const Codec*);void colour_key(Codec*,Colour*);void __fastcall quantize_key(Codec*,void*);
void decode_colour(Colour*,Dword);void read(Codec*,Dword,Dword,Dword,Colour*);void write(Codec*,Dword,Dword,Dword,const Colour*);
Codec* construct_linear(Codec*,const Image*,Dword);Dword* create(Image*);
Codec* construct_yuv(Codec*,const Image*);Codec* construct_uyvy(Codec*,const Image*);Codec* construct_yuy2(Codec*,const Image*);
void destroy_yuv(Codec*);int flush_yuv(Codec*);int cache_yuv(Codec*,Dword,Dword,Dword);void read_yuv(Codec*,Dword,Dword,Colour*);void write_yuv(Codec*,Dword,Dword,const Colour*);
extern const Dword* const linear_tables[30];extern const Dword linear_formats[30];

}
