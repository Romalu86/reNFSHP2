#include "nfs/audio/backend/sound_ownership.hpp"
namespace nfs::sound::backend {
alignas(16) Byte configuration_bytes[360]{}, platform_bytes[88]{}, lifecycle_bytes[56]{},
    mixer_bytes[516]{};
alignas(4) Byte sample_bindings[256 * 24]{}, deferred_events[768]{};
alignas(4) Byte platform_tail_bytes[36]{};
int deferred_count{};
Dword &stream_limits = *reinterpret_cast<Dword *>(lifecycle_bytes + 44);
Dword *&stream_bindings = *reinterpret_cast<Dword **>(lifecycle_bytes + 48);
Dword *const stream_devices = reinterpret_cast<Dword *>(sample_bindings + 4);
void *&direct_sound = *reinterpret_cast<void **>(platform_bytes);
Byte &use_effect_interface = platform_bytes[25];
namespace pending {
Dword (&codec_constructors)[33] = *reinterpret_cast<Dword (*)[33]>(mixer_bytes + 24);
Dword (&codec_sizes)[33] = *reinterpret_cast<Dword (*)[33]>(mixer_bytes + 156);
Byte *const mix_format = mixer_bytes + 12;
Byte &mix_modes = mixer_bytes[5];
Dword *stream_nodes[32]{};
std::int8_t pan_table[1024]{};
} // namespace pending
} // namespace nfs::sound::backend
namespace nfs::sound {
BackendVoice *&backends = *reinterpret_cast<BackendVoice **>(backend::lifecycle_bytes + 40);
MixVoice *&mix_voices = *reinterpret_cast<MixVoice **>(backend::mixer_bytes + 504);
Dword &output_channels = *reinterpret_cast<Dword *>(backend::configuration_bytes + 72);
namespace pending {
extern std::uint16_t *const backend_formats =
    reinterpret_cast<std::uint16_t *>(backend::configuration_bytes + 80);
Dword &bank_limits = *reinterpret_cast<Dword *>(backend::configuration_bytes + 40);
Byte &allow_equal_priority = backend::configuration_bytes[60];
Byte &backend_count = backend::configuration_bytes[77];
Byte &reserved_voice_count = backend::configuration_bytes[46];
} // namespace pending
} // namespace nfs::sound
namespace nfs::game::runtime::race_audio::pending {
std::uint8_t &backend_stream_slot_count = sound::backend::configuration_bytes[71];
void *stream_slots[32]{};
} // namespace nfs::game::runtime::race_audio::pending
