#include "vehicle_renderable_resources.hpp"
#include "vehicle_shader_parameters.hpp"
#include "nfs/graphics/resources/eagl_assets.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
#include <cmath>
#include <cstring>
namespace nfs::game::world::vehicle_render {
float hot_specular_amount{};

float *vehicle_light(void *lighting, Dword index) {
    return reinterpret_cast<float *>(offset(lighting, 76 + index * 28));
}

void smooth_hot_specular(void *body) {
    if (at<Byte>(subsystem(52), 81296))
        return;
    if (!at<Byte>(body, 2448)) {
        std::memcpy(offset(body, 2432), offset(body, 2416), 16);
        at<Byte>(body, 2448) = 1;
    }
    auto X = [](float value) { return static_cast<long double>(value); };
    float current = at<float>(body, 2432), target = at<float>(body, 2416);
    if (!std::isnan(current) && !std::isnan(target) && current < target) {
        auto next = X(current) + .1f;
        at<float>(body, 2432) = float(next);

        if (!(next < X(target)))
            at<float>(body, 2432) = target;
    } else if (current > target) {
        auto next = X(current) - .1f;
        at<float>(body, 2432) = float(next);
        if (!(X(target) < next))
            at<float>(body, 2432) = target;
    }
}

void prepare_hot_specular(void *body) {
    float enabled = 0;
    void *car = at<void *>(body, 2124);
    if (car && !at<Byte>(subsystem(52), 81296) &&
        at<std::uint16_t>(at<void *>(car, 1768), 584) != 4)
        enabled = 1;
    for (unsigned channel = 0; channel < 4; ++channel)
        at<float>(body, 2416 + channel * 4) = enabled;
    smooth_hot_specular(body);
    auto X = [](float value) { return static_cast<long double>(value); };
    float heat = at<float>(body, 2076);
    if (heat > .2f || std::isnan(heat)) {
        if (at<Byte>(subsystem(52), 81296) != 1) {
            auto amount = X(heat) + .2f;
            hot_specular_amount = X(1) < amount || std::isnan(heat) ? 1 : float(amount);
        } else
            hot_specular_amount = 0;
    } else
        hot_specular_amount = 0;
    shader_parameters.hot_specular_colour[3] = 0;
    for (unsigned channel = 0; channel < 3; ++channel) {
        float paint = at<float>(body, 2368 + channel * 4);
        shader_parameters.hot_specular_colour[channel] =
            float((X(at<float>(body, 2432)) - paint) * hot_specular_amount + paint);
    }
}

void prepare_shader_lighting(void *body) {
    auto X = [](float value) { return static_cast<long double>(value); };
    auto &shader = shader_parameters;
    if (at<void *>(body, 2112)) {
        if (shader.damage_texture && at<Dword *>(body, 2340))
            graphics::tar::replace_shared(shader.damage_texture + 10, at<Dword *>(body, 2340));
        // The texture callback and both damage callbacks may replace body+840.
        if (!at<Byte>(at<void *>(body, 2112), 20)) {
            for (float &zone : shader.damage_zone_amounts)
                zone = 1;
        } else {
            for (int zone = 0; zone < 10; ++zone) {

                volatile float amount = damage_amount(at<void *>(body, 2112), zone);
                volatile float limit = damage_limit(at<void *>(body, 2112), zone);
                auto remaining = X(1);
                if (amount > 0)
                    remaining = X(1) - X(amount) / X(limit);
                for (unsigned channel = 0; channel < 4; ++channel)
                    shader.damage_zone_amounts[zone * 4 + channel] = float(remaining);
            }
            for (unsigned channel = 60; channel < 64; ++channel)
                shader.damage_zone_amounts[channel] = 1;
        }
    }
    float heat = at<float>(body, 2076);
    if (.4f > heat)
        heat = .4f;
    shader.paint_light_constants[0] = heat;
    shader.paint_light_constants[1] = at<float>(body, 4);
    shader.paint_light_constants[2] = at<float>(body, 2068);
    shader.paint_light_constants[3] = at<float>(body, 2064);
    for (unsigned i = 0; i < 4; ++i) {
        shader.sphere_map_constants[i] = .5f;
        shader.sphere_map_scale[i] = 0.85000002384185791015625f;
    }
    shader.glass_light_constants[0] = heat;
    shader.glass_light_constants[1] = at<float>(body, 8);
    shader.glass_light_constants[2] = at<float>(body, 2068);
    shader.glass_light_constants[3] = at<float>(body, 2064);
    shader.gloss_colour_constants[0] = shader.gloss_colour_constants[1] =
        shader.gloss_colour_constants[2] = 1;
    shader.gloss_colour_constants[3] = 0;
    shader.rubber_light_constants[0] = heat;
    shader.rubber_light_constants[1] = at<float>(body, 12);
    shader.rubber_light_constants[2] = 255;
    shader.rubber_light_constants[3] = at<float>(body, 2064);
    shader.standard_constants[0] = 0;
    shader.standard_constants[1] = .5f;
    shader.standard_constants[2] = 1;
    shader.standard_constants[3] = .4f;
    for (float &constant : shader.pixel_standard_constants)
        constant = 1;
    float *lights[] = {vehicle_light(subsystem(56), 0), vehicle_light(subsystem(56), 1),
                       vehicle_light(subsystem(56), 2)};
    for (unsigned channel = 0; channel < 3; ++channel) {
        auto value =
            X(at<float>(body, 2108)) * lights[0][channel] + at<float>(body, 2092 + channel * 4);
        // TEST AH,5/JNP caps greater and unordered values at one.
        at<float>(body, 2368 + channel * 4) =
            X(1) < value || __builtin_isnan(value) ? 1 : float(value);
    }
    for (unsigned channel = 0; channel < 4; ++channel)
        at<float>(body, 2400 + channel * 4) = at<float>(body, 2384 + channel * 4) = 1;
    shader.gloss_colour_constants[3] = 1;
    at<Dword>(body, 2380) = 0;
    void *lighting = subsystem(56);
    std::memcpy(shader.ambient_sky, offset(lighting, 12), 12);
    std::memcpy(shader.ambient_ground, offset(lighting, 24), 12);
    shader.gloss_colour_constants[0] = shader.gloss_colour_constants[1] =
        shader.gloss_colour_constants[2] = 0;
    shader.ambient_sky[3] = shader.ambient_ground[3] = 0;
    if (!at<Byte>(subsystem(52), 81296)) {
        std::memcpy(lights[0] + 3, offset(lighting, 64), 12);
        lights[0][6] = heat;
    }
    for (unsigned light = 0; light < 3; ++light) {
        std::memcpy(shader.light_block + light * 4, lights[light] + 3, 12);
        shader.light_block[light * 4 + 3] = 0;
        for (unsigned channel = 0; channel < 3; ++channel)
            shader.light_block[12 + light * 4 + channel] =
                float(X(lights[light][channel]) * lights[light][6]);
        shader.light_block[12 + light * 4 + 3] = lights[light][6];
    }
}

void *body_layer(void *geometry, int index) {
    return offset(geometry, Dword(index) * 24);
}

void select_body_layers(void *body) {
    for (int index = 0; index < at<int>(at<void *>(body, 28), 156); ++index) {
        void *layer = body_layer(offset(body, 36), index);
        bool visible = false;
        if (!at<Byte>(layer, 20) || at<Byte>(layer, 21)) {
            float distance = at<float>(body, 2120), minimum = at<float>(layer, 8),
                  maximum = at<float>(layer, 4);
            visible = !(distance <= minimum) &&
                      !(distance > maximum || std::isnan(distance) || std::isnan(maximum)) &&
                      (at<Dword>(layer, 12) & at<Dword>(body, 2116));
            if (at<Byte>(layer, 23) && (at<Dword>(layer, 12) & at<Dword>(body, 2116)))
                visible = at<Byte>(subsystem(52), 81296) != 0;
        }
        at<std::uint16_t>(at<void *>(layer, 0), 2) = visible ? 1 : 0;
    }
}

void draw_body_layer(void *body, Dword mask) {
    at<Dword>(body, 2116) = mask;
    select_body_layers(body);
    pending::draw_model(at<void *>(body, 28),
                        reinterpret_cast<const Dword *>(offset(body, 1920)));
}
// RET4; matrix argument is ignored.
void render_body_passes(void *body, const Dword *) {
    graphics::backend::set_depth_write(graphics::context, 0);
    pending::draw_vehicle_shadow(body, 0);
    pending::draw_vehicle_shadow(body, 1);
    graphics::backend::set_depth_write(graphics::context, 1);
    if (!at<void *>(body, 28))
        return;
    auto *state = reinterpret_cast<Dword *>(offset(body, 2204));
    bind_model_states(at<void *>(body, 28), state, 0);
    draw_body_layer(body, 1);
    bind_model_states(at<void *>(body, 28), state, 1);
    draw_body_layer(body, 8);
    graphics::backend::set_depth_write(graphics::context, 0);
    bind_model_states(at<void *>(body, 28), state, 1);
    draw_body_layer(body, 2);
    bind_model_states(at<void *>(body, 28), state, 2);
    draw_body_layer(body, 4);
    graphics::backend::set_depth_write(graphics::context, 1);
}
} // namespace nfs::game::world::vehicle_render
namespace nfs::game::world::vehicle_render::pending {
// RET8.
void __fastcall prepare_body_shader(void *body, void *, const Dword *matrix, const Dword *position) {
    auto &shader = shader_parameters;
    shader.camera_position[2] = -at<float>(body, 1916);
    std::memcpy(offset(body, 1896), position, 12);
    shader.camera_position[0] = at<float>(body, 1908);
    std::memcpy(shader.vehicle_position, offset(body, 1896), 8);
    std::memcpy(offset(body, 1920), matrix, 64);
    shader.camera_position[1] = at<float>(body, 1912);
    shader.vehicle_position[2] = at<float>(body, 1904);
    std::memcpy(shader.sun_position, offset(subsystem(56), 36), 12);
    shader.camera_position[3] = 0;
    shader.vehicle_position[3] = 1;
    shader.sun_position[3] = 0;
    if (at<Byte>(subsystem(52), 81296)) {
        shader.camera_position[0] = shader.vehicle_position[0];
        shader.camera_position[2] =
            float(static_cast<long double>(shader.vehicle_position[2]) +
                  at<volatile float>(body, 1916) + 20);
        shader.camera_position[1] = shader.vehicle_position[1];
    }
    std::memcpy(shader.paint_specular_colour, offset(body, 2368), 16);
    std::memcpy(shader.glass_specular_colour, offset(body, 2400), 16);
    std::memcpy(shader.rubber_specular_colour, offset(body, 2384), 16);
    prepare_hot_specular(body);
    prepare_shader_lighting(body);
    render_body_passes(body, matrix);
}
} // namespace nfs::game::world::vehicle_render::pending
