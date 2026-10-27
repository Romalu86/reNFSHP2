#include "../../network/vehicles/remote_car_runtime.hpp"
#include "../population/ai_population_runtime.hpp"
#include "nfs/physics/contacts/vehicle_contacts.hpp"
#include "../../math/vectors/retained_vector.hpp"
namespace nfs::game::runtime::ai::lifecycle::pending {

Dword project_reset_position(void *owner, const float *query, void **section, int *sample,
                            float *position, float *rotation, Dword reverse, void *retained_path) {
    using kernel::numeric::Extended;
    const auto format = kernel::numeric::current_format();
    auto X = [&](float value) { return Extended(value, format); };
    alignas(4) Byte reference[48], path[88];
    reset_path_reference(reference);
    construct_population_path(path);
    try {
        Byte use_path = 0;
        if (retained_path && path_current_section(bytes(retained_path) + 16)) {
            for (unsigned word = 0; word < 10; ++word)
                at<Dword>(path, word * 4) = at<Dword>(retained_path, word * 4);
            copy_path_reference(path + 40, bytes(retained_path) + 40);
            use_path = 1;
        }
        Dword direction[4];
        const float heading[3] = {1, 0, 0};
        construct_pursuit_route(bytes(owner) + 4, reference, reinterpret_cast<Dword *>(sample),
                                direction, heading, query, path, use_path, 0);
        int node = *sample;
        if (node < 0) {
            destroy_population_path(path);
            return 0;
        }
        *section = at<void *>(reference, 8);
        if (!*section) {
            destroy_population_path(path);
            return 0;
        }
        sample_route_pose(*section, node, position, rotation, Byte(reverse));
        if (use_path) {
            const auto *source = static_cast<const float *>(section_position(*section, node));
            float start[3];
            std::memcpy(start, source, 12);
            float tangent[3]{}, delta[3];
            track::path_direction(*section, node, tangent);
            for (unsigned axis = 0; axis < 3; ++axis)
                delta[axis] = float(X(query[axis]) - start[axis]);
            const auto projection = retained_dot_product(tangent, delta);
            const float stored_projection = float(projection);
            if (X(0.f) < projection && node < at<int>(*section, 36) - 1) {
                ++node;
            } else if (stored_projection < 0 && node >= 1) {
                --node;
            } else {
                destroy_population_path(path);
                return 1;
            }
            source = static_cast<const float *>(section_position(*section, node));
            float segment[3];
            for (unsigned axis = 0; axis < 3; ++axis)
                segment[axis] = float(X(source[axis]) - start[axis]);
            const auto length = Extended::from_double(
                double(physics::pending::normalize_collision_entry(segment)), format);
            if (float(length) != 0) {
                auto absolute_projection = X(stored_projection);
                if (stored_projection < 0)
                    absolute_projection = -absolute_projection;
                auto fraction = absolute_projection / length;
                if (fraction < X(0.f))
                    fraction = X(0.f);
                else if (X(1.f) < fraction)
                    fraction = X(1.f);
                for (unsigned axis = 0; axis < 3; ++axis)
                    position[axis] = float(X(float(X(segment[axis]) * fraction)) + start[axis]);
                position[2] = -position[2];
            }
        }
        destroy_population_path(path);
        return 1;
    } catch (...) {
        destroy_population_path(path);
        throw;
    }
}
} // namespace nfs::game::runtime::ai::lifecycle::pending
