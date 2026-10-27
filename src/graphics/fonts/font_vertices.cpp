#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "nfs/graphics/fonts/font_renderer.hpp"
#include "nfs/graphics/resources/eagl_assets.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include "../../gui/lifecycle/gui_render_numeric_x87.hpp"
#include <cstdlib>
#include <cstring>
namespace gui_numeric = ::nfs::gui::retained;
namespace nfs::graphics::font_renderer {
Dword state[17]{};
backend::TextureStageState stages{};
float inverse_height = 0, inverse_width = 0, scale_y = 0, scale_x = 0;
float reciprocal_w = 0, depth = 0, offset_u = 0, offset_v = 0;
Dword colour = 0, vertex_count = 0, index_count = 0, references = 0;
Vertex *vertices = nullptr;
std::uint16_t *indices = nullptr;
Dword *palette = nullptr;
static std::uint8_t destruction_flags{};  

void construct_font_render_state() {
    lifecycle::construction::render_state(state);
}

void destroy_font_render_state() {
    if (!(destruction_flags & 1)) {
        destruction_flags |= 1;
        geoprim::destroy(state);
    }
}
// Initialize the vertex storage and register its destructor.
void initialize_font_render_state() {
    construct_font_render_state();
    std::atexit(destroy_font_render_state);
}

void construct_font_texture_stages() {
    backend::construct_texture_stages(&stages);
}

void destroy_font_texture_stages() {
    if (!(destruction_flags & 2)) {
        destruction_flags |= 2;
        backend::destroy_texture_stages(&stages);
    }
}
// Initialize the vertex storage and register its destructor.
void initialize_font_texture_stages() {
    construct_font_texture_stages();
    std::atexit(destroy_font_texture_stages);
}
namespace {
template <class T> T read(const std::uint8_t *at) {
    T result;
    std::memcpy(&result, at, sizeof result);
    return result;
}
static __forceinline void append_index(Dword offset, Dword delta) {
    volatile std::uint16_t *array = gui_numeric::read_field(indices);
    const Dword position = gui_numeric::read_field(index_count);
    const Dword value = gui_numeric::read_field(vertex_count) + delta;
    array[position + offset] = static_cast<std::uint16_t>(value);
}
template <bool Right, bool Bottom>
static __forceinline Dword append_vertex(const std::uint8_t *glyph, long double bearing_x,
                                        long double bearing_y, float u, float v, long double width,
                                        float height, float x, float y, Dword first_index) {
    auto vertex = []() {
        return gui_numeric::read_field(vertices) + gui_numeric::read_field(vertex_count);
    };
    if constexpr (Right && Bottom)
        (gui_numeric::read_field(vertices) + first_index)->x =
            float((gui_numeric::integer(gui_numeric::read_field(glyph[2])) + bearing_x) *
                      gui_numeric::number(gui_numeric::read_field(scale_x)) +
                  x);
    else if constexpr (Right)
        (gui_numeric::read_field(vertices) + first_index)->x =
            float((width + bearing_x) * gui_numeric::number(gui_numeric::read_field(scale_x)) + x);
    else
        (gui_numeric::read_field(vertices) + first_index)->x =
            float(gui_numeric::number(gui_numeric::read_field(scale_x)) * bearing_x + x);
    if constexpr (Right && Bottom)
        vertex()->y = float((gui_numeric::integer(gui_numeric::read_field(glyph[3])) + bearing_y) *
                                gui_numeric::number(gui_numeric::read_field(scale_y)) +
                            y);
    else if constexpr (Bottom)
        vertex()->y = float((gui_numeric::number(height) + bearing_y) *
                                gui_numeric::number(gui_numeric::read_field(scale_y)) +
                            y);
    else
        vertex()->y = float(gui_numeric::number(gui_numeric::read_field(scale_y)) * bearing_y + y);
    vertex()->z = gui_numeric::read_field(depth);
    vertex()->reciprocal_w = gui_numeric::read_field(reciprocal_w);
    vertex()->colour = gui_numeric::read_field(colour);
    if constexpr (Right)
        vertex()->u = float((gui_numeric::number(gui_numeric::read_field(offset_u)) + width + u) *
                            gui_numeric::number(gui_numeric::read_field(inverse_width)));
    else
        vertex()->u = float((gui_numeric::number(gui_numeric::read_field(offset_u)) + u) *
                            gui_numeric::number(gui_numeric::read_field(inverse_width)));
    if constexpr (Bottom)
        vertex()->v = float((gui_numeric::number(gui_numeric::read_field(offset_v)) + height + v) *
                            gui_numeric::number(gui_numeric::read_field(inverse_height)));
    else
        vertex()->v = float((gui_numeric::number(gui_numeric::read_field(offset_v)) + v) *
                            gui_numeric::number(gui_numeric::read_field(inverse_height)));
    const Dword next_index = gui_numeric::read_field(vertex_count) + Dword(1);
    gui_numeric::write_field(vertex_count, next_index);
    return next_index;
}
void append(const std::uint8_t *glyph, float x, float y) {
    append_index(0, 0);
    append_index(1, 1);
    append_index(2, 3);
    append_index(3, 3);
    append_index(4, 1);
    append_index(5, 2);
    gui_numeric::write_field(index_count, gui_numeric::read_field(index_count) + Dword(6));
    const long double bearing_x = gui_numeric::integer(static_cast<std::int8_t>(glyph[9]));
    const long double bearing_y = gui_numeric::integer(static_cast<std::int8_t>(glyph[10]));
    const float u = read<std::uint16_t>(glyph + 4), v = read<std::uint16_t>(glyph + 6);
    const long double width = gui_numeric::integer(glyph[2]);
    const float height = glyph[3];
    Dword first_index = gui_numeric::read_field(vertex_count);
    first_index = append_vertex<false, false>(glyph, bearing_x, bearing_y, u, v, width, height, x,
                                              y, first_index);
    first_index = append_vertex<true, false>(glyph, bearing_x, bearing_y, u, v, width, height, x, y,
                                             first_index);
    first_index = append_vertex<true, true>(glyph, bearing_x, bearing_y, u, v, width, height, x, y,
                                            first_index);
    append_vertex<false, true>(glyph, bearing_x, bearing_y, u, v, width, height, x, y, first_index);
}
} // namespace

void __cdecl draw(fonts::Font *, const std::uint8_t *glyph, float x, float y) {
    append(glyph, x, y);
}

void __cdecl batch(fonts::Font *, const fonts::DrawItem *items, int count) {
    const std::uint8_t *glyph = items->glyph;
    if (count > 0) {
        do {
            append(glyph, items->x, items->y);
            ++items;
            --count;
            glyph = items->glyph;
        } while (count);
    }
}
} // namespace nfs::graphics::font_renderer

namespace nfs::graphics::immediate {
void initialize_textured_draw_stages();
void initialize_gouraud_draw_stages();
} // namespace nfs::graphics::immediate
// Preserve the relative order of the draw-buffer and font-vertex initializers.
 
NFS_KERNEL_INITIALIZER(startup_initialize_textured_draw_stages, ".CRT$XCU_NFS_006903BC",
                       nfs::graphics::immediate::initialize_textured_draw_stages);
NFS_KERNEL_INITIALIZER(startup_initialize_gouraud_draw_stages, ".CRT$XCU_NFS_006903C0",
                       nfs::graphics::immediate::initialize_gouraud_draw_stages);
NFS_KERNEL_INITIALIZER(startup_initialize_font_render_state, ".CRT$XCU_NFS_00690418",
                       nfs::graphics::font_renderer::initialize_font_render_state);
NFS_KERNEL_INITIALIZER(startup_initialize_font_texture_stages, ".CRT$XCU_NFS_0069041C",
                       nfs::graphics::font_renderer::initialize_font_texture_stages);
