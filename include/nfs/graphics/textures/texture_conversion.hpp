#pragma once
#include "nfs/graphics/resources/eagl_graphics.hpp"
#include "nfs/resources/ini/ini.hpp"
namespace nfs::graphics::textures {
struct Format {Dword format;const char* name;Dword bits,shape;};
struct Fallback {Dword alpha,format;};
static_assert(sizeof(Format)==16&&sizeof(Fallback)==8);
extern const Format formats[12];
extern Fallback indexed_fallback[3],packed_fallback[3],colour_fallback[3];
extern TextureReader database;extern std::uint8_t conversion_enabled;
TextureReader* construct(TextureReader*);TextureReader* construct(TextureReader*,const char*,const char*);void destroy(TextureReader*);
Dword reserve(TextureReader*,Dword);bool equivalent(const TextureReader*,const TextureReader*);bool different(const TextureReader*,const TextureReader*);
bool supports_shape(const TextureReader*,Dword);bool supports_dxt3(const TextureReader*);bool supports_format(const TextureReader*,Dword);
void select_fallbacks(TextureReader*);Dword fallback_format(Dword,Dword);void constrain_dimensions(const TextureReader*,Dword*,Dword*);void clamp_dimensions(const TextureReader*,Dword*,Dword*);
Dword read_settings(TextureReader*,const char*,const char*);Dword write_settings(TextureReader*,const char*,const char*);Dword device_settings(TextureReader*);
void lower_power(Dword*);bool is_power(Dword);bool outside_range(Dword,Dword,Dword);
Dword shape_type(const Dword*);Dword* shape_end(Dword*);double storage_bytes(Dword,int,int);double storage_ratio(const Dword*,Dword);
std::uint8_t palette_alpha(const std::uint8_t*);void alpha1(const Dword*,Dword*);void alpha4(const Dword*,Dword*);void alpha8(const Dword*,Dword*);std::uint8_t alpha(const Dword*,std::uint8_t);
Dword* convert_shape(Dword*,Dword);bool extension(const char*,const char*);bool is_fsh(const char*);bool is_viv(const char*);
void resize_pixels(std::uint8_t*,const std::uint8_t*,int,int,int,int,int);
namespace shape {
Dword pitch(const Dword*);Dword* palette(Dword*);bool is_palette(Dword);Dword bytes(Dword,int,int,Dword,std::uint8_t);Dword size(const Dword*);int compressed_size(const std::uint8_t*,Dword*);
}
}
