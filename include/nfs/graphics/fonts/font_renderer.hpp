#pragma once
#include "nfs/graphics/fonts/fonts.hpp"
#include "nfs/graphics/backend/eagl_backend.hpp"
namespace nfs::graphics::font_renderer {
struct Vertex {
    float x, y, z, reciprocal_w;
    Dword colour;
    float u, v;
};
static_assert(sizeof(Vertex) == 28);
extern const fonts::Callbacks callbacks;
extern const fonts::Settings batch_settings;
extern Dword state[17];
extern backend::TextureStageState stages;
extern float inverse_height, inverse_width, scale_y, scale_x, reciprocal_w, depth, offset_u,
    offset_v;
extern Dword colour, vertex_count, index_count, references;
extern Vertex *vertices;
extern std::uint16_t *indices;
extern Dword *palette;
void __cdecl draw(fonts::Font *, const std::uint8_t *, float, float);
void __cdecl batch(fonts::Font *, const fonts::DrawItem *, int);
void __cdecl begin(fonts::Font *);
void __cdecl end(fonts::Font *);
void __cdecl initialize(fonts::Font *);
void __cdecl release(fonts::Font *);
Dword *convert_shape(Dword *);
} // namespace nfs::graphics::font_renderer
namespace nfs::graphics::tar {
Dword *create_image(Dword *);
}
