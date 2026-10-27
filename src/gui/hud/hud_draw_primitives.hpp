#pragma once
#include "nfs/graphics/primitives/immediate_draw.hpp"
namespace nfs::graphics::bindings {
struct HudVertex {
    float x, y, z;
    Dword colour;
    float u, v;
};
static_assert(sizeof(HudVertex) == 24);
extern Dword hud_texture;
extern std::uint8_t hud_alpha;
extern float hud_depth;
void bind_hud_texture(Dword);
void set_hud_alpha(std::uint8_t);
void set_hud_matrix(const Dword *);
void set_hud_depth(float);
void draw_hud_triangle(const HudVertex *, const HudVertex *, const HudVertex *);
void draw_hud_textured_triangle(const HudVertex *, const HudVertex *, const HudVertex *);
void draw_hud_quad(const HudVertex *, const HudVertex *, const HudVertex *, const HudVertex *);
Dword rear_view_texture(void *, Dword);
} // namespace nfs::graphics::bindings
