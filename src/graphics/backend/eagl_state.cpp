#include "nfs/graphics/backend/eagl_state.hpp"
namespace nfs::graphics::state {
std::uint8_t hardware_vertex_programs_supported=0,hardware_fixed_function_processing_supported=0;Dword render_frame_serial=0,texture_lod_bias_bits_override=0,texture_filter_override=4294967295u; // Shared graphics state.
static Dword* owner(Dword* object){return reinterpret_cast<Dword*>(object[1]);}

std::uint8_t read_depth_write(Dword* object,std::uint8_t* output){*output=reinterpret_cast<std::uint8_t*>(object)[109];return 1;}

std::uint8_t read_context_multisample_type(Dword* object,Dword* output){*output=owner(object)[24/4];return 1;}

std::uint8_t mark_context_dirty(Dword* object){owner(object)[172/4]|=1;return 1;}

std::uint8_t set_context_backbuffer_count(Dword* object,Dword value){owner(object)[56/4]=value;return 1;}

Dword context_mode(Dword* object){return owner(object)[152/4];}

void set_context_stencil_bits(Dword* object,Dword value){owner(object)[48/4]=value;}

Dword context_stencil_bits(Dword* object){return owner(object)[48/4];}

void set_context_external_window_handle_pointer(Dword* object,Dword value){owner(object)[160/4]=value;}

std::uint8_t supports_hardware_vertex_programs(){return hardware_vertex_programs_supported;}

std::uint8_t supports_hardware_fixed_function_processing(){return hardware_fixed_function_processing_supported;}

Dword context_backbuffer_format(Dword* object){return owner(object)[4];}

std::uint8_t set_context_fog_enabled(Dword* object,Dword value){reinterpret_cast<std::uint8_t*>(owner(object))[110]=static_cast<std::uint8_t>(value);return 1;}

void read_fog_start(Dword* object,Dword* output){*output=owner(object)[120/4];}

void read_fog_end(Dword* object,Dword* output){*output=owner(object)[124/4];}

void read_context_fog_enabled(Dword* object,std::uint8_t* output){*output=reinterpret_cast<std::uint8_t*>(owner(object))[110];}

std::uint8_t read_context_table_fog_mode(Dword* object,Dword* output){*output=owner(object)[112/4];return 1;}

std::uint8_t read_context_vertex_fog_mode(Dword* object,Dword* output){*output=owner(object)[116/4];return 1;}

std::uint8_t set_fog_start(Dword* object,Dword value){owner(object)[120/4]=value;return 1;}

std::uint8_t set_fog_end(Dword* object,Dword value){owner(object)[124/4]=value;return 1;}

std::uint8_t read_fog_density(Dword* object,Dword* output){*output=owner(object)[128/4];return 1;}

std::uint8_t read_window_name(Dword* object,Dword* output){*output=owner(object)[180/4];return 1;}

std::uint8_t set_texture_lod_bias_bits_override(Dword value){texture_lod_bias_bits_override=value;return 1;}

std::uint8_t set_texture_filter_override(Dword value){texture_filter_override=value;return 1;}

void set_error_callback(Dword* object,Dword value){owner(object)[188/4]=value;}

Dword get_render_frame_serial(){return render_frame_serial;}
}
