#pragma once
#include "nfs/graphics/textures/texture_conversion.hpp"
#include "nfs/core/types/kernel.hpp"

namespace nfs::graphics::textures::shape {

using Byte = std::uint8_t;
using Allocate = void *(__cdecl *)(const char *, int, Dword);
using AllocateAligned = void *(__cdecl *)(const char *, int, Dword, Dword, Dword);
using Release = std::uint8_t(__cdecl *)(void *);
using RowConverter = void (*)(void *, const void *, int);

extern Allocate allocate;
extern AllocateAligned allocate_aligned;
extern Release release;
extern Byte palette_scratch[1024], default_palette[1024];
extern Byte *active_palette;
extern Byte *colour_index_lookup;
extern const RowConverter transparent_converters[144], opaque_converters[144];
extern const Byte conversion_types[144];

Dword pack_alpha1555(Dword);
Dword pack_visible565(Dword);
void make_alpha_palette(void *);
void palette_rgb6(int, const void *, void *);
void palette_rgb8(int, const void *, void *);
Dword *palette_colours(Dword *, Dword *);
int format_index(Dword);
Dword type_from_depth(int);
int normalized_depth(int);
int mip_storage_bytes(int, int, int, int);
int palette_entries(int);
Dword palette_type_from_depth(int);
int allocation_bytes(int, int, int, int, int, int, int);
void initialize_shape(Dword *, int, int, int, int, int, Dword, int);
Dword *allocate_shape(int, int, int, int, int, Dword, int, int);
Dword *convert_type(Dword *, Dword, Byte, int, int);
Dword *convert_type_default(Dword *, Dword);
Dword *convert_depth(Dword *, int);
int attachment_bytes(Dword *);
void copy_attachments(void *, Dword *);
char *shape_name(Dword *);
Dword *copy_shape(Dword *);
void release_shape(Dword *);
bool swizzle32(void *, const void *, int, int);
bool swizzle16(void *, const void *, int, int);
bool swizzle8(void *, const void *, int, int);
bool swizzle4(void *, const void *, int, int);
bool unswizzle32(void *, const void *, int, int);
bool unswizzle16(void *, const void *, int, int);
bool unswizzle8(void *, const void *, int, int);
bool unswizzle4(void *, const void *, int, int);
Byte swizzle_levels(Dword *, Dword *);
Dword *swizzle_shape(Dword *);
Dword *unswizzle_shape(Dword *);
Dword *half_size(Dword *);
Dword *build_mips(Dword *, int);
Dword *build_mips(Dword *);
Dword *unpack_shape(Dword *);
Dword *pack_shape(Dword *, int, Dword);
using AlternateUnpack = Dword (*)(const Byte *, void *);
extern AlternateUnpack alternate_unpack;
int unpack_refpack(void *, const Byte *, int *);
Dword unpack_buffer(const Byte *, void *);
void decode_colour_tile(Dword *, int, const Dword *);
void decode_alpha_tile(Dword *, int, const Byte *, int);
void decode_dxt_image(const void *, void *, int, int, int);
void compress_dxt_image(const void *, void *, int, int, int, Dword);
// This is a separate integer SHAPE encoder, not the D3DX fitter.
void encode_dxt_tile(const void *, void *, int, int, int, int, int, int, int);

namespace rows {
void copy_indices4(void *, const void *, int);
void pack_indices4(void *, const void *, int);
void swap_nibbles(void *, const void *, int);
void expand_indices4(void *, const void *, int);
void copy_indices8(void *, const void *, int);
void lookup_565(void *, const void *, int);
void rgb565_to_argb1555(void *, const void *, int);
void rgb565_to_xrgb1555(void *, const void *, int);
void palette_to_argb1555(void *, const void *, int);
void palette_to_argb1555_opaque_binding(void *, const void *, int);
void indices4_to_word16(void *, const void *, int);
void indices8_to_word16(void *, const void *, int);
void argb1555_to_rgb565(void *, const void *, int);
void copy_word16(void *, const void *, int);
void palette_to_rgb565(void *, const void *, int);
void palette_to_rgb565_opaque_binding(void *, const void *, int);
void bgra_to_argb4444(void *, const void *, int);
void argb1555_to_argb4444(void *, const void *, int);
void rgb565_to_argb4444(void *, const void *, int);
void bgra_high_word16(void *, const void *, int);
void indices8_to_bgr(void *, const void *, int);
void rgb565_to_bgr(void *, const void *, int);
void copy_bgr(void *, const void *, int);
void bgra_to_bgr(void *, const void *, int);
void bgra_to_argb6666(void *, const void *, int);
void bgra_to_argb2101010(void *, const void *, int);
void indices4_to_bgra(void *, const void *, int);
void indices8_to_bgra(void *, const void *, int);
void argb1555_to_bgra(void *, const void *, int);
void rgb565_to_bgra(void *, const void *, int);
void argb4444_to_bgra(void *, const void *, int);
void alpha1555_to_bgra(void *, const void *, int);
void alpha565_to_bgra(void *, const void *, int);
void alpha_luminance8_to_bgra(void *, const void *, int);
void bgr_to_bgra(void *, const void *, int);
void argb6666_to_bgra(void *, const void *, int);
void argb2101010_to_bgra(void *, const void *, int);
void copy_bgra(void *, const void *, int);
void yuy2_to_bgra(void *, const void *, int);
void bgra_to_yuy2(void *, const void *, int);
} // namespace rows

} // namespace nfs::graphics::textures::shape
