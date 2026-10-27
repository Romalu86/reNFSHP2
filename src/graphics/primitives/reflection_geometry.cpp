#include "nfs/core/services/service_lifecycle.hpp"
#include "nfs/graphics/skids/skid_lifecycle.hpp"
#include "nfs/world/resources/runtime_resources.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include "../../math/precision/extended_float.hpp"
#include <cmath>
namespace nfs::graphics::immediate {
void *draw_array_vertices(Dword *);
}
namespace nfs::services::pending {
using game::runtime::at;
using game::runtime::bytes;
namespace skids = game::runtime::skids::lifecycle;

void *construct_reflection_item(void *storage) {
    auto *primitive = static_cast<Dword *>(storage);
    auto *method = static_cast<Dword *>(eagl::allocator(44, "EAGL::Rendermethod new"));
    primitive[0] =
        reinterpret_cast<Dword>(method ? skids::pending::construct_render_method(
                                            method, skids::pending::render_method_definition)
                                      : nullptr);
    const char *names[] = {"EAGL::ViewPort::gpModelViewProjectionMatrix",
                           "EAGL::ViewPort::gpModelMatrix", "EAGL::ViewPort::gpViewMatrix",
                           "EAGL::ViewPort::gpProjectionMatrix",
                           "&EAGL::RenderMethodConstants::gFogParameters"};
    for (unsigned index = 0; index < 5; ++index) {
        Byte found;
        primitive[4 + index * 2] = eagl::pool_lookup(&eagl::external_symbols, names[index], &found);
        primitive[3 + index * 2] = 1;
    }
    return storage;
}

void destroy_reflection_item(void *primitive) {
    if (Dword *method = at<Dword *>(primitive, 0)) {
        skids::pending::destroy_render_method(method);
        eagl::deallocator(method, 44);
    }
}

void project_reflection_vertex(void *owner, Dword face, int column, int row, const float *origin,
                               float *position, float *uv, Dword *colour, Byte rim) {
    constexpr float step = 0.2857142984867095947265625f;
    const auto format = kernel::numeric::current_format();
    auto E = [&](float value) { return kernel::numeric::Extended(value, format); };
    auto I = [&](int value) { return kernel::numeric::Extended::integer(value, format); };
    float point[3] = {origin[0], origin[1], origin[2]};
    switch (face) {
    case 0:
        point[2] = float(I(column) * step + point[2]);
        {
            const auto x = I(row) * step + point[0];
            point[0] = float(x);
            uv[0] = float(x * .5f + .5f);
        }
        uv[1] = float(E(point[2]) * .5f + .5f);
        break;
    case 1:
    case 3:
        point[2] = float(I(column) * step + point[2]);
        point[1] = float(E(point[1]) - I(row) * step);
        uv[0] = face == 1 ? float(E(point[2]) * .5f + .5f)
                          : float(E(1.f) - (E(point[2]) * .5f + .5f));
        uv[1] = float(E(point[1]) * .5f + .5f);
        break;
    case 2:
    case 4:
        point[0] = float(I(column) * step + point[0]);
        point[1] = float(E(point[1]) - I(row) * step);
        uv[0] = float(E(point[0]) * .5f + .5f);
        uv[1] = float(E(point[1]) * .5f + .5f);
        break;
    case 5:
        point[0] = float(I(column) * step + point[0]);
        point[2] = float(I(row) * step + point[2]);
        uv[0] = float(E(point[0]) * .5f + .5f);
        uv[1] = float(E(1.f) - (E(point[2]) * .5f + .5f));
        break;
    }
    float normal[3];
    game::runtime::normalize_vector(point, normal);
    const auto z = E(normal[2]) + 1.f;
    const auto yy = E(normal[1]) * normal[1];
    const auto xx = E(normal[0]) * normal[0];
    const auto xy = yy + xx;
    const auto zz = z * z;
    const auto inverse = E(1.f) / (xy + zz).square_root();
    float projected[3] = {float(E(normal[0]) * inverse), float(inverse * normal[1]), 0};
    if (rim) {
        game::runtime::normalize_vector(projected, normal);
        projected[0] = normal[0];
        projected[1] = normal[1];
        projected[2] = normal[2];
    }
    const Dword packed = (((at<Dword>(owner, 8) | 4294967040u) << 8) | (at<Dword>(owner, 12) & 255))
                            << 8 |
                        (at<Dword>(owner, 16) & 255);
    const auto projected_x = E(projected[0]) * 140.8000030517578125f + 128.f;
    const auto projected_y = E(projected[1]) * 140.8000030517578125f + 128.f;
    position[0] = float(projected_x);
    position[2] = -1;
    position[1] = float(projected_y);
    if (rim)
        uv[0] = uv[1] = .5f;
    *colour = packed;
}

void build_reflection_face(void *owner, int face, const float *origin) {
    auto *array = reinterpret_cast<Dword *>(bytes(at<void *>(owner, 512)) + face * 128);
    Byte *vertices = static_cast<Byte *>(graphics::immediate::draw_array_vertices(array));
    if (auto *indices = at<std::uint16_t *>(array, 124))
        for (int index = 0; index < 294; index += 6) {
            const auto first = std::uint16_t(index / 6 * 4);
            indices[index] = first;
            indices[index + 1] = first + 1;
            indices[index + 2] = first + 2;
            indices[index + 3] = first + 2;
            indices[index + 4] = first + 3;
            indices[index + 5] = first;
        }
    auto vertex = [&](int column, int row, Byte rim) {
        project_reflection_vertex(
            owner, Dword(face), column, row, origin, reinterpret_cast<float *>(vertices),
            reinterpret_cast<float *>(vertices + 16), reinterpret_cast<Dword *>(vertices + 12), rim);
        vertices += 24;
    };
    if (face != 4) {
        for (int row = 0; row < 7; ++row)
            for (int column = 0; column < 7; ++column) {
                vertex(column, row, 0);
                vertex(column + 1, row, 0);
                vertex(column + 1, row + 1, 0);
                vertex(column, row + 1, 0);
            }
        return;
    }
    for (int row = 0; row < 7; ++row) {
        vertex(0, row, 1);
        vertex(0, row + 1, 1);
        vertex(0, row + 1, 0);
        vertex(0, row, 0);
        vertex(7, row, 1);
        vertex(7, row + 1, 1);
        vertex(7, row + 1, 0);
        vertex(7, row, 0);
    }
    for (int column = 0; column < 7; ++column) {
        vertex(column, 1, 1);
        vertex(column + 1, 1, 1);
        vertex(column + 1, 1, 0);
        vertex(column, 1, 0);
        vertex(column, 7, 1);
        vertex(column + 1, 7, 1);
        vertex(column + 1, 7, 0);
        vertex(column, 7, 0);
    }
}

void initialize_reflection_camera(void *owner) {
    Dword *render = reinterpret_cast<Dword *>(bytes(owner) + 520);
    graphics::geoprim::primitive(render, 4);
    graphics::geoprim::shading(render, 1);
    graphics::geoprim::texture_enable(render, 1);
    graphics::geoprim::transparency(render, 1);
    graphics::geoprim::alpha_blend_mode(render, 0);
    graphics::geoprim::depth_method(render, 8);
    graphics::geoprim::alpha_reference(render, 0);
    graphics::geoprim::alpha_method(render, 5);
    graphics::geoprim::alpha_test(render, 1);
    auto allocate_array = [](Dword size, Dword stride, void (*constructor)(void *),
                             void (*destructor)(void *), const char *label) -> void * {
        void *allocation = lifecycle::allocate(size, label, lifecycle::cars_pool);
        if (!allocation)
            return nullptr;
        at<Dword>(allocation, 0) = 6;
        void *array = bytes(allocation) + 4;
        try {
            lifecycle::construction::construct_array(array, stride, 6, constructor, destructor);
        } catch (...) {
            gui::application_release(allocation);
            throw;
        }
        return array;
    };
    at<void *>(owner, 512) = allocate_array(
        772, 128, [](void *p) { skids::construct_array(static_cast<Dword *>(p)); },
        destroy_reflection_mesh, "REFLECTION FACE ARRAY");
    at<void *>(owner, 516) = allocate_array(508, 84,
                                              [](void *storage) { construct_reflection_item(storage); },
                                              destroy_reflection_item, "REFLECTION FACE PRIMS");
    for (int face = 0; face < 6; ++face)
        skids::pending::allocate_vertices(
            reinterpret_cast<Dword *>(bytes(at<void *>(owner, 512)) + face * 128), 294, 4,
            reinterpret_cast<Dword *>(bytes(at<void *>(owner, 516)) + face * 84), render,
            at<Dword *>(at<void *>(owner, 400), face * 108 + 8), 8,
            "REFLECTION DRAWARRAY VTX DATA", lifecycle::cars_pool);
    const float positive_z[] = {-1, 1, 1}, negative_x[] = {-1, 1, -1}, positive_x[] = {1, 1, -1};
    const float negative_y[] = {-1, -1, -1}, positive_y[] = {-1, 1, -1}, negative_z[] = {-1, 1, -1};
    build_reflection_face(owner, 2, positive_z);
    build_reflection_face(owner, 1, negative_x);
    build_reflection_face(owner, 3, positive_x);
    build_reflection_face(owner, 0, negative_y);
    build_reflection_face(owner, 5, positive_y);
    build_reflection_face(owner, 4, negative_z);
}
} // namespace nfs::services::pending
