#pragma once
#include "nfs/world/lifecycle/world_lifecycle.hpp"
#include "nfs/graphics/primitives/immediate_draw.hpp"
namespace nfs::graphics::methods {
Dword *construct_definition(Dword *, Dword *, Dword *, Dword, const char *const *, Dword);
}
namespace nfs::graphics::gouraud {
using kernel::Dword;
extern Dword render_definition[11];
Dword *construct(Dword *, Dword, Dword, Dword);
void destroy(Dword *);
void set_state(Dword *, const Dword *);
void set_matrix(Dword *, const Dword *);
void set_vertex(Dword *, Dword, const float *, const Dword *);
void commit(Dword *);
void draw(Dword *);
} // namespace nfs::graphics::gouraud
