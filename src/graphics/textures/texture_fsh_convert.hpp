#pragma once
#include "nfs/graphics/textures/texture_conversion.hpp"
namespace nfs::graphics::textures {
void copy_resized_shape(Dword *, Dword *, Dword, Dword);
Dword convert_shape_into(TextureReader *, Dword *, Dword *, Dword, Dword *, float);
Dword *convert_fsh_frame(TextureReader *, Dword *, Dword *, Dword, std::uint8_t, std::uint8_t);
Dword *convert_fsh_into(TextureReader *, const Dword *, Dword *, Dword);
} // namespace nfs::graphics::textures
