#include "nfs/physics/contacts/vehicle_contacts.hpp"
#include "../../math/vectors/retained_vector.hpp"
#include <cmath>
#include <initializer_list>
namespace nfs::game::runtime::physics {


void mark_track_vehicle_moved(void *node) {
    void *previous = at<void *>(node, 12);
    if (previous)
        at<Dword>(previous, 8) = at<Dword>(node, 8);
    if (void *next = at<void *>(node, 8)) {
        at<void *>(next, 12) = previous;
        at<Dword>(node, 8) = 0;
    }
    at<Dword *>(node, 8) = pending::moved_nodes;

    at<void *>(node, 12) = pending::moved_sentinel;
    if (pending::moved_nodes)
        at<void *>(pending::moved_nodes, 12) = node;
    pending::moved_nodes = static_cast<Dword *>(node);
}

void update_track_car(void *binding) {
    void *track_body = at<void *>(binding, 80);
    if (!track_body)
        return;
    auto copy_position = [binding](void *body) {
        const float z = -at<float>(body, 688);
        const Dword x = at<Dword>(body, 680), y = at<Dword>(body, 684);
        at<float>(binding, 36) = z;
        at<Dword>(binding, 28) = x;
        at<Dword>(binding, 32) = y;
    };
    copy_position(track_body);
    read_track_rotation(at<void *>(binding, 80),
                        reinterpret_cast<float *>(bytes(binding) + 40));
    mark_track_vehicle_moved(binding);
    void *current = at<void *>(binding, 80);
    float distance = 300.0f;
    if (current) {
        float position[3] = {at<float>(binding, 28), at<float>(binding, 32),
                             -at<float>(binding, 36)};
        void *player = at<void *>(local_car(system(68), 0), 1768);
        void *descriptor = at<void *>(at<void *>(binding, 76), 32);
        descriptor = at<void *>(descriptor, 0);
        const auto format = kernel::numeric::current_format();
        distance = float(
            retained_point_distance(position, reinterpret_cast<float *>(bytes(player) + 680)) -
            (kernel::numeric::Extended(at<float>(descriptor, 68), format) + at<float>(player, 132)));
        if (split_screen()) {
            player = at<void *>(local_car(system(68), 1), 1768);
            const auto other =
                retained_point_distance(position, reinterpret_cast<float *>(bytes(player) + 680)) -
                (kernel::numeric::Extended(at<float>(descriptor, 68), format) + at<float>(player, 132));
            if (!(kernel::numeric::Extended(distance, format) < other))
                distance = static_cast<float>(other);
        }
    }
    const bool distant = at<Byte>(binding, 97) && distance >= 300.0f;
    if (distant) {
        if (!current)
            return;
        if (at<float>(current, 60) == 0)
            at<Byte>(binding, 116) = 1;
        copy_position(current);
        read_track_rotation(at<void *>(binding, 80), reinterpret_cast<float *>(bytes(binding) + 40));
    } else {
        bool stationary =
            (at<float>(track_body, 592) < 0.1500000059604644775390625f) &&
            (at<float>(track_body, 12) <= 0.3062500059604644775390625f);
        if (stationary)
            for (unsigned offset : {24, 20, 28})
                if (!(std::fabs(at<float>(track_body, offset)) < 0.05f)) {
                    stationary = false;
                    break;
                }
        if (at<std::int16_t>(track_body, 596) || !stationary || !at<Byte>(binding, 97) ||
            !(distance >= 20.0f) || !current)
            return;
        if (at<float>(current, 60) == 0)
            at<Byte>(binding, 116) = 1;
        copy_position(current);
        read_track_rotation(at<void *>(binding, 80),
                            reinterpret_cast<float *>(bytes(binding) + 40));
    }
    release_vehicle(current);
    at<void *>(binding, 80) = nullptr;
}
} // namespace nfs::game::runtime::physics
