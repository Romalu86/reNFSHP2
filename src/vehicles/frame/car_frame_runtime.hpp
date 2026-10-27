#pragma once
#include "../../physics/vehicles/car_physics_frame.hpp"
#include "../../input/vehicles/car_input_events.hpp"
#include "nfs/ai/lifecycle/ai_dependencies.hpp"
#include "nfs/network/session/network_runtime.hpp"
#include "nfs/graphics/resources/eagl_graphics.hpp"
#include "nfs/core/lifecycle/system_runtime.hpp"
namespace nfs::game::runtime::ai::entities {
void update_entity_path_sample(void *);
}
namespace nfs::game::runtime::ai::cars {
void dispatch_physical_car_frame(void *);
void update_car_siren_control(void *, Dword *);
void submit_nearby_audio_car(void *, Dword);
Byte participant_inactive_racer(void *);
Byte sample_race_progress(void *, void *, int, const float *, float *);
void update_racer_progress(void *);
void send_car_lap_progress(void *, Dword, Dword, Dword);
inline bool track_is_circuit() {
    void *catalogue = system(40);
    return at<Byte>(at<void *>(catalogue, 12 + at<Dword>(catalogue, 8) * 4), 24) != 0;
}
} // namespace nfs::game::runtime::ai::cars
