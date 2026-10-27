#include "camera_director_runtime.hpp"
#include "../../math/vectors/retained_vector.hpp"
#include "nfs/audio/vehicles/vehicle_audio.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
namespace nfs::game::world::camera {
using kernel::numeric::Extended;

Extended plane_height(const float *normal, const float *origin, const float *point) {
    const auto format = kernel::numeric::current_format();
    auto X = [&](float value) { return Extended(value, format); };
    return X(origin[1]) -
           ((X(point[2]) - origin[2]) * normal[2] + (X(point[0]) - origin[0]) * normal[0]) /
               X(normal[1]);
}

void raise_camera_above_ground(void *ground, float *position, float clearance) {
    if (!at<Dword>(ground, 24))
        return;
    const auto height = plane_height(reinterpret_cast<float *>(offset(ground, 12)),
                                     static_cast<float *>(ground), position);
    const auto raised = height + clearance;
    if (Extended(position[1], kernel::numeric::current_format()) < raised)
        position[1] = float(raised);
}

Dword refresh_camera_ground(const float *position, void *body, void *ground) {
    if (body)
        std::memcpy(ground, offset(body, 508), 84);
    alignas(4) Byte candidate[84];
    std::memcpy(candidate, ground, sizeof(candidate));
    if (!runtime::physics::sample_ground(position, candidate))
        return 0;
    std::memcpy(ground, candidate, sizeof(candidate));
    return 1;
}
// The fifth caller word is unused; only AL is consumed.
Byte clip_camera_position(void *ground, float *position, void *body, float radius, Byte) {
    void *car = runtime::physics::vehicle_car(body);
    if (!car || !game::audio::vehicle::car_audio_suppressed(car))
        return 0;
    alignas(4) Byte contact[84];
    std::memcpy(contact, ground, sizeof(contact));
    void *cached = at<void *>(contact, 24), *current = at<void *>(body, 532);
    Dword surface =
        ((cached ? at<Byte>(cached, 59) : 0) | (current ? at<Byte>(current, 59) : 0)) & 15;
    const Dword car_slot = at<Dword>(car, 1776);
    float normal[3]{}, hit[3]{};
    Dword reference;
    const auto format = kernel::numeric::current_format();
    auto X = [&](float value) { return Extended(value, format); };
    Byte corrected = 0;
    if (runtime::physics::contacts_pending::find_body_edge_contact(
            reinterpret_cast<float *>(offset(body, 680)), position, normal, hit, &reference,
            car_slot, surface)) {
        float delta[3];
        for (unsigned axis = 0; axis < 3; ++axis)
            delta[axis] = float(X(position[axis]) - hit[axis]);
        delta[1] = 0;
        const auto projection = runtime::retained_dot_product(normal, delta);
        const float saved_projection = float(projection);
        if (projection < X(0.f)) {
            corrected = 1;
            const auto correction = X(radius) * 0.5f - saved_projection;
            const float y = float(X(normal[1]) * correction);
            const float z = float(X(normal[2]) * correction);
            position[0] = float(X(normal[0]) * correction + position[0]);
            position[1] = float(X(y) + position[1]);
            position[2] = float(X(z) + position[2]);
        }
    }
    surface = cached ? at<Byte>(cached, 59) & 15 : 0;
    for (int ray = 0; ray < 16; ++ray) {
        float endpoint[3];
        std::memcpy(endpoint, position, 12);
        const float angle = float(Extended::integer(ray, format) * 0.0625f);
        endpoint[0] =
            float(Extended::from_double(double(runtime::turn_sine(angle)), format) * radius +
                  endpoint[0]);
        endpoint[2] =
            float(Extended::from_double(double(runtime::turn_cosine(angle)), format) * radius +
                  endpoint[2]);
        float ray_normal[3]{}, ray_hit[3]{};
        if (runtime::physics::contacts_pending::find_body_edge_contact(
                position, endpoint, ray_normal, ray_hit, &reference, car_slot, surface)) {
            const float delta[3] = {float(X(endpoint[0]) - ray_hit[0]), 0,
                                    float(X(endpoint[2]) - ray_hit[2])};
            const auto projection = runtime::retained_dot_product(ray_normal, delta);
            if (projection < X(0.f)) {
                corrected = 1;
                const float z = float(X(ray_normal[2]) * -projection);
                position[0] = float(X(ray_normal[0]) * -projection + position[0]);
                position[2] = float(X(z) + position[2]);
            }
        }
    }
    return corrected;
}
// Failure follows the ordered, finite path.
 
Byte displace_from_vehicle(float *position, const float *center, const float *extent,
                           const float *rotation, float radius, Byte outside) {
    const auto format = kernel::numeric::current_format();
    auto X = [&](float value) { return Extended(value, format); };
    float delta[3], local[3], bounds[3];
    for (unsigned axis = 0; axis < 3; ++axis)
        delta[axis] = float(X(position[axis]) - center[axis]);
    for (unsigned axis = 0; axis < 3; ++axis) {
        local[axis] = float(runtime::retained_dot_product(delta, rotation + axis * 3));
        bounds[axis] = float(X(radius) + extent[axis]);
    }
    const float absolute[3] = {std::fabs(local[0]), std::fabs(local[1]), std::fabs(local[2])};
    if (!(absolute[0] < bounds[0] && absolute[1] < bounds[1] && absolute[2] < bounds[2]))
        return 0;
    float sign = local[1] <= 0 ? -1.f : 1.f;
    if (!outside)
        sign = -sign;
    const auto ellipse = X(1.f) - (X(local[2]) * local[2] / (X(bounds[2]) * bounds[2]) +
                                  X(local[0]) * local[0] / (X(bounds[0]) * bounds[0]));
    if (ellipse < X(0.f) || std::isnan(double(ellipse)))
        return 0;
    const float squared_height = float(X(bounds[1]) * bounds[1] * ellipse);
    const float height = float(X(squared_height).square_root());
    if (!(absolute[1] < height))
        return 0;
    const float displaced[3] = {local[0], float(X(height) * sign), local[2]};
    float inverse[9];
    runtime::matrices::transpose(rotation, inverse);
    const float world_x = float(runtime::retained_dot_product(displaced, inverse));
    float world_y = float(runtime::retained_dot_product(displaced, inverse + 3));
    const auto world_z = runtime::retained_dot_product(displaced, inverse + 6);
    if (world_y < 0)
        world_y = -world_y;
    position[0] = float(X(world_x) + center[0]);
    position[1] = float(X(world_y) + center[1]);
    position[2] = float(world_z + center[2]);
    return 1;
}

Byte avoid_camera_vehicles(float *position, void *car, float radius) {
    if (!car || !position)
        return 0;
    void *body = at<void *>(car, 1768);
    displace_from_vehicle(position, reinterpret_cast<float *>(offset(body, 680)),
                          reinterpret_cast<float *>(offset(body, 120)),
                          reinterpret_cast<float *>(offset(body, 692)), radius, 1);
    void *entity = at<void *>(car, 7988);
    Byte corrected = 0;
    if (!entity)
        return 0;
    for (int index = 0; index < at<int>(entity, 8); ++index) {
        void *neighbor = at<void *>(entity, 12 + Dword(index) * 4);
        if (!neighbor)
            continue;
        auto interface_at = [](void *object, unsigned slot) {
            return offset(object, 4 + at<const int *>(object, 4)[slot]);
        };
        if (!invoke<void *>(interface_at(neighbor, 2), 1))
            continue;
        void *vehicle = invoke<void *>(interface_at(neighbor, 2), 1);
        if (!vehicle)
            continue;
        void *other = invoke<void *>(interface_at(vehicle, 3), 0);
        if (!other || !at<Byte>(other, 728))
            continue;
        if (Extended(20.f, kernel::numeric::current_format()) <
            runtime::retained_point_distance(reinterpret_cast<float *>(offset(other, 680)),
                                             position))
            return corrected;
        corrected =
            displace_from_vehicle(position, reinterpret_cast<float *>(offset(other, 680)),
                                  reinterpret_cast<float *>(offset(other, 120)),
                                  reinterpret_cast<float *>(offset(other, 692)), radius, 1);
    }
    return corrected;
}
} // namespace nfs::game::world::camera
