#include "contact_boxes.hpp"
#include "../motion/world_motion_x87.hpp"
#include <cstring>
namespace nfs::game::runtime::physics::boxes {
// The saved fast path uses max component speed, not length.
void select_normal(void *first, void *second, Dword reverse) {
    auto E = [](float value) { return static_cast<long double>(value); };
    if (reverse) {
        void *temporary = first;
        first = second;
        second = temporary;
    }
    auto first_speed = E(at<float>(second, 8)) - at<float>(first, 8);
    if (first_speed < E(0))
        first_speed = -first_speed;
    float speed[3] = {float(first_speed), 0, 0};
    for (unsigned i = 1; i < 3; ++i) {
        auto component_speed = E(at<float>(second, 8 + i * 4)) - at<float>(first, 8 + i * 4);
        if (component_speed < E(0))
            component_speed = -component_speed;
        speed[i] = float(component_speed);
    }
    auto maximum = first_speed;
    if (speed[0] < speed[1])
        maximum = E(speed[1]);
    if (maximum < E(speed[2]))
        maximum = E(speed[2]);
    if (!(E(15) < maximum)) {
        const auto depth = [&](unsigned axis) {
            const float coordinate = probe_reversed[2 - axis];
            return coordinate < 0 ? E(coordinate) + at<float>(first, 120 + axis * 4)
                                  : E(at<float>(first, 120 + axis * 4)) - coordinate;
        };
        const float depth_x = float(depth(0));
        const auto depth_y = depth(1), depth_z = depth(2);
        unsigned axis;
        if (E(depth_x) < depth_y && E(depth_x) < depth_z)
            axis = 0;
        else
            axis = depth_y < depth_z ? 1 : 2;
        const float coordinate = probe_reversed[2 - axis];
        Dword normal_bits[3];
        for (unsigned i = 0; i < 3; ++i)
            normal_bits[i] = at<Dword>(first, 692 + axis * 12 + i * 4);
        std::memcpy(pending::collision_normal + 1, normal_bits + 1, sizeof(Dword));
        std::memcpy(pending::collision_normal, normal_bits, sizeof(Dword));
        std::memcpy(pending::collision_normal + 2, normal_bits + 2, sizeof(Dword));
        bool invert = reverse ? coordinate > 0 : coordinate < 0;
        if (invert)
            for (float &component : pending::collision_normal)
                component = float(-E(component));
        return;
    }
    float axes[3][3], velocity[3];
    copy_triplet(velocity, reinterpret_cast<float *>(bytes(second) + 8));
    for (unsigned axis = 0; axis < 3; ++axis) {
        copy_triplet(axes[axis], reinterpret_cast<float *>(bytes(first) + 692 + axis * 12));
        const float coordinate = probe_reversed[2 - axis];
        if (reverse ? coordinate > 0 : coordinate < 0)
            for (float &component : axes[axis])
                component = float(-E(component));
    }
    const auto projection = [&](unsigned axis) {
        auto value = hardware::dot_product(axes[axis], velocity);
        return value < E(0) ? -value : value;
    };
    const float first_projection = float(projection(0));
    const float second_projection = float(projection(1));
    const auto third_projection = projection(2);
    unsigned axis;
    if (second_projection < first_projection && third_projection < E(first_projection))
        axis = 0;
    else
        axis = third_projection < E(second_projection) ? 1 : 2;
    copy_triplet(pending::collision_normal, axes[axis]);
}
} // namespace nfs::game::runtime::physics::boxes
