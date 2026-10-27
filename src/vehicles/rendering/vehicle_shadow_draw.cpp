#include "../../graphics/actors/model_render_runtime.hpp"
#include "nfs/graphics/primitives/immediate_draw.hpp"
#include "nfs/physics/frame/simulation_runtime.hpp"
#include "nfs/physics/frame/dynamics_runtime.hpp"
#include "../../math/precision/extended_float.hpp"
#include <cmath>
#include <cstring>
#include <initializer_list>
namespace nfs::gui::engine::images {
std::uint8_t suspend_drawing();
}
namespace nfs::game::world::vehicle_render {
namespace {
float scalar(Dword bits) {
    float result;
    std::memcpy(&result, &bits, 4);
    return result;
}
} // namespace

void matrix_from_axes(float *matrix, const float *normal, const float *direction, int normal_axis,
                      int direction_axis) {
    float cross[3], remaining[3];
    runtime::cross_vectors_entry(direction, normal, cross);
    runtime::normalize_vector(cross, cross);
    runtime::cross_vectors_entry(normal, cross, remaining);
    runtime::normalize_vector(remaining, remaining);
    int third_axis = 12 - normal_axis - direction_axis;
    for (unsigned i = 0; i < 3; ++i)
        matrix[third_axis + i] = cross[i];
    for (unsigned i = 0; i < 3; ++i)
        matrix[normal_axis + i] = normal[i];
    for (unsigned i = 0; i < 3; ++i)
        matrix[direction_axis + i] = remaining[i];
    matrix[15] = 1;
    matrix[3] = matrix[7] = matrix[11] = matrix[12] = matrix[13] = matrix[14] = 0;
}

void append_surface_axes(float *matrix, const float *normal, const float *direction,
                         int normal_axis, int direction_axis) {
    float axes[16];
    matrix_from_axes(axes, normal, direction, normal_axis, direction_axis);
    graphics::math::append(matrix, axes);
}

void shadow_plane_matrix(const float *normal, const float *position, const float *direction,
                         float *matrix) {
    auto format = kernel::numeric::current_format();
    auto X = [&](float f) { return kernel::numeric::Extended(f, format); };
    const auto normal_x = X(normal[0]), normal_y = X(normal[1]);
    const float normal_z = normal[2];
    auto light_x = X(direction[0]), light_y = X(direction[1]);
    float light_z = direction[2];
    const auto dot = (X(normal_z) * light_z + normal_y * light_y) + normal_x * light_x;
    if (dot < X(0) || X(0) < dot || std::isnan(float(dot))) {
        const auto inverse = X(-1) / dot;
        light_x = inverse * light_x;
        light_y = inverse * light_y;
        light_z = float(inverse * light_z);
    }
    const float distance =
        float(-((X(normal_z) * position[2] + normal_y * position[1]) + normal_x * position[0]));
    matrix[3] = matrix[7] = matrix[11] = 0;
    matrix[15] = 1;
    matrix[0] = float(normal_x * light_x + 1.f);
    matrix[1] = float(normal_x * light_y);
    matrix[2] = float(normal_x * light_z);
    matrix[4] = float(normal_y * light_x);
    matrix[5] = float(normal_y * light_y + 1.f);
    matrix[6] = float(normal_y * light_z);
    matrix[8] = float(X(normal_z) * light_x);
    matrix[9] = float(X(normal_z) * light_y);
    matrix[10] = float(X(normal_z) * light_z + 1.f);
    matrix[12] = float(X(distance) * light_x);
    matrix[13] = float(X(distance) * light_y);
    matrix[14] = float(X(distance) * light_z);
}

void enable_shadow_stencil(Dword mask) {
    using namespace graphics;
    backend::set_stencil_depth_fail(context, 1);
    backend::set_stencil_pass(context, 3);
    backend::set_stencil_fail(context, 1);
    backend::set_stencil_function(context, 6);
    backend::set_stencil_reference(context, mask);
    backend::set_stencil_mask(context, mask);
    backend::set_stencil_write_mask(context, mask);
    backend::set_stencil_enable(context, 1);
}

void disable_shadow_stencil() {
    graphics::backend::set_stencil_enable(graphics::context, 0);
}

void draw_projected_shadow(void *owner, void *model, const float *transform, const float *position,
                           const float *normal, const float *opacity) {
    using namespace graphics;
    void *lighting = subsystem(56);
    kernel::memory_copy(offset(owner, 80), offset(lighting, 48), 16);
    float sun[3];
    kernel::memory_copy(sun, offset(lighting, 36), 12);
    at<float>(owner, 92) = *opacity;
    sun[1] *= sun[1] < 1000 && !std::isnan(sun[1]) ? 4.0f : 3.0f;
    runtime::normalize_vector(sun, sun);
    float plane[16], view[16], projection[16], pose[16], translation[16], temporary[16];
    shadow_plane_matrix(normal, position, sun, plane);
    kernel::memory_copy(view, backend::view_matrix, 64);
    kernel::memory_copy(projection, backend::projection_matrix, 64);
    kernel::memory_copy(pose, transform, 64);
    math::translation(translation, 0, 0, at<float>(owner, 96));
    auto *result = reinterpret_cast<float *>(offset(owner, 16));
    for (unsigned i = 0; i < 16; ++i)
        result[i] = i % 5 == 0 ? 1.0f : 0.0f;
    const float *operands[] = {translation, projection, view, plane, pose};
    for (const float *left : operands) {
        math::multiply(left, result, temporary);
        kernel::memory_copy(result, temporary, 64);
    }
    enable_shadow_stencil(1);
    pending::draw_model(model, reinterpret_cast<Dword *>(offset(model, 12)));
    disable_shadow_stencil();
}

void draw_shadow(void *owner, Byte texture_pass, void *model, const float *transform,
                 const float *position, const float *surface_normal, const float *opacity,
                 float width, float length) {
    using namespace graphics;
    if (gui::engine::images::suspend_drawing())
        return;
    float normal[3] = {surface_normal[0], surface_normal[1], surface_normal[2]};
    if (!texture_pass) {
        draw_projected_shadow(owner, model, transform, position, normal, opacity);
        return;
    }
    viewports::begin(ui_viewport);
    float forward[3] = {transform[8], transform[9], transform[10]}, matrix[16];
    math::diagonal(matrix, width, length, 1, 1);
    append_surface_axes(matrix, normal, forward, 8, 4);
    matrix[12] = position[0];
    matrix[13] = position[1];
    matrix[14] = position[2];
    auto format = kernel::numeric::current_format();
    auto X = [&](float f) { return kernel::numeric::Extended(f, format); };
    auto alpha = X(1) - (X(transform[13]) - position[1] - .5f) * scalar(1051372203);
    if (alpha < X(0))
        alpha = X(0);
    else if (X(1) < alpha)
        alpha = X(1);
    Dword colour = Dword((alpha * 255.0f).truncate_to_integer()) << 24;
    Dword *draw = bindings::single_draw_texture;
    kernel::memory_copy(reinterpret_cast<Byte *>(draw) + 84, matrix, 64);
    immediate::set_texture(draw, at<Dword>(owner, 108));
    immediate::set_render_state(bindings::single_draw_texture,
                                reinterpret_cast<Dword *>(offset(owner, 112)));
    immediate::set_textured_primitive(bindings::single_draw_texture, 4);

    static const float positions[4][4] = {
        {-1, 1, 0, 1}, {1, 1, 0, 1}, {1, -1, 0, 1}, {-1, -1, 0, 1}};
    static const float coordinates[4][4] = {{0, 0, 1, 1}, {1, 0, 1, 1}, {1, 1, 1, 1}, {0, 1, 1, 1}};
    for (unsigned vertex : {0u, 1u, 2u, 3u, 0u, 2u})
        immediate::textured_vertex(bindings::single_draw_texture, positions[vertex], &colour,
                                   coordinates[vertex]);
    immediate::end_textured_primitive(bindings::single_draw_texture);
    immediate::flush_textured(bindings::single_draw_texture);
    viewports::end(ui_viewport);
}

void pending::draw_vehicle_shadow(void *body, Byte texture_pass) {
    void *lod = subsystem(140);
    if (!at<Byte>(lod, 6) || (!texture_pass && !at<Byte>(lod, 5)) || !at<void *>(body, 32))
        return;
    float distance = at<float>(body, 2120);
    if (distance < 50 || at<Byte>(subsystem(52), 81296))
        return;
    void *car = at<void *>(body, 2124), *physical = at<void *>(car, 1768);
    if (!at<Dword>(physical, 532))
        return;
    auto format = kernel::numeric::current_format();
    auto X = [&](float f) { return kernel::numeric::Extended(f, format); };
    float alpha = float((X(distance) - 50.0f) * scalar(1017370378));
    if (!texture_pass)
        alpha = float((X(at<float>(body, 2076)) - .5f) * alpha);
    if (alpha > 1)
        alpha = 1;
    else if (alpha <= 0 && !std::isnan(alpha))
        return;
    float position[3] = {at<float>(body, 1896), at<float>(body, 1900), -at<float>(body, 1904)};
    float normal[3] = {0, 1, 0};
    if (!at<Byte>(subsystem(52), 81296)) {
        alignas(4) Byte contact[84];
        kernel::memory_copy(contact, offset(physical, 508), sizeof contact);
        runtime::physics::surface_height(contact, position);
        if (!at<Dword>(contact, 24))
            return;
        kernel::memory_copy(normal, contact + 12, 12);
        if (normal[1] < scalar(1063105069) && !std::isnan(normal[1])) {
            float square = float(X(normal[2]) * normal[2] + X(normal[0]) * normal[0]);
            float length = float(std::sqrt(double(X(square))));
            auto scale = X(.5f) / X(length);
            normal[1] = scalar(1063105069);
            normal[0] = float(X(normal[0]) * scale);
            normal[2] = float(X(normal[2]) * scale);
            runtime::physics::pending::normalize_collision_entry(normal);
        }
    }
    normal[2] = -normal[2];
    position[2] = at<float>(body, 1904);
    float smooth[3];
    float delta_x = float((X(normal[0]) - at<float>(body, 2132)) * scalar(1036831949));
    float delta_z = float(X(normal[2]) - at<float>(body, 2140));
    smooth[0] = float(X(delta_x) + at<float>(body, 2132));
    smooth[1] = float((X(normal[1]) - at<float>(body, 2136)) * scalar(1036831949) +
                      at<float>(body, 2136));
    smooth[2] = float(X(delta_z) * scalar(1036831949) + at<float>(body, 2140));
    runtime::physics::pending::normalize_collision_entry(smooth);
    kernel::memory_copy(offset(body, 2132), smooth, 12);
    draw_shadow(subsystem(124), texture_pass, at<void *>(body, 32),
                reinterpret_cast<float *>(offset(body, 1988)), position, smooth, &alpha,
                at<float>(body, 2056), at<float>(body, 2060));
}
} // namespace nfs::game::world::vehicle_render
