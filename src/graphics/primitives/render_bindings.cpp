#include "nfs/graphics/primitives/immediate_draw.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include "nfs/core/lifecycle/kernel_lifecycle.hpp"
#include <initializer_list>

namespace nfs::graphics::bindings {
Dword *triangle_texture_alpha = nullptr;
Dword *triangle_texture = nullptr;
Dword *triangle_alpha = nullptr;
Dword *triangle = nullptr;
Dword *line_alpha = nullptr;
Dword *line = nullptr;
Dword *single_draw = nullptr;
Dword *single_draw_texture = nullptr;
Dword previous_depth_method = 0;
Dword enabled = 1;
Dword matrix[16]{};
namespace {
Dword *allocate_state(const char *name) {
    auto *state = static_cast<Dword *>(lifecycle::allocate(68, name, gui::application_pool));
    if (state)
        lifecycle::construction::render_state(state);
    return state;
}
void release_state(Dword *&state) {
    if (state) {
        geoprim::destroy(state);
        eagl::deallocator(state, 68);
    }
    state = nullptr;
}
} // namespace

void initialize() {
    triangle_texture_alpha = allocate_state("gStateTriTexAlpha");
    geoprim::read_depth_method(triangle_texture_alpha, &previous_depth_method);
    geoprim::shading(triangle_texture_alpha, 1);
    geoprim::primitive(triangle_texture_alpha, 4);
    geoprim::texture_enable(triangle_texture_alpha, 1);
    geoprim::alpha_blend_mode(triangle_texture_alpha, 1);
    geoprim::transparency(triangle_texture_alpha, 1);
    geoprim::alpha_reference(triangle_texture_alpha, 0);
    geoprim::alpha_method(triangle_texture_alpha, 5);
    geoprim::alpha_test(triangle_texture_alpha, 1);

    triangle_texture = allocate_state("gStateTriTexNoAlpha");
    geoprim::shading(triangle_texture, 1);
    geoprim::primitive(triangle_texture, 4);
    geoprim::texture_enable(triangle_texture, 1);
    geoprim::alpha_test(triangle_texture, 0);

    triangle_alpha = allocate_state("gStateTriNoTexAlpha");
    geoprim::shading(triangle_alpha, 1);
    geoprim::primitive(triangle_alpha, 4);
    geoprim::texture_enable(triangle_alpha, 0);
    geoprim::alpha_test(triangle_alpha, 0);
    geoprim::alpha_blend_mode(triangle_alpha, 1);
    geoprim::transparency(triangle_alpha, 1);
    geoprim::alpha_method(triangle_alpha, 8);

    triangle = allocate_state("gStateTriNoTexNoAlpha");
    geoprim::shading(triangle, 1);
    geoprim::primitive(triangle, 4);
    geoprim::texture_enable(triangle, 0);
    geoprim::alpha_test(triangle, 0);

    line_alpha = allocate_state("gStateLineAlpha");
    geoprim::shading(line_alpha, 1);
    geoprim::primitive(line_alpha, 2);
    geoprim::texture_enable(line_alpha, 0);
    geoprim::alpha_test(line_alpha, 0);
    geoprim::alpha_blend_mode(line_alpha, 1);
    geoprim::transparency(line_alpha, 1);
    geoprim::alpha_method(line_alpha, 8);

    line = allocate_state("gStateLineNoAlpha");
    geoprim::shading(line, 1);
    geoprim::primitive(line, 2);
    geoprim::texture_enable(line, 0);
    geoprim::alpha_test(line, 0);

    auto *draw =
        static_cast<Dword *>(lifecycle::allocate(256, "gpEAGLSingleDraw", gui::application_pool));
    single_draw = draw ? immediate::construct(draw) : nullptr;
    auto *textured = static_cast<Dword *>(
        lifecycle::allocate(152, "gpEAGLSingleDrawTex", gui::application_pool));
    single_draw_texture = textured ? immediate::construct_textured(textured) : nullptr;
    immediate::initialize(single_draw);
    immediate::initialize_textured(single_draw_texture);
    const Dword initial_matrix[16] = {1065353216, 0, 0,          0, 0, 1065353216, 0, 0,
                                     0,          0, 1065353216, 0, 0, 0,          0, 1065353216};
    for (unsigned i = 0; i != 16; ++i)
        matrix[i] = initial_matrix[i];
    enabled = 1;
    for (Dword *state :
         {triangle_texture_alpha, triangle_texture, triangle_alpha, triangle, line_alpha, line})
        geoprim::depth_method(state, 8);
}


void shutdown() {
    release_state(triangle_texture_alpha);
    release_state(triangle_texture);
    release_state(triangle_alpha);
    release_state(triangle);
    release_state(line_alpha);
    release_state(line);
    if (single_draw) {
        immediate::destroy(single_draw);
        eagl::deallocator(single_draw, 256);
    }
    single_draw = nullptr;
    if (single_draw_texture) {
        immediate::destroy_textured(single_draw_texture);
        eagl::deallocator(single_draw_texture, 152);
    }
    single_draw_texture = nullptr;
}
} // namespace nfs::graphics::bindings
