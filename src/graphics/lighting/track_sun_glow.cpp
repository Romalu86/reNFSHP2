#include "../../world/resources/track_render_resources.hpp"
#include "nfs/graphics/resources/eagl_assets.hpp"
#include "nfs/world/resources/runtime_resources.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include "../../math/precision/extended_float.hpp"
#include <cstring>
#include <initializer_list>
namespace nfs::game::world::track_render {

void *construct_sun_glow(void *owner) {
    std::memset(owner, 0, 24);
    construct_light_glow(offset(owner, 24));
    try {
        lifecycle::construction::render_state(reinterpret_cast<Dword *>(offset(owner, 272)));
        try {
            lifecycle::construction::render_state(reinterpret_cast<Dword *>(offset(owner, 340)));
        } catch (...) {
            graphics::geoprim::base_destroy(reinterpret_cast<Dword *>(offset(owner, 272)));
            throw;
        }
    } catch (...) {
        destroy_light_glow(offset(owner, 24));
        throw;
    }
    at<Byte>(owner, 408) = 0;
    at<Dword>(owner, 412) = 0;
    return owner;
}

void bind_sun_glow(void *owner, const float *position, void *image, float size,
                   const float *colour) {
    using kernel::numeric::Extended;
    const auto format = kernel::numeric::current_format();
    auto X = [&](float x) { return Extended(x, format); };
    std::memcpy(owner, position, 12);
    runtime::physics::pending::normalize_collision_entry(static_cast<float *>(owner));
    at<Dword>(owner, 28) = at<Dword>(owner, 268) = 1065353216;
    at<float>(owner, 0) = float(X(at<float>(owner, 0)) * 2000.f);
    at<float>(owner, 4) = float(X(at<float>(owner, 4)) * 2000.f);
    at<float>(owner, 8) = float(X(at<float>(owner, 8)) * -2000.f);
    at<float>(owner, 40) = at<float>(owner, 36) = size;
    at<Dword>(owner, 24) = 0;
    at<Byte>(owner, 252) = 0;
    Byte channels[4];
    for (unsigned i = 0; i < 4; ++i) {
        channels[i] = Byte((X(colour[i]) * 255.f).truncate_to_integer());
        at<Byte>(owner, 248 + i) = channels[i];
    }
    at<float>(owner, 256) = float(Extended::integer(channels[3], format) * 0.0039215688593685626983642578125f);
    at<Dword>(owner, 236) = (Dword(channels[3]) << 24) | (Dword(channels[0]) << 16) |
                            (Dword(channels[1]) << 8) | channels[2];
    constexpr Dword coordinates[24] = {0,          0,          1065353216, 1065353216, 0,
                                      1065353216, 1065353216, 1065353216, 1065353216, 1065353216,
                                      1065353216, 1065353216, 1065353216, 1065353216, 1065353216,
                                      1065353216, 1065353216, 0,          1065353216, 1065353216,
                                      0,          0,          1065353216, 1065353216};
    float negative_half = float(-(X(size) * .5f));
    for (unsigned vertex = 0; vertex < 6; ++vertex) {
        at<float>(owner, 44 + vertex * 16) = negative_half;
        at<Dword>(owner, 48 + vertex * 16) = 0;
        at<float>(owner, 52 + vertex * 16) = negative_half;
        at<Dword>(owner, 56 + vertex * 16) = 1065353216;
        std::memcpy(offset(owner, 140 + vertex * 16), coordinates + vertex * 4, 16);
    }
    for (unsigned field : {68u, 76u, 84u, 92u})
        at<float>(owner, field) = float(X(at<float>(owner, field)) + size);
    auto *texture =
        static_cast<Dword *>(lifecycle::allocate(48, "TrackLightGlow TAR", lifecycle::level_pool));
    if (texture) {
        try {
            graphics::tar::construct_shape(
                texture,
                reinterpret_cast<Dword *>(reinterpret_cast<Dword>(image) + at<Dword>(image, 20)));
        } catch (...) {
            gui::application_release(texture);
            throw;
        }
    }
    at<Dword *>(owner, 244) = texture;
    at<Dword>(owner, 264) = 0;
    at<float>(owner, 240) = float(
        Extended::integer(std::int32_t(graphics::unresolved::camera_state), format) *
        0.01666666753590106964111328125f);
    at<Byte>(owner, 32) = at<Byte>(owner, 260) = 0;
    at<Byte>(owner, 261) = 1;
    auto *state = reinterpret_cast<Dword *>(offset(owner, 272));
    graphics::geoprim::shading(state, 1);
    graphics::geoprim::texture_enable(state, 1);
    graphics::geoprim::culling(state, 0);
    graphics::geoprim::alpha_blend_mode(state, 2);
    graphics::geoprim::transparency(state, 1);
    graphics::geoprim::alpha_test(state, 0);
    graphics::geoprim::depth_method(state, 8);
    state = reinterpret_cast<Dword *>(offset(owner, 340));
    graphics::geoprim::shading(state, 1);
    graphics::geoprim::primitive(state, 4);
    graphics::geoprim::texture_enable(state, 0);
    graphics::geoprim::culling(state, 0);
    graphics::geoprim::alpha_blend_mode(state, 2);
    graphics::geoprim::transparency(state, 1);
    graphics::geoprim::alpha_test(state, 0);
    graphics::geoprim::depth_method(state, 4);
    at<Byte>(owner, 408) = 1;
}
} // namespace nfs::game::world::track_render
