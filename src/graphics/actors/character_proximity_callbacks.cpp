#include "character_runtime.hpp"
#include "../../math/vectors/retained_vector.hpp"
#include "../cameras/camera_controller_runtime.hpp"
#include <cstring>
namespace nfs::game::world::actor_resources {
using kernel::numeric::Extended;

void set_channel_offset(void *channels, int index, Dword value) {
    void *channel = bytes(at<void *>(channels, 12)) + Dword(index) * 20;
    at<Dword>(channel, 8) = value;
    at<Dword>(channel, 4) = value;
}
// Cdecl callback consumes only the first of its caller's two words.
void freeze_actor_channel_offsets(void *actor, Dword) {
    at<Byte>(actor, 60) = 1;
    set_channel_offset(offset(actor, 52), 0, at<Dword>(at<void *>(actor, 64), 12));
    set_channel_offset(offset(actor, 52), 1, at<Dword>(at<void *>(actor, 64), 32));
    set_channel_offset(offset(actor, 52), 2, at<Dword>(at<void *>(actor, 64), 52));
}

void add_actor_position(const float *position, const float *displacement, float *output) {
    const auto format = kernel::numeric::current_format();
    for (unsigned axis = 0; axis < 3; ++axis)
        output[axis] = float(Extended(position[axis], format) + displacement[axis]);
}

Dword place_actor_on_ground(void *actor) {
    const auto format = kernel::numeric::current_format();
    float position[3], displacement[3], proposed[3];
    invoke(actor, 0, position);
    actor_displacement(actor, displacement);
    displacement[2] = -displacement[2];
    for (unsigned axis = 0; axis < 3; ++axis)
        proposed[axis] = float(Extended(displacement[axis], format) + position[axis]);
    if (!runtime::physics::surface_height(offset(actor, 180), proposed))
        return 0;
    at<float>(actor, 124) = proposed[0];
    at<float>(actor, 128) = float(Extended(at<float>(actor, 116), format) + proposed[1]);
    at<float>(actor, 132) = -proposed[2];
    return 1;
}
// The EAX result is the caller-supplied output pointer.
float *actor_avoidance_direction(float *output, void *actor) {
    const auto format = kernel::numeric::current_format();
    auto E = [&](float value) { return Extended(value, format); };
    void *car = runtime::physics::local_car(runtime::system(68), 0);
    const auto speed = runtime::retained_vector_length(&at<float>(at<void *>(car, 1768), 8));
    float side[3];
    if (speed < E(1)) {
        const float facing[3] = {at<float>(actor, 136), at<float>(actor, 140),
                                 -at<float>(actor, 144)};
        runtime::scale_vectors(1, facing, -1, side);
        runtime::normalize_vector(side, side);
    } else {
        car = runtime::physics::local_car(runtime::system(68), 0);
        float direction[3];
        runtime::normalize_vector(&at<float>(at<void *>(car, 1768), 8), direction);
        runtime::cross_vectors_entry(direction, camera::up_direction, side);
    }
    float position[3];
    actor_world_position(actor, position);
    car = runtime::physics::local_car(runtime::system(68), 0);
    void *body = at<void *>(car, 1768);
    float to_car[3] = {float(E(at<float>(body, 680)) - position[0]),
                       float(E(at<float>(body, 684)) - position[1]),
                       float(E(-at<float>(body, 688)) - position[2])};
    runtime::normalize_vector(to_car, to_car);

    const float dot = float(runtime::retained_dot_product(to_car, side));
    if (dot > 0)
        runtime::scale_vectors(1, side, -1, side);
    std::memcpy(output, side, 12);
    return output;
}
namespace {
Extended proximity_distance_limit() {
    const auto format = kernel::numeric::current_format();
    void *car = runtime::physics::local_car(runtime::system(68), 0);
    void *body = at<void *>(car, 1768);
    const float velocity[3] = {at<float>(body, 8), 0, at<float>(body, 16)};
    auto speed = runtime::retained_vector_length(velocity);
    if (speed < Extended(10.f, format))
        speed = Extended(10.f, format);
    return speed / Extended(40.f, format) * 15.f;
}
void advance_proximity_animation(void *actor, Dword near_animation, Dword far_animation,
                                 bool require_near_animation) {
    if (!place_actor_on_ground(actor))
        yaw_actor(actor, 5);
    float position[3];
    actor_world_position(actor, position);
    void *car = runtime::physics::local_car(runtime::system(68), 0);
    void *body = at<void *>(car, 1768);
    float car_position[3] = {at<float>(body, 680), at<float>(body, 684),
                             -at<float>(body, 688)};
    const float distance = float(runtime::retained_point_distance(position, car_position));
    const auto format = kernel::numeric::current_format();
    const auto retained_distance = Extended(distance, format);
    if (retained_distance < proximity_distance_limit()) {
        float direction[3];
        actor_avoidance_direction(direction, actor);
        std::memcpy(offset(actor, 136), direction, 12);
        at<float>(actor, 144) = -at<float>(actor, 144);
        if (at<Dword>(actor, 92) != near_animation)
            vehicle_render::pending::select_character_animation(actor, near_animation, 0);
    }
    const auto far_limit = proximity_distance_limit();
    const Dword current = at<Dword>(actor, 92);
    if (far_limit + far_limit < retained_distance &&
        (require_near_animation ? current == near_animation : current != far_animation))
        vehicle_render::pending::select_character_animation(actor, far_animation, 0);
}
} // namespace

void update_actor_proximity_loop(void *actor) {
    advance_proximity_animation(actor, 10102, 10101, false);
}

void update_actor_proximity_transition(void *actor) {
    advance_proximity_animation(actor, 10102, 10103, true);
}

void update_actor_proximity_pose(void *actor) {
    advance_proximity_animation(actor, 10404, 10401, true);
}

void update_actor_proximity_motion(void *actor) {
    advance_proximity_animation(actor, 10404, 20408, true);
}
} // namespace nfs::game::world::actor_resources
