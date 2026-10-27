#include "nfs/graphics/particles/particle_lifecycle.hpp"
#include "nfs/graphics/primitives/immediate_draw.hpp"
#include "../../gui/lifecycle/gui_images.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
#include "../../race/session/race_particle_render.hpp"
#include "nfs/core/lifecycle/system_runtime.hpp"
#include "nfs/physics/frame/simulation_runtime.hpp"

namespace nfs::game::runtime::particles::lifecycle {
extern float near_fade_inverse;
// Read the saved x87 body, not its empty void pseudocode.
static long double particle_distance_squared(const float *value) {
    const auto z = static_cast<long double>(value[2]);
    const auto y = static_cast<long double>(value[1]);
    const auto x = static_cast<long double>(value[0]);
    const auto xx = x * x;
    const auto yy = y * y;
    const auto xy = xx + yy;
    const auto zz = z * z;
    return xy + zz;
}

void collect_visible_particles(void *buffer, void *camera) {
    if (!at<int>(buffer, 72000))
        return;
    Dword *pose = invoke<Dword *>(camera, 2);
    const float *position = reinterpret_cast<float *>(bytes(pose) + 4);
    const float eye[3] = {position[0], position[1], -position[2]};
    float orientation[9];
    race_audio::camera_orientation(pose, orientation);
    const float forward[3] = {orientation[6], orientation[7], -orientation[8]};
    auto E = [](float value) { return static_cast<long double>(value); };
    auto settings = [] { return reinterpret_cast<void *>(nfs::lifecycle::runtime::particle_emitters); };
    auto byte_value = [&](float value) {
        return Dword(static_cast<std::int64_t>(E(value))) & 255;
    };
    for (int index = 0; index < at<int>(buffer, 72000); ++index) {
        auto *particle = at<Particle *>(buffer, 68000 + Dword(index) * 4);
        void *definition = particle->definition;
        const float delta[3] = {float(E(particle->position[0]) - eye[0]),
                                float(E(particle->position[1]) - eye[1]),
                                float(E(particle->position[2]) - eye[2])};
        const float depth = float(hardware::dot_product(forward, delta));
        particle->depth = depth;
        if (!(depth < at<float>(settings(), 12)) || !(depth > 1.f))
            continue;
        const float squared = float(particle_distance_squared(delta));
        const float far_squared = at<float>(settings(), 20);
        if (far_squared <= squared)
            continue;
        auto opacity = E(1);
        if (E(at<float>(settings(), 24)) < E(squared)) {
            const float distance = float(__builtin_sqrtl(E(squared)));
            opacity = (E(at<float>(settings(), 12)) - distance) * at<float>(settings(), 28);
        } else if (depth < 1.5f)
            opacity = (E(depth) - 1.f) * near_fade_inverse;
        const auto retained_age = E(particle->inverse_lifetime) * particle->age;
        const float age = float(retained_age);
        const bool second_half = E(0.5f) < retained_age || __builtin_isnan(retained_age);
        const auto alpha =
            (E(age) * at<float>(definition, second_half ? 292 : 276) +
             at<float>(definition, second_half ? 308 : 228)) * opacity;
        const float rounded_alpha = float(alpha);
        if (alpha < E(0))
            continue;
        float colour[3];
        scale_vectors(1, reinterpret_cast<float *>(bytes(definition) + (second_half ? 280 : 264)),
                       age, colour);
        const unsigned base = second_half ? 296 : 216;
        for (unsigned channel = 0; channel < 3; ++channel)
            colour[channel] = float(E(colour[channel]) + at<float>(definition, base + channel * 4));
        const Dword alpha_byte = byte_value(rounded_alpha);
        const Dword red = byte_value(colour[0]);
        const Dword green = byte_value(colour[1]);
        const Dword blue = byte_value(colour[2]);
        particle->colour = (alpha_byte << 24) | (red << 16) | (green << 8) | blue;
        auto *owner = at<Renderer *>(nfs::lifecycle::runtime::particle_owner, 8);
        append_render_particle(owner, particle);
    }
}

void draw_particle_billboard(void *particle, const float *up, const float *right) {
    auto E = [](float value) { return static_cast<long double>(value); };
    const auto age = E(at<float>(particle, 28)) * at<float>(particle, 12);
    const float width = float(age * at<float>(particle, 40) + at<float>(particle, 32));
    const float height = float(age * at<float>(particle, 44) + at<float>(particle, 36));
    const float sine = float(turn_sine(at<float>(particle, 48)));
    const auto cosine = turn_cosine(at<float>(particle, 48));
    const auto sine_width = E(sine) * width, cosine_width = cosine * width;
    const auto sine_height = E(sine) * height, cosine_height = cosine * height;
    float horizontal[3];
    horizontal[0] = float(sine_height * up[0] - cosine_height * right[0]);
    horizontal[1] = float(sine_height * up[1] - cosine_height * right[1]);
    horizontal[2] = float(sine_height * up[2] - cosine_height * right[2]);
    const float vertical_x = float(sine_width * right[0] + cosine_width * up[0]);
    const float vertical_y = float(sine_width * right[1] + cosine_width * up[1]);
    const auto vertical_z = sine_width * right[2] + cosine_width * up[2];
    const auto minus_x = E(at<float>(particle, 0)) - horizontal[0];
    const auto minus_y = E(at<float>(particle, 4)) - horizontal[1];
    const float minus_z = float(E(at<float>(particle, 8)) - horizontal[2]);
    const auto plus_x = E(horizontal[0]) + at<float>(particle, 0);
    const auto plus_y = E(horizontal[1]) + at<float>(particle, 4);
    const float plus_z = float(E(horizontal[2]) + at<float>(particle, 8));
    float corners[4][4];
    corners[0][0] = float(minus_x - vertical_x);
    corners[0][1] = float(minus_y - vertical_y);
    corners[0][2] = float(E(minus_z) - vertical_z);
    corners[1][0] = float(plus_x - vertical_x);
    corners[1][1] = float(plus_y - vertical_y);
    corners[1][2] = float(E(plus_z) - vertical_z);
    corners[2][0] = float(plus_x + vertical_x);
    corners[2][1] = float(plus_y + vertical_y);
    corners[2][2] = float(E(plus_z) + vertical_z);
    corners[3][0] = float(minus_x + vertical_x);
    corners[3][1] = float(minus_y + vertical_y);
    corners[3][2] = float(E(minus_z) + vertical_z);
    for (auto &corner : corners)
        corner[3] = 1;
    if (gui::engine::images::suspend_drawing())
        return;
    const Dword texture = at<Dword>(at<void *>(particle, 60), 196);
    const Dword *colour = &at<Dword>(particle, 64);
    using namespace graphics;
    if (texture) {
        immediate::set_texture(bindings::single_draw_texture, texture);
        immediate::set_textured_primitive(bindings::single_draw_texture, 6);
        static const float uv[4][4] = {
            {0, 0, 1, 1}, {0, 1, 1, 1}, {1, 1, 1, 1}, {1, 0, 1, 1}};
        for (unsigned vertex = 0; vertex < 4; ++vertex)
            immediate::textured_vertex(bindings::single_draw_texture, corners[vertex], colour,
                                       uv[vertex]);
        immediate::end_textured_primitive(bindings::single_draw_texture);
        immediate::flush_textured(bindings::single_draw_texture);
    } else {
        immediate::set_primitive(bindings::single_draw, 6);
        for (auto &corner : corners)
            immediate::vertex(bindings::single_draw, corner, colour);
        immediate::end_primitive(bindings::single_draw);
        immediate::flush(bindings::single_draw);
    }
}
} // namespace nfs::game::runtime::particles::lifecycle
