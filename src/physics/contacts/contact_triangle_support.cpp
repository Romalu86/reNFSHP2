#include "nfs/physics/frame/dynamics_runtime.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
#include "../vehicles/car_physics_frame.hpp"
#include <cstring>
namespace nfs::game::runtime::physics {

int surface_height(void *contact, float *position) {
    if (!pending::sample_ground_entry(position, contact))
        return 0;
    const auto z_slope =
        (static_cast<long double>(position[2]) - at<float>(contact, 8)) *
        at<float>(contact, 20);
    const auto x_slope =
        (static_cast<long double>(position[0]) - at<float>(contact, 0)) *
        at<float>(contact, 12);
    const auto height_offset =
        (z_slope + x_slope) / static_cast<long double>(at<float>(contact, 16));
    position[1] = float(static_cast<long double>(at<float>(contact, 4)) - height_offset);
    return 1;
}
// Three support vertices, penetration correction and friction.
int triangle_support(void *body, const float *triangle, float clearance) {
    auto E = [](float value) { return static_cast<long double>(value); };
    float surface[3][3], normals[3][3], height[3], minimum = 0;
    for (unsigned vertex = 0; vertex < 3; ++vertex) {
        alignas(4) Byte contact[84];
        std::memcpy(contact, bytes(body) + 508, 84);
        copy_triplet(surface[vertex], triangle + vertex * 3);
        if (surface_height(contact, surface[vertex]))
            copy_triplet(normals[vertex], reinterpret_cast<float *>(contact + 12));
        else
            construct_triplet(normals[vertex], 0, 1, 0);
        const auto difference = E(triangle[vertex * 3 + 1]) - surface[vertex][1];
        height[vertex] = float(difference);
        if (!vertex || difference < E(minimum))
            minimum = float(difference);
    }
    if (minimum > clearance)
        return 0;
    float angular[3];
    scale_vectors(1, reinterpret_cast<float *>(bytes(body) + 20), 6.283185482025146484375f, angular);
    float selected[9], selected_surface[9];
    unsigned count = 0;
    for (unsigned vertex = 0; vertex < 3; ++vertex) {
        float arm[3], rotational[3], velocity[3];
        contact_arm(body, triangle + vertex * 3, arm);
        cross_vectors(angular, arm, rotational);
        for (unsigned i = 0; i < 3; ++i)
            velocity[i] = float(E(rotational[i]) + at<float>(body, 8 + i * 4));
        const auto approach = hardware::dot_product(velocity, normals[vertex]);
        const auto predicted =
            (approach - E(car_frame::gravity) * 0.0333333350718021392822265625f) * 0.0333333350718021392822265625f + height[vertex];
        if (predicted < E(clearance)) {
            copy_triplet(selected + count * 3, triangle + vertex * 3);
            copy_triplet(selected_surface + count * 3, surface[vertex]);
            ++count;
        }
    }
    if (count < 3)
        return 0;
    float shape_normal[3], surface_normal[3], tilt[3];
    triangle_normal(shape_normal, selected);
    triangle_normal(surface_normal, selected_surface);
    cross_vectors(shape_normal, surface_normal, tilt);
    at<float>(body, 20) = float(E(at<float>(body, 20)) * 0.875f + E(tilt[0]) * 0.3333f);
    at<float>(body, 28) = float(E(at<float>(body, 28)) * 0.875f + E(tilt[2]) * 0.3333f);
    auto *linear = reinterpret_cast<float *>(bytes(body) + 8);
    auto *rotation = reinterpret_cast<float *>(bytes(body) + 20);
    float delta[3];
    if (minimum <= 0) {
        scale_vectors(1, surface_normal, -minimum, delta);
        accumulate_triplet(reinterpret_cast<float *>(bytes(body) + 680), delta);
        const auto projected = hardware::dot_product(surface_normal, linear);
        if (projected < E(0)) {
            scale_vectors(1, surface_normal, float(-projected), delta);
            accumulate_triplet(linear, delta);
        }
    }
    // Native coefficients are captured before the following normalization/cross calls.
    const float static_factor = saved::ground_friction[0];
    const float dynamic_factor = saved::ground_friction[1];
    const auto retained_normal_load = E(surface_normal[1]) * at<float>(body, 32) * 9.81f;
    const float normal_load = float(retained_normal_load);
    if (!(retained_normal_load <= E(0))) {
        float crossed[3], tangent[3];
        cross_vectors(linear, surface_normal, crossed);
        cross_vectors(crossed, surface_normal, tangent);
        const float speed = hardware::normalize_collision(tangent);
        const float spin = float(hardware::dot_product(rotation, surface_normal) * 6.283185482025146484375f);
        auto translation_drag = E(speed) * 16.f;
        auto rotation_drag = E(spin) * 16.f;
        if (translation_drag < E(0))
            translation_drag = -translation_drag;
        if (rotation_drag < E(0))
            rotation_drag = -rotation_drag;
        const float drag = float((rotation_drag + translation_drag) * at<float>(body, 32));
        if (drag != 0) {
            float coefficient;
            const auto static_friction = E(static_factor) * normal_load;
            if (!(static_friction >= E(drag))) {
                at<Dword>(body, 636) = 2;
                coefficient = float(E(dynamic_factor) * normal_load / E(drag));
            } else {
                coefficient = 1;
                at<Dword>(body, 636) = 1;
            }
            float tangent_change[3], spin_change[3];
            scale_vectors(1, tangent, float(E(coefficient) * speed), tangent_change);
            scale_vectors(1, surface_normal, float(E(coefficient) * spin * -0.15915493667125701904296875f),
                          spin_change);
            accumulate_triplet(linear, tangent_change);
            accumulate_triplet(rotation, spin_change);
            for (unsigned i = 0; i < 3; ++i)
                at<float>(body, 624 + i * 4) = float(E(tangent[i]) * drag);
        }
    }
    return 1;
}
} // namespace nfs::game::runtime::physics
