#pragma once
#include "race_frame_draw.hpp"
#include "nfs/graphics/particles/particle_lifecycle.hpp"
#include "nfs/graphics/primitives/immediate_draw.hpp"
#include "nfs/physics/frame/simulation_runtime.hpp"
#include "../../gui/lifecycle/gui_images.hpp"
#include "../../math/precision/extended_float.hpp"
namespace nfs::game::world::frame {
struct ScreenEffect {
    void *definition;
    Dword remaining;
    float angle;
    Dword colour;
    float x, y, depth, size;
    ScreenEffect *next;
};
static_assert(sizeof(ScreenEffect) == 36 && offsetof(ScreenEffect, next) == 32);
inline constexpr float effect_uv[6][4] = {
    {0, 0, 1, 1}, {0, 1, 1, 1}, {1, 1, 1, 1},
    {1, 1, 1, 1}, {1, 0, 1, 1}, {0, 0, 1, 1}};
Dword *pack_effect_colour(Dword *, Dword, Dword, Dword, Dword);
float *assign_effect_vector(float *, float, float, float, float);
float *subtract_effect_vector(const float *, float *, const float *);
void *draw_rotated_effect_batch(void *, const float *);
void *draw_trail_effect_batch(void *, const float *, const float *, Dword);
void draw_screen_effects(void *);
void draw_world_effects(void *);
}
