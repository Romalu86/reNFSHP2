#include "character_runtime.hpp"
#include "../../math/vectors/retained_vector.hpp"
#include <cstring>
namespace nfs::game::world::actor_resources {
using kernel::numeric::Extended;
namespace {
Extended path_proximity_limit() {
    const auto format = kernel::numeric::current_format();
    void *car = runtime::physics::local_car(runtime::system(68), 0);
    void *body = at<void *>(car, 1768);
    const float velocity[3] = {at<float>(body, 8), 0, at<float>(body, 16)};
    auto speed = runtime::retained_vector_length(velocity);
    if (speed < Extended(10.f, format))
        speed = Extended(10.f, format);
    return speed / Extended(40.f, format) * 15.f;
}
void advance_path_actor(void *actor, Byte walking) {
    const auto format = kernel::numeric::current_format();
    auto E = [&](float value) { return Extended(value, format); };
    if (at<Byte>(actor, 40)) {
        float position[3];
        invoke(actor, 0, position);
        const Dword height = *reinterpret_cast<const Dword *>(position + 1);
        const Dword old_x = at<Dword>(actor, 104), old_y = at<Dword>(actor, 108);
        const float distance = at<float>(actor, 112);
        position[1] = 0;
        if (distance > .0001f) {
            float next[3], direction[3];
            advance_path_cursor(offset(actor, 36), position, distance, next, direction);
            at<Dword>(actor, 104) = old_x;
            at<Dword>(actor, 108) = old_y;
            at<float>(actor, 112) = -0.f;
            std::memcpy(offset(actor, 136), direction, 12);
            at<float>(actor, 144) = -at<float>(actor, 144);
            at<float>(actor, 124) = next[0];
            at<Dword>(actor, 128) = height;
            at<float>(actor, 132) = -next[2];
            if (walking)
                place_actor_on_ground(actor);
            else {
                float point[3], displacement[3], ground[3];
                invoke(actor, 0, point);
                actor_displacement(actor, displacement);
                displacement[2] = -displacement[2];
                add_actor_position(point, displacement, ground);
                if (runtime::physics::surface_height(offset(actor, 180), ground)) {
                    at<float>(actor, 124) = ground[0];
                    at<float>(actor, 128) = float(E(at<float>(actor, 116)) + ground[1]);
                    at<float>(actor, 132) = -ground[2];
                }
            }
        }
    } else if (!place_actor_on_ground(actor))
        yaw_actor(actor, 5);
    float position[3];
    actor_world_position(actor, position);
    void *car = runtime::physics::local_car(runtime::system(68), 0);
    void *body = at<void *>(car, 1768);
    const float car_position[3] = {at<float>(body, 680), at<float>(body, 684),
                                   -at<float>(body, 688)};
    const float distance = float(runtime::retained_point_distance(position, car_position));
    if (E(distance) < path_proximity_limit()) {
        at<Byte>(actor, 40) = at<Byte>(actor, 41) = 0;
        float direction[3];
        actor_avoidance_direction(direction, actor);
        std::memcpy(offset(actor, 136), direction, 12);
        at<float>(actor, 144) = -at<float>(actor, 144);
        if (walking && at<Dword>(actor, 92) != 10102)
            vehicle_render::pending::select_character_animation(actor, 10102, 0);
    }
    const auto far_limit = path_proximity_limit();
    if (far_limit + far_limit < E(distance) &&
        (walking ? at<Dword>(actor, 92) != 10101 : !at<Byte>(actor, 40))) {
        if (walking)
            vehicle_render::pending::select_character_animation(actor, 10101, 0);
        float track_position[3];
        actor_world_position(actor, track_position);
        track_position[2] = -track_position[2];
        locate_path_cursor(offset(actor, 36), track_position);
    }
}
} // namespace entry is inside the analyzer's adjacent function range.
 
void update_actor_walking_path(void *actor) {
    advance_path_actor(actor, 1);
}

void update_actor_continuous_path(void *actor) {
    advance_path_actor(actor, 0);
}
} // namespace nfs::game::world::actor_resources
