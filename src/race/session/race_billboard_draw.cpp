#include "race_world_draw.hpp"
#include "../../world/resources/track_render_resources.hpp"
#include "nfs/graphics/math/eagl_math.hpp"
#include "nfs/graphics/primitives/immediate_draw.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
#include "../../gui/lifecycle/gui_images.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
#include <cmath>
#include <cstring>
namespace nfs::game::world::camera {
void active_camera_position(void *, float *);
}
namespace nfs::game::world::vehicle_render {
void matrix_from_axes(float *, const float *, const float *, int, int);
}
namespace nfs::game::world::actor_resources {
void append_actor_scale(float *, float, float, float, float);
}
namespace nfs::game::world::track_render {
float glow_visibility[2] = {1.f, 1.f};

void draw_light_glow(void *glow, void *director, const float *rotation, const float *position,
                      const float *direction) {
    auto E = [](float value) { return static_cast<long double>(value); };
    const auto now = static_cast<long double>(
                         std::int32_t(graphics::unresolved::camera_state)) *
                     0.01666666753590106964111328125f;
    const float period = at<float>(glow, 0);
    if (period > 0) {
        const auto deadline = E(at<float>(glow, 216)) + period;
        if (!__builtin_isnan(deadline) && !(now < deadline)) {
            at<float>(glow, 216) = float(now);
            at<Byte>(glow, 8) = (at<Byte>(glow, 8) + 1) & 1;
        }
    }
    if (at<Byte>(glow, 8) == 1)
        return;
    float eye[3];
    camera::active_camera_position(director, eye);
    eye[2] = -eye[2];
    float location[3] = {position[0], position[1], position[2]};
    float facing[3] = {float(E(location[0]) - eye[0]), float(E(location[1]) - eye[1]),
                        float(E(location[2]) - eye[2])};
    const float up[3] = {0, 1, 0}, origin[3] = {0, 0, 0};
    const float distance = float(runtime::hardware::vector_length(facing));
    runtime::hardware::normalize_collision(facing);
    const Byte directional = at<Byte>(glow, 228);
    float size = at<float>(glow, 4);
    auto set_alpha = [&](Dword alpha) {
        at<Dword>(glow, 212) =
            (((alpha << 8 | at<Byte>(glow, 224)) << 8 | at<Byte>(glow, 225)) << 8) |
            at<Byte>(glow, 226);
    };
    if (directional) {
        float axes[16], normal[3];
        graphics::math::from_compact_transform(axes, rotation, origin);
        graphics::math::transform_position(direction, axes, normal);
        auto gain = runtime::hardware::dot_product(facing, normal);
        if (gain < E(0))
            gain = E(0);
        set_alpha(Dword(static_cast<long long>(gain * at<float>(glow, 232) * 255.f)));
    } else if (at<Byte>(glow, 236)) {
        size = float(E(distance) / E(glow_scale) + glow_distance);
        auto gain = E(glow_fade) / E(distance);
        if (E(1) < gain)
            gain = E(1);
        set_alpha(Dword(static_cast<long long>(gain * 255.f)));
    }
    const Byte relative = at<Byte>(glow, 237);
    if (relative)
        for (unsigned axis = 0; axis < 3; ++axis)
            location[axis] = float(E(eye[axis]) + location[axis]);
    float matrix[16];
    vehicle_render::matrix_from_axes(matrix, up, facing, 0, 4);
    actor_resources::append_actor_scale(matrix, size, size, size, 1);
    graphics::math::append_translation(matrix, location[0], location[1], location[2]);
    using namespace graphics;
    immediate::set_texture(bindings::single_draw_texture, at<Dword>(glow, 220));
    std::memcpy(bindings::single_draw_texture + 84 / 4, matrix, 64);
    std::memcpy(bindings::single_draw + 84 / 4, matrix, 64);
    if (relative) {
        const Dword player = at<Dword>(director, 304);
        glow_visibility[player] = 0;
        int alpha = int(Dword(static_cast<long long>(E(glow_visibility[player]) * 255.f)));
        if (alpha > 255)
            alpha = 255;
        set_alpha(Dword(alpha));
    }
    if (directional) {
        float normal[3];
        math::from_compact_transform(matrix, rotation, origin);
        math::transform_position(direction, matrix, normal);
        vehicle_render::matrix_from_axes(matrix, up, normal, 0, 4);
        actor_resources::append_actor_scale(matrix, size, size, size, 1);
        math::append_translation(matrix, position[0], position[1], position[2]);
    }
    if (gui::engine::images::suspend_drawing())
        return;
    immediate::set_textured_primitive(bindings::single_draw_texture, 4);
    constexpr Dword vertices[] = {20, 36, 52, 52, 68, 20};
    for (unsigned index = 0; index < 6; ++index)
        immediate::textured_vertex(bindings::single_draw_texture,
                                   reinterpret_cast<float *>(offset(glow, vertices[index])),
                                   &at<Dword>(glow, 212),
                                   reinterpret_cast<float *>(offset(glow, 116 + index * 16)));
    immediate::end_textured_primitive(bindings::single_draw_texture);
    immediate::flush_textured(bindings::single_draw_texture);
}

void draw_compartment_glows(void *glows, const float *rotation) {
    if (!at<Dword>(glows, 68))
        return;
    graphics::immediate::set_render_state(graphics::bindings::single_draw_texture,
                                          static_cast<Dword *>(glows));
    graphics::backend::set_depth_write(graphics::context, 0);
    const float forward[3] = {0, 0, 1};
    void *director = tick_services[0];
    int count = at<int>(glows, 68);
    auto *instance = offset(at<void *>(glows, 72), 8);
    void *definitions = at<void *>(glows, 80);
    while (count-- > 0) {
        void *glow = offset(definitions, at<Dword>(instance, 0) * 248);
        draw_light_glow(glow, director, rotation,
                         reinterpret_cast<const float *>(instance + 8), forward);
        instance += 28;
    }
    graphics::backend::set_depth_write(graphics::context, 1);
}
} // namespace nfs::game::world::track_render
