#include "vehicle_renderable_frame.hpp"
#include "../../graphics/cameras/camera_director_runtime.hpp"
#include "../../math/vectors/retained_vector.hpp"
#include <cstring>
#include <cmath>
namespace nfs::game::world::vehicle_render {
using kernel::numeric::Extended;

void bind_renderable_car(void *renderable, void *car) {
    at<void *>(renderable, 3976) = car;
    at<void *>(renderable, 3464) = car;
    at<void *>(renderable, 3128) = car;
    bind_driver_car(offset(renderable, 856), car);
}

void renderable_half_extents(void *renderable, float *output) {
    const auto format = kernel::numeric::current_format();
    for (unsigned axis = 0; axis < 3; ++axis)
        output[axis] = float((Extended(at<float>(renderable, 3992 + axis * 4), format) -
                              at<float>(renderable, 3980 + axis * 4)) *
                             .5f);
    std::memcpy(offset(renderable, 4028), output, 12);
}

Byte renderable_visible(void *renderable, void *director) {
    void *state = world::invoke<void *>(director, 2), *car = at<void *>(renderable, 3976);
    if ((at<Dword>(state, 72) == 3 || !at<Byte>(subsystem(52), 81297)) &&
        at<void *>(director, 300) &&
        at<Dword>(car, 1772) == at<Dword>(at<void *>(director, 300), 1772))
        return 0;
    float position[3];
    camera::active_camera_position(director, position);
    position[2] = -position[2];
    const auto format = kernel::numeric::current_format();
    const auto distance = runtime::retained_point_distance(
        position, reinterpret_cast<const float *>(offset(renderable, 3892)));
    if (!(Extended(100, format) < distance) ||
        at<Dword>(at<void *>(at<void *>(renderable, 3976), 1768), 532)) {
        if (!(Extended(300, format) < distance)) {
            float extents[3];
            renderable_half_extents(renderable, extents);
            float radius = extents[1];
            if (radius < extents[0])
                radius = extents[0];
            if (!(extents[2] < radius))
                radius = extents[2];
            return graphics::model_render::sphere_visible(
                       graphics::main_viewport,
                       reinterpret_cast<float *>(offset(renderable, 3892)), radius) != 0;
        }
    }
    return 0;
}

void prepare_renderable_frame(void *renderable, void *director, const float *position,
                              const float *rotation) {
    std::memcpy(offset(renderable, 3892), position, 12);
    std::memcpy(offset(renderable, 3904), rotation, 36);
    graphics::math::from_compact_transform(reinterpret_cast<float *>(offset(renderable, 928)),
                                           reinterpret_cast<float *>(offset(renderable, 3904)),
                                           reinterpret_cast<float *>(offset(renderable, 3892)));
    const Byte visible = renderable_visible(renderable, director);
    at<Byte>(renderable, 4052) = visible;
    if (visible) {
        void *state = world::invoke<void *>(director, 2);
        std::memcpy(offset(renderable, 2912), offset(state, 4), 12);
        prepare_renderable_models(renderable, director);
        advance_damage_visibility(renderable);
    }
}

void prepare_renderable_models(void *renderable, void *director) {
    void *state = world::invoke<void *>(director, 2);
    if (at<Dword>(state, 72) == 4 && !at<Dword>(subsystem(52), 81284))
        at<Dword>(renderable, 3124) = at<Dword>(subsystem(52), 81312);
    else
        update_renderable_lod(offset(renderable, 1004), director,
                              reinterpret_cast<float *>(offset(renderable, 3892)));
    update_driver_animation(offset(renderable, 856), at<Dword>(renderable, 56));
    if (at<Dword>(subsystem(52), 81284)) {
        bool draw = true;
        if (at<float>(renderable, 3124) < 75.f || std::isnan(at<float>(renderable, 3124))) {
            state = world::invoke<void *>(director, 2);
            if (at<Dword>(state, 72) != 4 &&
                at<Dword>(world::invoke<void *>(director, 2), 72) != 2)
                draw = false;
            const Dword car = at<Dword>(renderable, 3952);
            if (car != 1 && car != 15 && car != 23)
                draw = false;
        }
        if (draw)
            draw_vehicle_driver(offset(renderable, 856), director,
                                reinterpret_cast<float *>(offset(renderable, 3904)),
                                reinterpret_cast<float *>(offset(renderable, 3892)));
    }
    pending::initialize_attachment_root(offset(renderable, 3480));
    evaluate_damage_pose(renderable);
    draw_damage_bounds(renderable, reinterpret_cast<float *>(offset(renderable, 3904)),
                       reinterpret_cast<float *>(offset(renderable, 3892)));
    prepare_renderable_panels(renderable);
    std::memcpy(offset(at<void *>(renderable, 3888), 48),
                runtime::skids::lifecycle::identity_matrix, 64);
    std::memcpy(offset(renderable, 2992), offset(renderable, 928), 64);
    pending::evaluate_attachments(offset(renderable, 3480));
    smooth_renderable_damage(offset(renderable, 1004), 1);
    world::invoke(offset(renderable, 1004), 1, offset(renderable, 928),
                  offset(renderable, 3892));
    update_renderable_light_channels(renderable);
    update_vehicle_light_positions(offset(renderable, 80),
                                   reinterpret_cast<float *>(offset(renderable, 928)));
}

void draw_renderable_lights(void *renderable, void *director) {
    if (at<Byte>(renderable, 4052))
        draw_vehicle_lights(offset(renderable, 80), director,
                            reinterpret_cast<float *>(offset(renderable, 3904)));
}

void update_renderable_light_channels(void *renderable) {
    if (!at<Byte>(subsystem(52), 81296)) {
        set_light_channel(offset(renderable, 80), 2, 0);
        set_light_channel(offset(renderable, 80), 0,
                          at<float>(at<void *>(renderable, 3976), 1608) > 0);
        set_light_channel(offset(renderable, 80), 1,
                          at<Byte>(at<void *>(renderable, 3976), 1579) != 0);
        if (!at<Byte>(at<void *>(renderable, 3976), 1584)) {
            set_light_channel(offset(renderable, 80), 3, 1);
            return;
        }
    } else
        for (Dword channel = 0; channel < 3; ++channel)
            set_light_channel(offset(renderable, 80), channel, 0);
    set_light_channel(offset(renderable, 80), 3, 0);
}

void release_preview_renderable(void *owner, void *renderable) {
    world::pending::unload_vehicle_renderable(renderable);
    --at<Dword>(owner, 81304);
}

void set_frontend_light(void *lighting, int index, const float *colour, Dword intensity,
                        const float *direction) {
    void *light = offset(lighting, 76 + Dword(index) * 28);
    std::memcpy(light, colour, 12);
    at<Dword>(light, 24) = intensity;
    std::memcpy(offset(light, 12), direction, 12);
}
} // namespace nfs::game::world::vehicle_render
