#include "nfs/physics/contacts/vehicle_contacts.hpp"
#include "../motion/world_motion_x87.hpp"
#include <algorithm>
#include <initializer_list>
namespace nfs::game::runtime::physics {

void update_car_damage(Dword *damage_state) {
    auto number = [](float value) { return static_cast<long double>(value); };
    void *car = at<void *>(damage_state, 28);
    void *body = at<void *>(car, 1768);
    if (at<Dword>(body, 732) || at<float>(body, 608) == 0)
        return;
    auto impulse = number(at<float>(body, 608)) * 0.001f;
    if (impulse < number(5))
        return;
    impulse = impulse * at<float>(car, 1568);
    const auto vertical_impulse = number(0.025f) * impulse;
    const float damage_speed = float(impulse * 0.02222222276031970977783203125f);
    const float lateral_impulse = float(impulse * 0.0333333350718021392822265625f);
    const float vertical_damage = float(vertical_impulse * vertical_impulse);
    const float lateral_damage = float(number(lateral_impulse) * lateral_impulse);
    float local_point[3] = {float(number(at<float>(body, 640)) - at<float>(body, 680)),
                            float(number(at<float>(body, 644)) - at<float>(body, 684)),
                            float(number(at<float>(body, 648)) - at<float>(body, 688))};
    float transformed[3];
    matrices::transform_vectors(1, local_point,
                                reinterpret_cast<float *>(bytes(at<void *>(car, 1768)) + 72),
                                transformed);
    for (unsigned axis = 0; axis < 3; ++axis)
        local_point[axis] = transformed[axis];
    body = at<void *>(car, 1768);
    matrices::transform_vectors(1, reinterpret_cast<float *>(bytes(body) + 612),
                                reinterpret_cast<float *>(bytes(body) + 72), transformed);
    float local_normal[3] = {transformed[0], transformed[1], transformed[2]};
    hardware::normalize_collision(local_normal);
    contacts_pending::accumulate_wheel_damage(damage_state, local_point,
                                              float(number(damage_speed) * damage_speed));
    body = at<void *>(at<void *>(damage_state, 28), 1768);
    if (number(at<float>(body, 128)) * .25f < number(local_point[2]) && lateral_damage > 3.0f) {
        auto next = number(lateral_damage) * 0.02f + at<float>(damage_state, 8);
        if (next < number(0))
            next = number(0);
        else if (number(1) < next)
            next = number(1);
        at<float>(damage_state, 8) = float(next);
    }
    if (!at<Dword>(body, 732) &&
        (!(number(local_point[1]) < number(at<float>(body, 124)) * -.5f) ||
         !(local_normal[1] > .5f))) {
        float bounded_damage = vertical_damage;
        if (vertical_damage < 0)
            bounded_damage = 0;
        else if (vertical_damage > 1)
            bounded_damage = 1;
        void *renderable = at<void *>(at<void *>(damage_state, 28), 3372);
        if (renderable) {
            float point[3] = {local_point[0], local_point[1], local_point[2]};
            contacts_pending::apply_surface_damage(renderable, point, bounded_damage);
        }
    }
}

Dword collide_wheels(void *car) {
    void *body = at<void *>(car, 1768);
    float extent = at<float>(car, 652);
    if (extent < at<float>(car, 648))
        extent = at<float>(car, 648);
    float local[12];
    for (unsigned wheel = 0; wheel < 4; ++wheel)
        for (unsigned axis = 0; axis < 3; ++axis)
            at<Dword>(local, wheel * 12 + axis * 4) =
                at<Dword>(car, 2224 + wheel * 296 + axis * 4);
    float world[12];
    matrices::transform_positions(4, local, reinterpret_cast<float *>(bytes(body) + 692),
                                  reinterpret_cast<float *>(bytes(body) + 680), world, 12);
    const float *centre = reinterpret_cast<float *>(bytes(body) + 680);
    for (unsigned edge : {2u, 1u, 0u, 3u}) {
        const float *first = world + edge * 3;
        const float *second = world + ((edge + 1) & 3) * 3;
        const long double side = (static_cast<long double>(second[2]) - first[2]) *
                                 (static_cast<long double>(centre[0]) - first[0]);
        const long double boundary = (static_cast<long double>(centre[2]) - first[2]) *
                                     (static_cast<long double>(second[0]) - first[0]);
        if (!(boundary <= side))
            return 0;
    }
    return contacts_pending::resolve_wheel_contacts(car, world, extent);
}
} // namespace nfs::game::runtime::physics
