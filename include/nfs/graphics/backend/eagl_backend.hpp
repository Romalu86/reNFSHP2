#pragma once
#include "nfs/graphics/resources/eagl_cleanup.hpp"
#include "nfs/graphics/backend/eagl_state.hpp"
namespace nfs::graphics::backend {
// Byte offsets and public COM slots come from the saved x86 image.
template<class... A> int com(Dword* object,unsigned offset,A... args){using Call=int(__stdcall*)(Dword*,A...);return reinterpret_cast<const Call*>(object[0])[offset/4](object,args...);}
inline Dword* owner(Dword* object){return reinterpret_cast<Dword*>(object[1]);}
struct Presentation {Dword width,height,format,count,multisample,swap;HWND window;Dword windowed,depth_enabled,depth_format,flags,refresh,interval;};
static_assert(sizeof(Presentation)==52&&offsetof(Presentation,flags)==40);
struct Capabilities {Dword words[1764];};
static_assert(sizeof(Capabilities)==7056);
extern Presentation presentation;extern Dword desktop_mode[4];extern Capabilities capability_data;
extern Dword* hardware_caps;extern Dword* alternate_caps;extern Dword material_color;
extern Dword stream_state,depth_write_state;extern std::uint8_t window_bound;extern MSG message;
extern WNDCLASSEXA window_definition;extern WORD saved_gamma[3][256];
extern const Dword formats16[5],formats32[2],texture_formats[36];
Dword format_depth(Dword);std::uint8_t choose_depth(Dword*,Dword,Dword,int*,int*,Dword*);
Dword exact_depth(Dword*,Dword,Dword,int,int);Dword choose_backbuffer(Dword*,Dword,int,std::uint8_t);Dword texture_format(Dword*,Dword);
std::uint8_t fullscreen_formats(Dword*,std::uint8_t);std::uint8_t windowed_formats(Dword*,std::uint8_t);std::uint8_t populate_presentation(Dword*);
Dword* retrieve_capabilities(Dword*,char*);void enumerate_modes(Dword*,Dword*);void enumerate_formats(Dword*,Dword*);void apply_device_quirks(Dword*);
Dword* query_adapter_capabilities(Dword*,Dword,char*);
Dword* adapter_caps();std::uint8_t retrieve_adapter_caps(std::uint8_t);void detect_vertex_programs(Dword*);void detect_fixed_function_vertex_processing(Dword*);
int create_hal_device(Dword*,std::uint8_t);int reset_device(Dword*);std::uint8_t external_window(Dword*,std::uint8_t,std::uint8_t);
std::uint8_t fullscreen_window(Dword*);std::uint8_t windowed_window(Dword*);HWND caps_window(WNDCLASSEXA);
bool recover_device(Dword*);void release_backbuffer(Dword*);void present(Dword*);int advance_frame(Dword*);std::uint8_t initialize_context(Dword*);
void set_dimensions(Dword*,float,float);void get_dimensions(Dword*,float*,float*);void set_color_depth(Dword*,Dword);Dword color_depth(Dword*);void set_depth_bits(Dword*,Dword);Dword requested_colour_depth(Dword*);
void set_display_mode(Dword*,Dword);std::uint8_t supports_format(Dword*,Dword,int,int,int,std::uint8_t);void drain_frames(Dword*);
void vertex_programs_restored();void pixel_programs_lost();void pixel_programs_restored();std::uint8_t set_software_processing(Dword);
// Driver compatibility: cache invalidation does not imply a physical shader unbind.
void invalidate_pixel_shader_cache() noexcept;
bool bind_pixel_shader(Dword);
void targets_lost();void targets_restored();void depth_targets_lost();void depth_targets_restored();
void set_vertex_capacity(Device*,Dword);Dword vertex_capacity(Device*);void set_index_capacity(Device*,Dword);Dword index_capacity(Device*);Dword font_glyph_capacity(Device*);std::uint8_t alternate_device(Device*);
Dword* selected_context(Device*);Device* selected_device();std::uint8_t initialize_device(Device*);
void register_matrix_symbols();
extern Dword identity[16];
extern Dword projection_matrix[16],model_matrix[16],view_projection_matrix[16],view_matrix[16],model_view_matrix[16],model_view_projection_matrix[16];
extern Dword fog_parameters[4];
struct TextureStageState {Dword values[17][8];Dword tail;};static_assert(sizeof(TextureStageState)==548);
extern TextureStageState texture_stage_state,texture_stage_snapshot;
void initialize_texture_stages();void apply_texture_stages(const TextureStageState*);
std::uint8_t set_depth_write(Dword*,std::uint8_t);std::uint8_t set_stencil_depth_fail(Dword*,Dword);std::uint8_t set_stencil_pass(Dword*,Dword);std::uint8_t set_stencil_fail(Dword*,Dword);
std::uint8_t set_stencil_function(Dword*,Dword);std::uint8_t set_stencil_reference(Dword*,Dword);std::uint8_t set_stencil_mask(Dword*,Dword);std::uint8_t set_stencil_write_mask(Dword*,Dword);
std::uint8_t set_stencil_enable(Dword*,std::uint8_t);std::uint8_t set_fog_density(Dword*,float);std::uint8_t set_fog_colour(Dword*,Dword);std::uint8_t set_table_fog(Dword*,Dword);std::uint8_t set_vertex_fog(Dword*,Dword);
namespace unresolved {
void buffers_lost();void buffers_restored();void buffer_bindings();void render_bindings();void vertex_state();void texture_state();
extern Dword* secondary_methods;

extern Dword texture_cache[8],render_width,render_height;extern std::uint8_t lighting_enabled;
extern Dword table_fog,vertex_fog;extern std::uint8_t fog_enabled;extern Dword display_mode;
}
}
