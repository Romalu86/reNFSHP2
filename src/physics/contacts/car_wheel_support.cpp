#include "nfs/physics/contacts/vehicle_contacts.hpp"
#include "../vehicles/car_physics_frame.hpp"
#include "../motion/world_motion_x87.hpp"
#include <cstring>
namespace nfs::game::runtime::physics::contacts_pending {
void notify_ground_impact(void *, float);
// The three writes are sequential, including overlapping inputs.
void add_contact_velocity(const float *first, const float *second, float *output) {
    for (unsigned axis = 0; axis < 3; ++axis)
        output[axis] = float(static_cast<long double>(first[axis]) + second[axis]);
}

Dword resolve_wheel_contacts(void *car, const float *wheel_positions, float tolerance) {
    auto E = [](float value) { return static_cast<long double>(value); };
    using runtime::turn_direction_retained;
    void *body = at<void *>(car, 1768);
    float ground_points[4][3], ground_normals[4][3], separations[4], minimum = 0;
    for (unsigned wheel = 0; wheel < 4; ++wheel) {
        alignas(4) Byte ground[84];
        std::memcpy(ground, bytes(body) + 508, sizeof ground);
        std::memcpy(ground_points[wheel], wheel_positions + wheel * 3, 12);
        if (surface_height(ground, ground_points[wheel]))
            std::memcpy(ground_normals[wheel], ground + 12, 12);
        else {
            ground_normals[wheel][0] = ground_normals[wheel][2] = 0;
            ground_normals[wheel][1] = 1;
        }
        const auto height = E(ground_points[wheel][1]) - at<float>(car, 2208 + wheel * 296);
        ground_points[wheel][1] =
            float(height); // FST retains this value for the following subtraction.
        const auto separation = E(wheel_positions[wheel * 3 + 1]) - height;
        separations[wheel] = float(separation);
        if (!wheel || separation < E(minimum))
            minimum = float(separation);
    }
    // TEST AH,41 / JE rejects ordered greater only; unordered takes the support path.
    if (minimum > tolerance)
        return 0;
    float angular_radians[3];
    scale_vectors(1, &at<float>(body, 20), 6.283185482025146484375f, angular_radians);
    float supported_positions[3][3], supported_ground[3][3];
    unsigned count = 0;
    for (unsigned wheel = 0; wheel < 4; ++wheel) {
        float arm[3], velocity[3];
        contact_arm(body, wheel_positions + wheel * 3, arm);
        cross_vectors_entry(angular_radians, arm, velocity);
        add_contact_velocity(velocity, &at<float>(body, 8), velocity);
        const auto normal_speed = hardware::dot_product(velocity, ground_normals[wheel]);
        const auto predicted =
            (normal_speed - E(car_frame::gravity) * 0.0333333350718021392822265625f) * 0.0333333350718021392822265625f +
            separations[wheel] - at<float>(car, 2208 + wheel * 296);
        if ((predicted < E(tolerance) || at<Byte>(car, 2204 + wheel * 296)) && count < 3) {
            std::memcpy(supported_positions[count], wheel_positions + wheel * 3, 12);
            std::memcpy(supported_ground[count], ground_points[wheel], 12);
            ++count;
        }
    }
    if (count < 3)
        return 0;
    float wheel_normal[3], surface_normal[3];
    triangle_normal(wheel_normal, supported_positions[0]);
    triangle_normal(surface_normal, supported_ground[0]);
    float local_angular[3], local_wheel_normal[3], local_surface_normal[3];
    matrices::transform_vectors(1, &at<float>(body, 20), &at<float>(body, 72), local_angular);
    matrices::transform_vectors(1, wheel_normal, &at<float>(body, 72), local_wheel_normal);
    matrices::transform_vectors(1, surface_normal, &at<float>(body, 72), local_surface_normal);
    float angle = float(turn_direction_retained(at<float>(body, 720), at<float>(body, 708)));
    at<float>(car, 456) = angle;
    at<float>(car, 456) =
        float(E(angle) - turn_direction_retained(local_surface_normal[2], local_surface_normal[1]) +
              at<float>(car, 360));
    angle = float(turn_direction_retained(at<float>(body, 696), at<float>(body, 708)));
    at<float>(car, 460) = angle;
    at<float>(car, 460) =
        float(E(angle) - turn_direction_retained(local_surface_normal[0], local_surface_normal[1]));
    float base[3], delta[3];
    scale_vectors(1, &at<float>(body, 704), -at<float>(body, 124), base);
    for (unsigned axis = 0; axis < 3; ++axis) {
        base[axis] = float(E(base[axis]) + at<float>(body, 680 + axis * 4));
        delta[axis] = float(E(base[axis]) - supported_ground[0][axis]);
    }
    const auto height = hardware::dot_product(delta, surface_normal);
    at<float>(car, 372) = float(height);
    at<float>(car, 372) = E(0) < height ? float(height) : 0.f;
    angle = float(turn_direction_retained(local_wheel_normal[1], local_wheel_normal[2]));
    float pitch =
        float(E(angle) - turn_direction_retained(local_surface_normal[1], local_surface_normal[2]));
    at<float>(car, 364) = pitch;
    angle = float(turn_direction_retained(local_wheel_normal[0], local_wheel_normal[1]));
    float roll =
        float(E(angle) - turn_direction_retained(local_surface_normal[0], local_surface_normal[1]));
    at<float>(car, 368) = roll;
    const float pitch_min = -at<float>(car, 264), pitch_max = at<float>(car, 268);
    // In these two clamps an unordered lower comparison proceeds to the upper comparison.
    pitch = pitch_min > pitch ? pitch_min : pitch > pitch_max ? pitch_max : pitch;
    const float roll_max = at<float>(car, 272), roll_min = -roll_max;
    roll = roll_min > roll ? roll_min : roll > roll_max ? roll_max : roll;
    at<float>(car, 364) = pitch;
    at<float>(car, 368) = roll;
    const float roll_damping = float((E(at<float>(car, 644)) + at<float>(car, 640)) * .5f);
    const float pitch_damping = at<float>(car, local_angular[0] > 0 ? 640 : 644);
    const float sprung_mass = float(E(at<float>(car, 520)) - at<float>(car, 524));
    const float mass_ratio = float(E(sprung_mass) / E(at<float>(car, 520)));
    const float roll_stiffness = at<float>(car, 344);
    const float spring = at<float>(car, pitch < 0 ? 616 : 620);
    const float pitch_stiffness =
        float((E(spring) + spring) - E(at<float>(car, 544)) * 0.454545438289642333984375f * sprung_mass *
                                         car_frame::gravity * 0.01745329238474369049072265625f);
    float squared_damping =
        float(E(roll_stiffness) * 57.2957763671875f * at<float>(car, 136) * mass_ratio);
    const float roll_root = float(__builtin_sqrtl(E(squared_damping)));
    squared_damping = float(E(pitch_stiffness) * 57.2957763671875f * at<float>(car, 128) * mass_ratio);
    const float pitch_root = float(__builtin_sqrtl(E(squared_damping)));
    const auto pitch_force =
        E(pitch) * pitch_stiffness * 180.f -
        (E(pitch_root) * pitch_damping * 2.f) * local_angular[0] * 6.283185482025146484375f;
    const auto roll_force = E(roll) * roll_stiffness * 180.f -
                            (E(roll_root) * roll_damping * 2.f) * local_angular[2] * 6.283185482025146484375f;
    const float pitch_acceleration = float(E(at<float>(car, 140)) * mass_ratio * pitch_force);
    const auto roll_acceleration = E(at<float>(car, 148)) * mass_ratio * roll_force;
    local_angular[0] = float(E(pitch_acceleration) * 0.0053051649592816829681396484375f + local_angular[0]);
    local_angular[2] = float(roll_acceleration * 0.0053051649592816829681396484375f + local_angular[2]);
    float world_angular[3];
    matrices::transform_vectors(1, local_angular, &at<float>(at<void *>(car, 1768), 692),
                                world_angular);
    std::memcpy(bytes(body) + 20, world_angular, 12);
    const float vertical_speed = float(hardware::dot_product(surface_normal, &at<float>(body, 8)));
    // TEST AH,41 / JP skips positive and unordered separation, including zero.
    if (minimum < 0) {
        float correction[3];
        scale_vectors(1, surface_normal, -minimum, correction);
        for (unsigned axis = 0; axis < 3; ++axis)
            base[axis] = float(E(correction[axis]) + at<float>(body, 680 + axis * 4));
        std::memcpy(bytes(body) + 680, base, 12);
    }
    if (minimum <= 0 && vertical_speed < 0) {
        float correction[3];
        scale_vectors(1, surface_normal, -vertical_speed, correction);
        for (unsigned axis = 0; axis < 3; ++axis)
            at<float>(body, 8 + axis * 4) =
                float(E(correction[axis]) + at<float>(body, 8 + axis * 4));
    }
    if (at<std::uint16_t>(body, 596) && vertical_speed < 0)
        notify_ground_impact(car, -vertical_speed);
    return 1;
}
} // namespace nfs::game::runtime::physics::contacts_pending
