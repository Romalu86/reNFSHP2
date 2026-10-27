#include "chase_runtime.hpp"
namespace nfs::game::runtime::ai::lifecycle::pending {

void *reserve_spike_entity(void *owner) {
    for (unsigned index = 0; index != 2; ++index) {
        void *spike = at<void *>(owner, index * 4);
        if (!spike || at<Byte>(spike, 232))
            continue;
        spike = at<void *>(owner, index * 4);
        if (at<Byte>(spike, 232))
            return spike;
        at<Byte>(spike, 232) = 1;
        invoke(spike, 6, 1);
        if (entity_body(spike))
            at<Byte>(entity_body(spike), 729) = 1;
        entities::update_spike_corners(spike);
        return spike;
    }
    return nullptr;
}

void position_roadblock_spikes(void *spike, const float *position, const float *rotation) {
    if (void *body = entity_body(spike)) {
        std::memcpy(bytes(body) + 680, position, 12);
        std::memcpy(bytes(body) + 692, rotation, 36);
        matrices::transpose(reinterpret_cast<float *>(bytes(body) + 692),
                            reinterpret_cast<float *>(bytes(body) + 72));
        at<Byte>(spike, 233) = 1;
        physics::clear_ground_contact(bytes(body) + 508);
        if (!entities::settle_spike_body(spike)) {
            entities::update_spike_corners(spike);
            void *vehicle = at<void *>(spike, 228);
            synchronize_track_vehicle(vehicle);
            physics::mark_track_vehicle_moved(vehicle);
        }
    }
}
} // namespace nfs::game::runtime::ai::lifecycle::pending
