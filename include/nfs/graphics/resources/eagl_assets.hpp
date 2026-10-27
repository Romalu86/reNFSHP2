#pragma once
#include "nfs/graphics/backend/eagl_backend.hpp"

extern "C" void __cdecl _assert(const char*,const char*,unsigned);
namespace nfs::graphics::properties {
struct Property {char* name;int count;char** values;};
struct Parser {int count;Property* entries;int length;char* text;};
static_assert(sizeof(Property)==12&&sizeof(Parser)==16);
using InstanceMethod=void(__thiscall*)(void*);
void __stdcall construct_elements(void*,int,int,InstanceMethod);void __stdcall destroy_elements(void*,int,int,InstanceMethod);
Dword* value_construct(Dword*);Property* property_construct(Property*);void property_destroy(Property*);
char* skip_delimiters(char*);char split_delimiter(char*,char**);void parse(Parser*,const char*);void destroy(Parser*);
std::uint8_t match_property(const Property*,const char*,Dword);
Dword geo_value(const char*);Dword pc_value(const char*);Dword tar_api_value(const char*);Dword tar_value(const char*);
}
namespace nfs::graphics::geoprim {
extern Dword current_primitive;extern std::uint8_t depth_override;
Dword* copy_construct(Dword*,const Dword*);Dword* assign(Dword*,const Dword*);Dword* base_construct(Dword*);void base_destroy(Dword*);void destroy(Dword*);
std::uint8_t primitive(Dword*,Dword);std::uint8_t shading(Dword*,Dword);std::uint8_t culling(Dword*,Dword);std::uint8_t depth_method(Dword*,Dword);std::uint8_t read_depth_method(Dword*,Dword*);
std::uint8_t alpha_blend_mode(Dword*,Dword);std::uint8_t alpha_test(Dword*,Dword);std::uint8_t alpha_reference(Dword*,Dword);std::uint8_t alpha_method(Dword*,Dword);std::uint8_t texture_enable(Dword*,Dword);
std::uint8_t texture_coordinates(Dword*,Dword);std::uint8_t transparency(Dword*,Dword);std::uint8_t chroma_colour(Dword*,Dword);std::uint8_t cull_direction(Dword*,Dword);std::uint8_t patch_segments(Dword*,Dword);
std::uint8_t fill_mode(Dword*,Dword);std::uint8_t blend_operation(Dword*,Dword);std::uint8_t blend_factors(Dword*,Dword,Dword,Dword);std::uint8_t depth_write(Dword*,Dword);
void apply_property(Dword*,const properties::Property*);Dword* runtime_allocate(const char*,eagl::Loader*,Dword*,std::uint8_t*);void runtime_release(Dword*,Dword);
std::uint8_t apply(Dword*);
}
namespace nfs::graphics::backend::unresolved {
extern Dword* texture_links;extern Dword texture_filters[7][8],texture_values[8],texture_objects[8],material_state[17],render_states[15],render_dirty;
}
namespace nfs::graphics::tar {
Dword *create_colour_target(Dword, Dword, Dword);
Dword *create_depth_target(Dword *, Dword, Dword, Dword);
extern std::uint8_t fallback_image[11072],validation_image[11072];extern Dword palette_sequence;
std::uint8_t replace_shared(Dword*,Dword*);Dword swizzled_offset(Dword,Dword,int,int,int);
Dword image_format(const Dword*);Dword storage_format(int);Dword* find_palette(Dword*);std::uint8_t image_depth(const Dword*);Dword* image_private(Dword*);Dword* image_level(Dword*,int,int,int);
Dword* private_data(Dword*);Dword* base_construct(Dword*);Dword* shared_construct(Dword*);void shared_destroy(Dword*);Dword* extension_construct(Dword*,Dword*);void extension_destroy(Dword*);
Dword* construct(Dword*);Dword *construct_shape(Dword*,Dword*);Dword* copy_construct(Dword*,Dword*);void destroy(Dword*);Dword* shape(Dword*);void upload(Dword*);
std::uint8_t replace_image(Dword *, Dword *);
void constructor(Dword*,eagl::Loader*);void destructor(Dword*);Dword* runtime_allocate(const char*,eagl::Loader*,Dword*,std::uint8_t*);void runtime_release(Dword*,int);
std::uint8_t set_stage(Dword*,Dword);void set_palette(Dword*,Dword*);void apply_property(Dword*,const properties::Property*);void bind(Dword*);
}
namespace nfs::graphics::methods {
Dword* find_command(Dword,Dword*);void initialize(Dword*);void initialize_variant(Dword*);
void build_buffers(Dword*,Dword);void build_variant(Dword*,Dword,Dword);void rebuild(Dword*);
Dword* duplicate_coordinates(int,Dword,int,int,Dword);Dword* clone_vertex(Dword*,std::uint8_t,std::uint8_t,Dword,std::uint8_t);
std::uint8_t cpu_program(Dword*);std::uint8_t command_draw_marker(Dword*,Dword,Dword);
void model_constructor(Dword*);void model_destructor(Dword*);void model_initialize(Dword*);
}
namespace nfs::graphics::techniques {
extern Dword device_caps[53];extern std::uint8_t caps_ready;
Dword* parameter_construct(Dword*,Dword**,Dword);Dword* pass_construct(Dword*,Dword**,Dword);Dword* technique_construct(Dword*,Dword**,Dword);Dword* object_construct(Dword*,Dword*);
void pass_constants(Dword*,Dword);void technique_constants(Dword*,Dword);void select(Dword*,Dword);void begin(Dword*,Dword*,Dword);void end(Dword*,Dword,Dword);void finish(Dword*);
std::uint8_t parameter_validate(Dword*);std::uint8_t pass_validate(Dword*);std::uint8_t technique_validate(Dword*);void pass_apply(Dword*);
}
namespace nfs::graphics::resources {Dword* vertex_program_construct(Dword*,Dword,Dword);void bind_vertex(Dword*);void bind_pixel(Dword*);}
namespace nfs::graphics::buffers {
extern const Dword vertex_methods[3];
Dword* vertex_construct(Dword*,Dword,Dword,Dword,Dword,Dword,Dword);void vertex_destroy(Dword*);void* __fastcall vertex_delete(Dword*,void*,unsigned);
void __fastcall vertex_lost(Dword*,void*);void __fastcall vertex_restore(Dword*,void*);
Dword used_bytes(Dword*);Dword capacity(Dword*);Dword usage(Dword*);Dword layout(Dword*);Dword stride(Dword*);Dword data(Dword*);
Dword lock(Dword*,Dword,Dword,Dword*,Dword);Dword unlock(Dword*);void replace_data(Dword*,Dword,Dword);
Dword* create_vertex(Dword,Dword,Dword,Dword,Dword,Dword);Dword* create_vertex_data(Dword,Dword,Dword,Dword,Dword,Dword);Dword* create_index_data(Dword,Dword,Dword*,Dword);
Dword* vertex_copy(Dword*,Dword*);Dword grow(Dword*,Dword);
Dword* vertex_construct_default(Dword*,Dword,Dword,Dword,Dword);Dword* create_index(Dword,Dword,Dword);
extern const Dword skinned_methods[3];extern Dword selection_index,last_vertex_type;
Dword* create_skinned(int,Dword,Dword,Dword,Dword,Dword,Dword,std::uint8_t);
void skinned_destroy(Dword*);void* __fastcall skinned_delete(Dword*,void*,unsigned);void __fastcall skinned_lost(Dword*,void*);void __fastcall skinned_restore(Dword*,void*);
void restore_segment(Dword*,int);void restore_segments(Dword*);Dword lock_segment(Dword*,Dword,Dword,Dword*,Dword);void reset_type_cache();
}
namespace nfs::graphics::backend {
Dword bound_pixel_shader();void clear_stream_bindings();void apply_texture_delta(const TextureStageState*);
void set_vertex_handle(Dword);void vertex_constants(Dword,Dword,Dword);void bind_stream(Dword,Dword*);void index_buffer(Dword*);void index_base(Dword);void bind_indices();
void pixel_constants(Dword,Dword,Dword);void draw_primitives(Dword,Dword,Dword);void draw_indexed(Dword,Dword,Dword,Dword,Dword);void draw_user(Dword,Dword,Dword);void draw_indexed_user(Dword,Dword,Dword,Dword,Dword,Dword);void clear_index_bindings();
void texture_factor(Dword);TextureStageState* construct_texture_stages(TextureStageState*);void destroy_texture_stages(TextureStageState*);
}
