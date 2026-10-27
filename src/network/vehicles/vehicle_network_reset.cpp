#include "remote_car_runtime.hpp"
#include "../../graphics/cameras/camera_director_runtime.hpp"
#include "nfs/physics/contacts/vehicle_contacts.hpp"
#include "nfs/ai/lifecycle/ai_lifecycle.hpp"
#include "nfs/graphics/particles/particle_lifecycle.hpp"
#include "../../physics/bodies/shape_body_runtime.hpp"
#include <cmath>
namespace nfs::game::runtime::physics {
using kernel::numeric::Extended;

Dword request_car_reset(void *car, float speed) {
    if (!race_started || at<Dword>(car, 1800) || race_ticks < 600)
        return 0;
    void *entity = at<void *>(car, 7988);
    if (entity && (invoke<Dword>(entity, 11) || ai::racer(entity)) &&
        at<float>(car, 3168) > 26.8218f)
        return 0;
    for (int wheel = 0; wheel < 4; ++wheel)
        particles::lifecycle::release_wheel_skid(car, wheel);
    contacts_pending::find_reset_transform(car);
    void *body = at<void *>(car, 1768);
    at<Dword>(body, 8) = 0;
    at<Dword>(body, 12) = 0;
    at<Dword>(body, 16) = 0;
    reset_car(static_cast<Dword *>(car));
    contacts_pending::clear_car_controls(car);
    const int engine_frequency = at<int>(car, 656);
    at<Byte>(car, 1584) = 2;
    at<Byte>(car, 1582) = 2;
    at<Dword>(car, 96) = 2;
    at<Dword>(at<void *>(car, 1768), 732) = at<Dword>(system(68), 692);
    const Dword recovery = at<Dword>(system(68), 696);
    at<float>(car, 4) = float(
        Extended::integer(engine_frequency, kernel::numeric::current_format()) * 0.10471975803375244140625f);
    at<Dword>(car, 1800) = recovery;
    ai::lifecycle::pending::set_car_speed(car, speed);
    return 1;
}

void sample_track_reset_pose(void *collision, int marker, float *position, float *rotation) {
    void *path = at<void *>(collision, 28);
    if (!path)
        return;
    const auto *point =
        reinterpret_cast<const float *>(bytes(at<void *>(path, 32)) + Dword(marker) * 12);
    std::memcpy(position, point, 12);
    position[2] = -position[2];
    const int next = static_cast<std::int32_t>(Dword(marker) + 1);
    marker = next < at<int>(path, 20) ? next : 0;
    alignas(4) Byte ground[84];
    pending::construct_ground_contact(ground);
    clear_ground_contact(ground);
    surface_height(ground, position);
    point = reinterpret_cast<const float *>(bytes(at<void *>(path, 32)) + Dword(marker) * 12);
    const float next_x = point[0], next_z = -point[2];
    // Both TEST AH,44 parity branches must take the ordered-equal path.
    if (next_x == position[0] && next_z == position[2]) {
        const float identity[9] = {1, 0, 0, 0, 1, 0, 0, 0, 1};
        std::memcpy(rotation, identity, sizeof(identity));
        return;
    }
    const auto format = kernel::numeric::current_format();
    float direction[3] = {float(Extended(next_x, format) - position[0]), 0,
                          float(Extended(next_z, format) - position[2])};
    normalize_vector(direction, direction);
    rotation[6] = direction[0];
    rotation[2] = -direction[0];
    rotation[7] = 0;
    rotation[8] = direction[2];
    rotation[3] = 0;
    rotation[4] = 1;
    rotation[5] = 0;
    rotation[0] = direction[2];
    rotation[1] = 0;
}
} // namespace nfs::game::runtime::physics
namespace nfs::game::runtime::physics::contacts_pending {

void find_reset_transform(void *car) {
    using kernel::numeric::Extended;
    alignas(4) Byte ground[84];
    pending::construct_ground_contact(ground);
    void *entity = at<void *>(car, 7988);
    if (!entity)
        return;
    void *body = at<void *>(car, 1768);
    const float query[3] = {at<float>(body, 680), at<float>(body, 684),
                            -at<float>(body, 688)};
    Byte reverse = 0;
    if ((invoke<Dword>(entity, 11) || ai::racer(entity)) &&
        invoke<Dword>(bytes(entity) + 4 + at<const int *>(entity, 4)[2], 4) &&
        at<int>(system(72), 152) != 10 && !at<Byte>(entity, 409)) {
        const auto direction = world::camera::route_heading_alignment(entity);
        reverse = direction < Extended(0.f, kernel::numeric::current_format());
    }
    void *section;
    int sample;
    float projected_position[3], projected_rotation[9];
    const Byte projected = Byte(ai::lifecycle::pending::project_reset_position(
        system(76), query, &section, &sample, projected_position, projected_rotation, reverse,
        bytes(entity) + 220));
    if (projected) {
        std::memcpy(ground, bytes(body) + 508, sizeof(ground));
        bool found = surface_height(ground, projected_position) != 0;
        if (!found) {
            projected_position[1] =
                float(Extended(projected_position[1], kernel::numeric::current_format()) + 100.f);
            std::memcpy(ground, bytes(body) + 508, sizeof(ground));
            found = surface_height(ground, projected_position) != 0;
        }
        if (found) {
            projected_position[1] =
                float(Extended(projected_position[1], kernel::numeric::current_format()) + 1.5f);
            std::memcpy(bytes(body) + 680, projected_position, 12);
            std::memcpy(bytes(body) + 692, projected_rotation, 36);
            matrices::transpose(reinterpret_cast<float *>(bytes(body) + 692),
                                reinterpret_cast<float *>(bytes(body) + 72));
            std::memcpy(bytes(body) + 508, ground, sizeof(ground));
            return;
        }
    }
    float fallback_position[3], fallback_rotation[9];
    sample_track_reset_pose(track_collision, at<int>(car, 7984), fallback_position,
                            fallback_rotation);
    std::memcpy(ground, bytes(body) + 508, sizeof(ground));
    if (surface_height(ground, fallback_position)) {
        fallback_position[1] =
            float(Extended(fallback_position[1], kernel::numeric::current_format()) + 1.5f);
        std::memcpy(bytes(body) + 508, ground, sizeof(ground));
        std::memcpy(bytes(body) + 680, fallback_position, 12);
        std::memcpy(bytes(body) + 692, fallback_rotation, 36);
        matrices::transpose(reinterpret_cast<float *>(bytes(body) + 692),
                            reinterpret_cast<float *>(bytes(body) + 72));
    }
}
} // namespace nfs::game::runtime::physics::contacts_pending
