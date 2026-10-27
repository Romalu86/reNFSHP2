#pragma once
#include "../../police/pursuit/chase_runtime.hpp"
#include "../../ai/driving/ai_car_behavior.hpp"
#include "../../graphics/cameras/camera_director_runtime.hpp"
#include "nfs/audio/vehicles/vehicle_audio.hpp"
namespace nfs::game::runtime::ai::cars {
Dword cop_unavailable(void *);
int idle_roadblock_cop_count(void *);
Byte free_spike_available(void *);
void play_unavailable_action(Dword);
void toggle_network_pursuit(void *);
Byte request_network_helicopter(void *);
Byte request_network_spikes(void *);
Byte request_network_backup(void *);
inline void *network_chase(void *entity) {
    return at<void *>(entities::interface_at(entity, 4), 8);
}
} // namespace nfs::game::runtime::ai::cars
