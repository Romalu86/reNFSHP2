#include "ai_entity_runtime.hpp"
#include "nfs/graphics/particles/particle_lifecycle.hpp"
#include "nfs/world/lifecycle/world_lifecycle.hpp"
#include "nfs/core/frame/game_step.hpp"
#include "nfs/physics/frame/dynamics_runtime.hpp"
#include "../../math/vectors/retained_vector.hpp"
#include <cmath>
#include <cstring>
namespace nfs::game::world::camera {
void active_camera_position(void *, float *);
}
namespace nfs::game::runtime::ai::entities::pending {
namespace {
using kernel::numeric::Extended;
bool visible_to_camera(const float *position) {
    float closest = -1.f;
    const int count = physics::split_screen() ? 2 : 1;
    for (int listener = 0; listener < count; ++listener) {
        float camera[3];
        game::world::camera::active_camera_position(game::tick_services[listener], camera);
        const auto distance = retained_point_distance(camera, position);
        if (distance < Extended(closest, kernel::numeric::current_format()) || closest < 0)
            closest = float(distance);
    }
    return !(150.f < closest);
}
void emit_definition(Dword effect, const float *position, float scale, const float *direction,
                     const float *velocity) {
    Byte *definition = particles::lifecycle::particle_prefix;
    for (;;) {
        const Dword identifier = at<Dword>(definition, 4);
        if (!identifier)
            return;
        if (identifier == effect)
            break;
        definition += 152;
    }
    // Both switch tables select identifiers 1, 5, 13 and 15.
    const Dword flags = effect == 1 || effect == 5 || effect == 13 || effect == 15;
    for (int index = 0; index < at<int>(definition, 8); ++index)
        particles::lifecycle::emit_car_particle(definition, position, scale, direction,
                                                saved::zero_vector, velocity, 1, flags);
}
} // namespace

void emit_world_effect(const float *position, const float *direction, Dword effect, float scale,
                       const float *velocity, float minimum_radius, float maximum_radius) {
    if (!visible_to_camera(position))
        return;
    if (scale < 0)
        scale = 0;
    else if (scale > 1.f)
        scale = 1.f;
    float inherited[] = {velocity[0], velocity[1], -velocity[2]};
    float location[] = {position[0], position[1], -position[2]}, heading[3];
    std::memcpy(heading, direction, 12);
    physics::pending::normalize_collision_entry(heading);
    if (minimum_radius != 0 && maximum_radius != 0) {
        const auto format = kernel::numeric::current_format();
        const Dword radius_word = emitter_random_word() & 65535;
        const auto fraction = Extended::integer(radius_word, format) * 0.0000152587890625f;
        minimum_radius = float(fraction * (Extended(maximum_radius, format) - minimum_radius) +
                               minimum_radius);
        const Dword angle_word = emitter_random_word() & 65535;
        const float angle = float(Extended::integer(angle_word, format) * 0.0000152587890625f);
        float sine, cosine;
        turn_sine_cosine(&angle, &sine, &cosine);
        location[0] = float(Extended(sine, format) * minimum_radius + location[0]);
        location[2] = float(Extended(cosine, format) * minimum_radius + location[2]);
    }
    emit_definition(effect, location, scale, heading, inherited);
}

void emit_body_effect(void *body, Dword effect, float scale, const float *local_offset) {
    const float *position = reinterpret_cast<float *>(bytes(body) + 680);
    if (!visible_to_camera(position))
        return;
    if (scale < 0)
        scale = 0;
    else if (scale > 1.f)
        scale = 1.f;
    float velocity[] = {at<float>(body, 8), at<float>(body, 12), -at<float>(body, 16)};
    float rotation[] = {at<float>(body, 692),  at<float>(body, 696),  -at<float>(body, 700),
                        at<float>(body, 704),  at<float>(body, 708),  -at<float>(body, 712),
                        -at<float>(body, 716), -at<float>(body, 720), at<float>(body, 724)};
    const float displacement[] = {local_offset[0], local_offset[1], -local_offset[2]};
    float offset[3] = {0, 0, 0};
    matrices::transform_vectors(1, displacement, rotation, offset);
    const auto format = kernel::numeric::current_format();
    const float location[] = {float(Extended(offset[0], format) + position[0]),
                              float(Extended(offset[1], format) + position[1]),
                              float(Extended(offset[2], format) - position[2])};
    float heading[3];
    std::memcpy(heading, velocity, 12);
    physics::pending::normalize_collision_entry(heading);
    emit_definition(effect, location, scale, heading, velocity);
}

void emit_helicopter_dust(void *entity) {
    void *body = invoke<void *>(interface_at(entity, 1, 320), 1);
    if (!body || at<std::int16_t>(body, 584) == 4)
        return;
    float position[3];
    std::memcpy(position, bytes(body) + 680, 12);
    alignas(4) Byte ground[84];
    std::memcpy(ground, bytes(body) + 508, 84);
    if (!physics::surface_height(ground, position))
        return;
    const auto format = kernel::numeric::current_format();
    const float height = float(Extended(at<float>(body, 684), format) - position[1]);
    if (height < 20.f || std::isnan(height)) {
        const float density = float(Extended(1.f, format) - Extended(height, format) * 0.05f);
        const float radius = float(Extended(density, format) * 0.5f * 6.f);
        emit_world_effect(position, reinterpret_cast<const float *>(bytes(body) + 716), 19,
                          float(Extended(density, format) * 0.8f + 0.2f),
                          reinterpret_cast<const float *>(bytes(body) + 8), radius,
                          float(Extended(radius, format) + 1.f));
    }
}
} // namespace nfs::game::runtime::ai::entities::pending
