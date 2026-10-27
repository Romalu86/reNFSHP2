#include "vehicle_renderable_frame.hpp"
#include "nfs/graphics/primitives/immediate_draw.hpp"
#include "nfs/world/resources/runtime_resources.hpp"
#include "../../gui/lifecycle/gui_images.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
#include <cstring>
#include <cmath>
namespace nfs::game::world::actor_resources {
void prepend_actor_rotation(float *, float, float, float, float);
void append_actor_scale(float *, float, float, float, float);
} // namespace nfs::game::world::actor_resources
namespace nfs::game::world::vehicle_render {
void matrix_from_axes(float *, const float *, const float *, int, int);

void update_vehicle_light_positions(void *lights, const float *matrix) {
    if (!at<void *>(lights, 68))
        return;
    for (Dword group = 0; group < 4; ++group) {
        const Dword base = 72 + group * 168;
        for (int index = 0; index < at<int>(lights, base + 20); ++index) {
            Dword rotation[9];
            float position[4];
            read_attachment_transform(at<void *>(lights, 768),
                                      at<Dword>(lights, base + 24 + Dword(index) * 4), rotation,
                                      reinterpret_cast<Dword *>(position));
            if (network::packets::rule_value(static_cast<Dword *>(subsystem(72)), 6) == 2 ||
                network::packets::rule_value(static_cast<Dword *>(subsystem(72)), 6) == 3)
                position[0] = -position[0];
            graphics::math::transform_position(
                position, matrix,
                reinterpret_cast<float *>(offset(lights, base + 80 + Dword(index) * 12)));
            at<Byte>(lights, base + 164) = group == 0 || group == 2 || group == 3;
        }
    }
}

void draw_vehicle_glow(void *glow, void *director, const float *rotation, const float *position,
                       const float *direction) {
    auto X = [](float value) { return static_cast<long double>(value); };
    const float period = at<float>(glow, 0);
    if (period > 0) {
        const auto now =
            static_cast<long double>(std::int32_t(graphics::unresolved::camera_state)) *
            0.01666666753590106964111328125f;
        if (!(now < X(at<float>(glow, 216)) + period) && !__builtin_isnan(now) &&
            !std::isnan(at<float>(glow, 216))) {
            at<float>(glow, 216) = float(now);
            at<Byte>(glow, 8) = (at<Byte>(glow, 8) + 1) & 1;
        }
    }
    if (at<Byte>(glow, 8) == 1)
        return;
    float eye[3];
    camera::active_camera_position(director, eye);
    eye[2] = -eye[2];
    float facing[3] = {float(X(position[0]) - eye[0]), float(X(position[1]) - eye[1]),
                       float(X(position[2]) - eye[2])};
    const float up[3] = {0, 1, 0}, origin[3] = {0, 0, 0};
    (void)runtime::hardware::vector_length(facing);
    runtime::hardware::normalize_collision(facing);
    float matrix[16];
    const Byte directional = at<Byte>(glow, 228);
    if (directional) {
        float normal_matrix[16], normal[3];
        graphics::math::from_compact_transform(normal_matrix, rotation, origin);
        graphics::math::transform_position(direction, normal_matrix, normal);
        auto facing_gain = runtime::hardware::dot_product(facing, normal);
        if (facing_gain < X(0))
            facing_gain = X(0);
        const auto alpha =
            static_cast<long long>(facing_gain * at<float>(glow, 232) * 255.f);
        at<Dword>(glow, 212) =
            (((Dword(alpha) << 8 | at<Byte>(glow, 224)) << 8 | at<Byte>(glow, 225)) << 8) |
            at<Byte>(glow, 226);
        graphics::math::from_compact_transform(matrix, rotation, origin);
        actor_resources::prepend_actor_rotation(matrix, 90, 1, 0, 0);
    } else
        matrix_from_axes(matrix, up, facing, 0, 4);
    const float size = at<float>(glow, 4);
    actor_resources::append_actor_scale(matrix, size, size, size, 1);
    graphics::math::append_translation(matrix, position[0], position[1], position[2]);
    graphics::immediate::set_texture(graphics::bindings::single_draw_texture, at<Dword>(glow, 220));
    std::memcpy(graphics::bindings::single_draw_texture + 84 / 4, matrix, 64);
    if (gui::engine::images::suspend_drawing())
        return;
    graphics::immediate::set_textured_primitive(graphics::bindings::single_draw_texture, 4);
    constexpr Dword vertices[6] = {20, 36, 52, 52, 68, 20};
    for (unsigned i = 0; i < 6; ++i)
        graphics::immediate::textured_vertex(
            graphics::bindings::single_draw_texture, reinterpret_cast<float *>(offset(glow, vertices[i])),
            reinterpret_cast<Dword *>(offset(glow, 212)),
            reinterpret_cast<float *>(offset(glow, 116 + i * 16)));
    graphics::immediate::end_textured_primitive(graphics::bindings::single_draw_texture);
    graphics::immediate::flush_textured(graphics::bindings::single_draw_texture);
}

void draw_vehicle_lights(void *lights, void *director, const float *rotation) {
    if (!at<void *>(lights, 68))
        return;
    graphics::immediate::set_render_state(graphics::bindings::single_draw_texture,
                                          static_cast<Dword *>(lights));
    graphics::backend::set_depth_write(graphics::context, 0);
    for (unsigned i = 0; i < 4; ++i) {
        const Dword group = 72 + i * 168;
        if (at<Byte>(lights, group) && at<Dword>(lights, group + 16) == 1) {
            for (int index = 0; index < at<int>(lights, group + 20); ++index) {
                void *glow = offset(at<void *>(lights, 744),
                                    at<Dword>(lights, group + 52 + Dword(index) * 4) * 248);
                at<Byte>(glow, 228) = at<Byte>(lights, group + 164);
                draw_vehicle_glow(
                    glow, director, rotation,
                    reinterpret_cast<float *>(offset(lights, group + 80 + Dword(index) * 12)),
                    reinterpret_cast<float *>(offset(lights, group + 4)));
            }
        }
    }
    graphics::backend::set_depth_write(graphics::context, 1);
}
} // namespace nfs::game::world::vehicle_render
