#pragma once
#include "nfs/graphics/particles/particle_lifecycle.hpp"
#include "nfs/graphics/primitives/immediate_draw.hpp"
namespace nfs::game::runtime::skids::lifecycle {
extern Byte records[1500 * 84];
extern Byte *free_head;
extern const Dword identity_matrix[16];
void initialize_free_list();
void *allocate_skid();
void release_skid(void *);
void initialize(Dword *);
void load_level(Dword *);
void unload_level(Dword *);
Dword *construct_array(Dword *);
void destroy_array(Dword *);
namespace pending {
Dword *construct_render_method(Dword *, Dword *);
void destroy_render_method(Dword *);
void allocate_vertices(Dword *, int, Dword, Dword *, Dword *, Dword *, Dword, const char *,
                       Dword);
extern Dword render_method_definition[];
} // namespace pending
} // namespace nfs::game::runtime::skids::lifecycle
