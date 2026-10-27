#include "race_legacy_effects.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
#include <cstring>
namespace nfs::game::world::frame {
static Dword effect_texture(void *particle) {
    void *resource = at<void *>(at<void *>(particle, 0), 108);
    return resource ? at<Dword>(resource, 8) : 0;
}

void *draw_rotated_effect_batch(void *particle, const float *orientation) {
    if (!particle || gui::engine::images::suspend_drawing())
        return nullptr;
    float right[3], up[3];
    std::memcpy(right, orientation, 12);
    std::memcpy(up, orientation + 3, 12);
    Dword *state = at<Dword *>(at<void *>(particle, 0), 96);
    const Dword texture = effect_texture(particle);
    graphics::immediate::set_render_state(graphics::bindings::single_draw_texture, state);
    graphics::immediate::set_texture(graphics::bindings::single_draw_texture, texture);
    graphics::immediate::set_textured_primitive(graphics::bindings::single_draw_texture, 4);
    auto E = [](float value) { return static_cast<long double>(value); };
    do {
        if (effect_texture(particle) != texture || at<Dword *>(at<void *>(particle, 0), 96) != state)
            break;
        const float size = at<float>(particle, 36);
        const float angle = at<float>(particle, 48);
        const float sine = float(runtime::turn_sine(angle));
        const float cosine = float(runtime::turn_cosine(angle));
        const float zero = float(E(size) * 0.f);
        float scaled_up[4], scaled_right[4], centre[4];
        std::memcpy(centre, offset(particle, 12), 16);
        centre[3] = 1;
        assign_effect_vector(scaled_up, float(E(up[0]) * size), float(E(up[1]) * size),
                               float(E(up[2]) * size), zero);
        assign_effect_vector(scaled_right, float(E(right[0]) * size), float(E(right[1]) * size),
                               float(E(right[2]) * size), zero);
        float up_cosine[4], right_sine[4], up_sine[4], right_cosine[4];
        for (unsigned lane = 0; lane < 4; ++lane) {
            up_cosine[lane] = float(E(scaled_up[lane]) * cosine);
            right_sine[lane] = float(E(scaled_right[lane]) * sine);
        }
        float corners[4][4];
        for (unsigned lane = 0; lane < 4; ++lane) {
            const float displaced = float(E(centre[lane]) - right_sine[lane]);
            corners[0][lane] = float(E(up_cosine[lane]) + displaced);
        }
        for (unsigned lane = 0; lane < 4; ++lane) {
            up_sine[lane] = float(E(scaled_up[lane]) * sine);
            right_cosine[lane] = float(E(scaled_right[lane]) * cosine);
        }
        for (unsigned lane = 0; lane < 4; ++lane) {
            const float displaced = float(E(right_cosine[lane]) + centre[lane]);
            corners[1][lane] = float(E(up_sine[lane]) + displaced);
        }
        // Native recomputes these products for the third and fourth corners.
        for (unsigned lane = 0; lane < 4; ++lane) {
            up_cosine[lane] = float(E(scaled_up[lane]) * cosine);
            right_sine[lane] = float(E(scaled_right[lane]) * sine);
        }
        for (unsigned lane = 0; lane < 4; ++lane) {
            const float displaced = float(E(right_sine[lane]) + centre[lane]);
            corners[2][lane] = float(E(displaced) - up_cosine[lane]);
        }
        for (unsigned lane = 0; lane < 4; ++lane) {
            up_sine[lane] = float(E(scaled_up[lane]) * sine);
            right_cosine[lane] = float(E(scaled_right[lane]) * cosine);
        }
        float displaced[4];
        subtract_effect_vector(centre, displaced, right_cosine);
        subtract_effect_vector(displaced, corners[3], up_sine);
        Dword colour;
        pack_effect_colour(&colour, at<Byte>(particle, 75), at<Byte>(particle, 74),
                             at<Byte>(particle, 73), at<Byte>(particle, 72));
        constexpr unsigned order[] = {0, 1, 2, 2, 3, 0};

        constexpr unsigned uv_order[] = {0, 1, 2, 2, 4, 0};
        for (unsigned vertex = 0; vertex < 6; ++vertex)
            graphics::immediate::textured_vertex(graphics::bindings::single_draw_texture,
                                                 corners[order[vertex]], &colour,
                                                 effect_uv[uv_order[vertex]]);
        particle = at<void *>(particle, 120);
    } while (particle);
    graphics::immediate::end_textured_primitive(graphics::bindings::single_draw_texture);
    graphics::immediate::flush_textured(graphics::bindings::single_draw_texture);
    return particle;
}

void *draw_trail_effect_batch(void *particle, const float *orientation, const float *eye, Dword player) {
    if (!particle || gui::engine::images::suspend_drawing())
        return nullptr;
    float inverse[9];
    runtime::matrices::transpose(orientation, inverse);
    Dword *state = at<Dword *>(at<void *>(particle, 0), 96);
    const Dword texture = effect_texture(particle);
    graphics::immediate::set_render_state(graphics::bindings::single_draw_texture, state);
    graphics::immediate::set_texture(graphics::bindings::single_draw_texture, texture);
    graphics::immediate::set_textured_primitive(graphics::bindings::single_draw_texture, 4);
    auto E = [](float value) { return static_cast<long double>(value); };
    do {
        if (effect_texture(particle) != texture || at<Dword *>(at<void *>(particle, 0), 96) != state)
            break;
        float *previous = reinterpret_cast<float *>(offset(particle, 88 + player * 12));
        if (at<Byte>(particle, 112 + player)) {
            const auto direction = runtime::hardware::dot_product(previous, orientation);
            const float sign = direction < E(0) ? -1.f : 1.f;
            float transformed[3];
            runtime::matrices::transform_vectors(1, previous, orientation, transformed);
            float corners[4][4];
            corners[0][3] = corners[1][3] = corners[2][3] = corners[3][3] = 1;
            float vertical[3], horizontal[3];
            const float size = at<float>(particle, 36);
            for (unsigned axis = 0; axis < 3; ++axis) {
                corners[0][axis] = float(E(transformed[axis]) + eye[axis]);
                corners[1][axis] = corners[0][axis];
                vertical[axis] = float(E(orientation[3 + axis]) * size * sign);
                horizontal[axis] = float(E(orientation[axis]) * size);
            }
            for (unsigned axis = 0; axis < 3; ++axis) {
                corners[2][axis] = float(E(horizontal[axis]) + vertical[axis] + at<float>(particle, 12 + axis * 4));
                corners[3][axis] = float(-E(horizontal[axis]) - vertical[axis] + at<float>(particle, 12 + axis * 4));
            }
            const Dword colour = at<Dword>(particle, 72);
            constexpr unsigned order[] = {0, 1, 2, 2, 3, 0};
            for (unsigned vertex = 0; vertex < 6; ++vertex)
                graphics::immediate::textured_vertex(graphics::bindings::single_draw_texture,
                                                     corners[order[vertex]], &colour,
                                                     effect_uv[vertex]);
        }
        for (unsigned axis = 0; axis < 3; ++axis)
            previous[axis] = float(E(at<float>(particle, 12 + axis * 4)) - eye[axis]);
        float transformed[3];
        runtime::matrices::transform_vectors(1, previous, inverse, transformed);
        std::memcpy(previous, transformed, 12);
        at<Byte>(particle, 112 + player) = 1;
        particle = at<void *>(particle, 120);
    } while (particle);
    graphics::immediate::end_textured_primitive(graphics::bindings::single_draw_texture);
    graphics::immediate::flush_textured(graphics::bindings::single_draw_texture);
    return particle;
}
} // namespace nfs::game::world::frame
