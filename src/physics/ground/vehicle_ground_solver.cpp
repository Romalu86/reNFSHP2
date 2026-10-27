#include "nfs/physics/solver/solver_runtime.hpp"
#include "../vehicles/car_physics_frame.hpp"
#include "../vehicles/car_drive_frame_x87.hpp"
#include <cmath>
#include <cstring>
namespace nfs::game::runtime::physics::pending {

int collide_ground(void *state) {
    using car_frame::drive::retained;
    alignas(4) Byte contacts[26][84];
    float offsets[26][3], normals[26][3];
    Dword active[26];
    for (auto &contact : contacts)
        construct_ground_contact(contact);
    auto *position = reinterpret_cast<float *>(bytes(state) + 680);
    for (int vertex = 0; vertex < 26; ++vertex) {
        active[vertex] = 0;
        matrices::transform_vectors(1, reinterpret_cast<float *>(bytes(state) + 140 + vertex * 12),
                                    reinterpret_cast<float *>(bytes(state) + 692),
                                    offsets[vertex]);
        if (offsets[vertex][1] > 0)
            continue;
        std::memcpy(contacts[vertex], bytes(state) + 508, 84);
        float point[3];
        for (int i = 0; i < 3; ++i)
            point[i] = position[i] + offsets[vertex][i];
        if (!sample_ground_entry(point, contacts[vertex]))
            continue;
        float normal[3], delta[3];
        for (int i = 0; i < 3; ++i) {
            normal[i] = at<float>(contacts[vertex], 12 + i * 4);
            delta[i] = point[i] - at<float>(contacts[vertex], i * 4);
        }
        const float penetration = float(hardware::dot_product(delta, normal));
        const auto toward = hardware::dot_product(offsets[vertex], normal);
        const auto anticipated = retained(penetration) -
                                 retained(at<float>(state, 12)) * 0.0333333350718021392822265625f;
        if (anticipated < retained(-1.0f) || !(penetration < 0) ||
            !(toward < retained(0.f)))
            continue;
        const auto distance = hardware::dot_product(delta, normal);
        if (distance < retained(0.f)) {
            float correction[3], corrected[3];
            scale_vectors(1, normal, float(-distance), correction);
            add_vector(position, correction, corrected);
            for (int i = 0; i < 3; ++i)
                position[i] = corrected[i];
            add_vector(point, correction, point);
        }
        for (int i = 0; i < 3; ++i)
            normals[vertex][i] = normal[i];
        active[vertex] = 1;
    }
    int resolved = 0;
    for (int vertex = 0; vertex < 26; ++vertex)
        if (active[vertex]) {
            float point[3], delta[3];
            for (int i = 0; i < 3; ++i) {
                point[i] = offsets[vertex][i] + position[i];
                delta[i] = point[i] - at<float>(contacts[vertex], i * 4);
            }
            const auto separation = hardware::dot_product(delta, normals[vertex]);
            // TEST AH,41h / JP rejects greater/unordered and accepts equality.
            if (retained(0.001f) < separation || __builtin_isnan(separation))
                continue;
            const float friction[2] = {saved::ground_friction[0], saved::ground_friction[1]};
            const Byte material =
                !at<void *>(contacts[vertex], 24) ? 0 : at<Byte>(contacts[vertex], 78) & 31;
            Dword *properties = static_cast<Dword *>(surface_properties(material));
            if (resolve_ground_point(state, point, normals[vertex], friction,
                                              *properties)) {
                ++resolved;
                at<Dword>(state, 652) = 1;
            }
        }
    return resolved;
}
} // namespace nfs::game::runtime::physics::pending
