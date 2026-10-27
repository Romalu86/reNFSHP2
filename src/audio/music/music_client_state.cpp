#include "nfs/audio/music/music_clients.hpp"
#include "nfs/physics/frame/simulation_runtime.hpp"
#include "nfs/physics/frame/dynamics_runtime.hpp"
namespace nfs::game::audio::state {
#include "music_client_data.inc"
Dword waveform_buffer{};
// Audio launch flags alias the shared Options object.
} // namespace nfs::game::audio::state
namespace nfs::network {
Dword &graphics_enabled = *reinterpret_cast<Dword *>(game::audio::state::client_configuration + 4);
}
namespace nfs::game::runtime::race_audio::pending {
Dword &race_audio_muted = *reinterpret_cast<Dword *>(audio::state::client_configuration + 12);
Dword &music_mode = *reinterpret_cast<Dword *>(audio::state::client_options + 32);
Dword &music_level = *reinterpret_cast<Dword *>(audio::state::client_configuration + 1612);
Dword &music_handle = *reinterpret_cast<Dword *>(audio::state::music_library + 40056);
Dword &car_channel = *reinterpret_cast<Dword *>(audio::state::client_options + 164);
Dword *const race_audio_event_channels = reinterpret_cast<Dword *>(audio::state::event_channels);
Byte *const bank_bindings = audio::state::bank_storage;
Dword &variation_index = *reinterpret_cast<Dword *>(audio::state::effects_storage + 40);
Byte *const event_flags = audio::state::audio_world;
Byte *const listener_state = audio::state::audio_world + 248;
Byte *const delayed_events = audio::state::audio_world + 6208;
Dword &delayed_count = *reinterpret_cast<Dword *>(audio::state::audio_world + 6848);
int &listener_index = *reinterpret_cast<int *>(audio::state::audio_world + 6852);
int race_audio_listener_count = 1;  
Dword *listener_configuration =
    reinterpret_cast<Dword *>(audio::state::audio_world + 6856);
float (&race_audio_listener_velocity)[6] = *reinterpret_cast<float (*)[6]>(audio::state::audio_world +
                                                                         6856);
float (&previous_listener)[3] = *reinterpret_cast<float (*)[3]>(audio::state::track_data +
                                                                         12184);
Byte *const engine_state = audio::state::engine_storage;
Byte &engines_disabled = audio::state::engine_storage[252];
Dword &emitter_serial = *reinterpret_cast<Dword *>(audio::state::audio_world + 6888);
int &sound_definition_count = *reinterpret_cast<int *>(audio::state::track_data + 92);
Byte *const sound_definitions = audio::state::track_data + 9300;
} // namespace nfs::game::runtime::race_audio::pending
