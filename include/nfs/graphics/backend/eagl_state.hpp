#pragma once
#include "nfs/graphics/resources/eagl_objects.hpp"
namespace nfs::graphics::state {
extern std::uint8_t hardware_vertex_programs_supported,hardware_fixed_function_processing_supported;extern Dword render_frame_serial,texture_lod_bias_bits_override,texture_filter_override;
std::uint8_t read_depth_write(Dword*,std::uint8_t*);std::uint8_t read_context_multisample_type(Dword*,Dword*);std::uint8_t mark_context_dirty(Dword*);
std::uint8_t set_context_backbuffer_count(Dword*,Dword);Dword context_mode(Dword*);void set_context_stencil_bits(Dword*,Dword);Dword context_stencil_bits(Dword*);void set_context_external_window_handle_pointer(Dword*,Dword);
std::uint8_t supports_hardware_vertex_programs();std::uint8_t supports_hardware_fixed_function_processing();Dword context_backbuffer_format(Dword*);std::uint8_t set_context_fog_enabled(Dword*,Dword);
void read_fog_start(Dword*,Dword*);void read_fog_end(Dword*,Dword*);void read_context_fog_enabled(Dword*,std::uint8_t*);
std::uint8_t read_context_table_fog_mode(Dword*,Dword*);std::uint8_t read_context_vertex_fog_mode(Dword*,Dword*);std::uint8_t set_fog_start(Dword*,Dword);std::uint8_t set_fog_end(Dword*,Dword);
std::uint8_t read_fog_density(Dword*,Dword*);std::uint8_t read_window_name(Dword*,Dword*);std::uint8_t set_texture_lod_bias_bits_override(Dword);std::uint8_t set_texture_filter_override(Dword);
void set_error_callback(Dword*,Dword);Dword get_render_frame_serial();
}
