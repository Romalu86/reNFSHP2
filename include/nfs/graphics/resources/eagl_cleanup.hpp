#pragma once
#include "nfs/graphics/resources/eagl_objects.hpp"
namespace nfs::graphics {
extern Dword* backend_device;extern Dword* backend_interface;
namespace resources {
struct CacheState {Dword* vertices;Dword* indices;Dword* vertex_free_positions;Dword* index_free_positions;Dword vertex_count,index_count;std::uint8_t ready,padding[3];Dword* vertex_head;Dword* pixel_head;Dword* uses;};
static_assert(sizeof(CacheState)==40&&offsetof(CacheState,uses)==36);
extern CacheState cache;extern int vertex_capacity,index_capacity;extern Dword bound_streams[16],bound_index;
extern Dword* render_methods;extern Dword vertex_program_mode,pixel_program_mode;
void index_destroy(Dword*);void uses_destroy(Dword*);void vertex_destroy(Dword*);void pixel_destroy(Dword*);
void destroy_cached_vertex(Dword*);void release_vertex(Dword*,int);void release_index(Dword*);void shutdown();void shutdown_tail();
void parameter_destroy(Dword*);void parameter_list_destroy(Dword*);void parameter_block_destroy(Dword*);void dictionary_destroy(Dword*);
Dword technique_count(const Dword*);Dword technique_at(const Dword*,int);
Dword* find_technique(int,Dword*);void normalize_command(Dword*);void release_program_data(Dword);
void clear_method(Dword*,Dword);void clear_method_variant(Dword*,Dword);void method_destroy(Dword*);
void initialize(Dword,Dword);Dword* index_construct(Dword*,Dword,Dword);Dword* index_construct_usage(Dword*,Dword,Dword,Dword);
Dword index_lock(Dword*,Dword,Dword,Dword*,Dword);Dword index_unlock(Dword*);void index_lost(Dword*);void index_restore(Dword*);
void uses_construct(Dword*,Dword,Dword,Dword,Dword);void uses_restore(Dword*);Dword* vertex_fvf_construct(Dword*,Dword);Dword* pixel_construct(Dword*,Dword);
void device_lost();void device_restored();void vertex_programs_lost();
}
void context_base_destroy(Dword*);void context_contents_shutdown(Dword*);
extern std::uint8_t window_created;extern HINSTANCE& window_instance;extern const char*& window_class;
namespace targets {
extern Dword* first_links;extern Dword* second_links;
Dword *construct(Dword *, Dword *); Dword *create(Device *); Dword *create_viewport(Dword *);
Dword bind_textures(Dword *, Dword *, Dword *);
void viewport_destroy(Dword*,Dword*);void detach_links(Dword*);void surface_destroy(Dword**);void contents_destroy(Dword*);void destroy(Device*,Dword*);
}
void device_state_destroy(Dword*);void device_base_destroy(Dword*);void device_shutdown(Device*);
}
