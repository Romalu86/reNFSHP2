#include "nfs/audio/vehicles/vehicle_audio.hpp"
namespace nfs::game::audio::vehicle {
#include "vehicle_audio_data.inc"
Progress *progress_head = nullptr;
} // namespace nfs::game::audio::vehicle
namespace nfs::game::runtime::race_audio::pending {
Dword *(&listener_objects)[2] =
    *reinterpret_cast<Dword *(*)[2]>(audio::vehicle::engine_owners + 808);
}
