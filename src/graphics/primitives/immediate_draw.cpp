#include "nfs/graphics/primitives/immediate_draw.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include "../actors/model_render_runtime.hpp"
#include <cstdlib>
namespace nfs::graphics::immediate {
backend::TextureStageState gouraud_stages{}, textured_stages{};  


void construct_textured_draw_stages() {
    backend::construct_texture_stages(&textured_stages);
}

void destroy_textured_draw_stages() {
    backend::destroy_texture_stages(&textured_stages);
}
// Initialize the draw buffer and register its destructor.
void initialize_textured_draw_stages() {
    construct_textured_draw_stages();
    std::atexit(destroy_textured_draw_stages);
}

void construct_gouraud_draw_stages() {
    backend::construct_texture_stages(&gouraud_stages);
}

void destroy_gouraud_draw_stages() {
    backend::destroy_texture_stages(&gouraud_stages);
}
// Initialize the draw buffer and register its destructor.
void initialize_gouraud_draw_stages() {
    construct_gouraud_draw_stages();
    std::atexit(destroy_gouraud_draw_stages);
}


Dword *construct(Dword *object) {
    object[0] = 1;
    lifecycle::construction::render_state(object + 1);
    object[72 / 4] = object[76 / 4] = object[80 / 4] = 0;
    model_builder_construct(object + 180 / 4);
    return object;
}

Dword *construct_textured(Dword *object) {
    object[0] = 1;
    lifecycle::construction::render_state(object + 1);
    object[72 / 4] = object[76 / 4] = object[80 / 4] = object[148 / 4] = 0;
    return object;
}

void destroy(Dword *object) {
    if (object[76 / 4]) {
        eagl::deallocator(reinterpret_cast<void *>(object[76 / 4]), 16);
        object[76 / 4] = 0;
    }
    if (object[72 / 4]) {
        resources::release_vertex(object + 72 / 4, 0);
        object[72 / 4] = 0;
    }
    model_builder_destroy(object + 180 / 4);
    geoprim::destroy(object + 1);
}

void destroy_textured(Dword *object) {
    if (object[76 / 4])
        eagl::deallocator(reinterpret_cast<void *>(object[76 / 4]), 24);
    if (object[72 / 4])
        resources::release_vertex(object + 72 / 4, 0);
    geoprim::destroy(object + 1);
}
namespace {
void matrix(Dword *object) {
    for (unsigned i = 0; i != 16; ++i)
        object[84 / 4 + i] = i % 5 == 0 ? 1065353216 : 0;
}
void disable_lighting() {
    if (backend::unresolved::lighting_enabled) {
        backend::unresolved::lighting_enabled = 0;
        backend::com(backend_device, 200, Dword(137), Dword(0));
    }
}
void disable_fog() {
    if (backend::unresolved::fog_enabled) {
        backend::unresolved::fog_enabled = 0;
        backend::com(backend_device, 200, Dword(28), Dword(0));
    }
}
void draw_state(Dword *object, Dword vertex_format, const backend::TextureStageState *stages) {
    backend::set_software_processing(!state::supports_hardware_fixed_function_processing());
    disable_fog();
    disable_lighting();
    backend::set_vertex_handle(vertex_format);
    backend::unresolved::texture_state();
    geoprim::apply(object + 1);
    backend::apply_texture_delta(stages);
}
} // namespace

Dword initialize(Dword *object) {
    object[76 / 4] =
        reinterpret_cast<Dword>(eagl::allocator(262144, "EAGL::DrawGouraud::GouraudVertex new[]"));
    object[72 / 4] =
        reinterpret_cast<Dword>(buffers::create_vertex(262144, 16, 520, 66, 1, 0));
    matrix(object);
    disable_lighting();
    geoprim::texture_enable(object + 1, 0);
    geoprim::shading(object + 1, 1);
    geoprim::alpha_blend_mode(object + 1, 1);
    geoprim::culling(object + 1, 0);
    gouraud_stages.values[3][0] = 0;
    gouraud_stages.values[0][0] = 3;
    gouraud_stages.values[4][0] = 3;
    gouraud_stages.values[7][0] = 0;
    return 1;
}

Dword initialize_textured(Dword *object) {
    object[76 / 4] =
        reinterpret_cast<Dword>(eagl::allocator(393216, "EAGL::DrawGouraud::TexturedVertex new[]"));
    object[72 / 4] =
        reinterpret_cast<Dword>(buffers::create_vertex(393216, 24, 520, 322, 1, 0));
    matrix(object);
    disable_lighting();
    geoprim::texture_enable(object + 1, 1);
    geoprim::texture_coordinates(object + 1, 1);
    geoprim::shading(object + 1, 1);
    geoprim::alpha_blend_mode(object + 1, 0);
    geoprim::culling(object + 1, 0);
    textured_stages.values[2][0] = 2;
    textured_stages.values[3][0] = 0;
    textured_stages.values[6][0] = 2;
    textured_stages.values[7][0] = 0;
    textured_stages.values[0][0] = 4;
    textured_stages.values[4][0] = 4;
    return 1;
}

void flush(Dword *object) {
    if (static_cast<int>(object[80 / 4]) > 0) {
        draw_state(object, 66, &gouraud_stages);
        geoprim::current_primitive = object[0];
        backend::com(backend_device, 148, Dword(256), object + 84 / 4);
        backend::com(backend_device, 148, Dword(2), backend::view_matrix);
        backend::com(backend_device, 148, Dword(3), backend::projection_matrix);
        model_render::identity_transforms = 0;
        backend::draw_user(object[80 / 4], object[76 / 4], 16);
    }
    object[80 / 4] = 0;
}

void flush_textured(Dword *object) {
    if (static_cast<int>(object[80 / 4]) > 0) {
        draw_state(object, 322, &textured_stages);
        tar::bind(reinterpret_cast<Dword *>(object[148 / 4]));
        geoprim::current_primitive = object[0];
        backend::com(backend_device, 148, Dword(256), object + 84 / 4);
        backend::com(backend_device, 148, Dword(2), backend::view_matrix);
        backend::com(backend_device, 148, Dword(3), backend::projection_matrix);
        model_render::identity_transforms = 0;
        backend::draw_user(object[80 / 4], object[76 / 4], 24);
    }
    object[80 / 4] = 0;
}

void set_primitive(Dword *object, Dword primitive) {
    if (primitive != object[0]) {
        flush(object);
        object[0] = primitive;
        geoprim::primitive(object + 1, primitive);
    }
}

void end_primitive(Dword *object) {
    if (static_cast<int>(object[80 / 4] + 3) < 16385 && object[0] != 5 && object[0] != 3 &&
        object[0] != 6 && object[0] != 2)
        return;
    flush(object);
}

void set_texture(Dword *object, Dword texture) {
    flush_textured(object);
    object[148 / 4] = texture;
}
} // namespace nfs::graphics::immediate
