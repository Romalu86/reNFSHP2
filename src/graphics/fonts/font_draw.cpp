#include "nfs/graphics/fonts/font_renderer.hpp"
#include "nfs/graphics/resources/eagl_assets.hpp"
namespace nfs::graphics::font_renderer {

void __cdecl begin(fonts::Font *font) {
    auto *object = reinterpret_cast<Dword *>(font);
    vertex_count = index_count = 0;
    backend::set_vertex_handle(324);
    auto *texture = reinterpret_cast<Dword *>(object[112 / 4]);
    texture[24 / 4] = object[72 / 4] == 1 ? 2 : 1;
    if (object[76 / 4] == 0) {
        geoprim::depth_write(state, 0);
        geoprim::depth_method(state, 8);
    } else if (object[76 / 4] == 1) {
        geoprim::depth_write(state, 1);
        geoprim::depth_method(state, 4);
    }
    geoprim::apply(state);
    backend::unresolved::texture_state();
    backend::apply_texture_delta(&stages);
    tar::bind(reinterpret_cast<Dword *>(object[112 / 4]));
    if (backend::unresolved::lighting_enabled) {
        backend::unresolved::lighting_enabled = 0;
        backend::com(backend_device, 200, Dword(137), Dword(0));
    }
    if (backend::unresolved::fog_enabled) {
        backend::unresolved::fog_enabled = 0;
        backend::com(backend_device, 200, Dword(28), Dword(0));
    }
    const auto *floats = reinterpret_cast<const float *>(object);
    depth = floats[40 / 4];
    reciprocal_w = floats[44 / 4];
    scale_x = floats[56 / 4];
    scale_y = floats[60 / 4];
    offset_u = floats[48 / 4];
    offset_v = floats[52 / 4];
    inverse_width = floats[124 / 4];
    inverse_height = floats[120 / 4];
    colour = object[32 / 4];
}

void __cdecl end(fonts::Font *) {
    backend::index_base(0);
    if (vertex_count)
        backend::draw_indexed_user(0, vertex_count, reinterpret_cast<Dword>(vertices), 28,
                                   index_count, reinterpret_cast<Dword>(indices));
}
} // namespace nfs::graphics::font_renderer
