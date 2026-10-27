#include "nfs/physics/frame/simulation_runtime.hpp"
#include "nfs/audio/backend/sound_runtime.hpp"
namespace nfs::game::runtime::race_audio {

void *stream(Dword handle) {
    const int index = static_cast<int>(handle);
    return index >= 0 && index < pending::backend_stream_slot_count
               ? pending::stream_slots[index]
               : nullptr;
}
// Stream handles and low-level voice handles are distinct.
int set_stream_volume(Dword handle, Dword value) {
    if (!sound::control.initialized)
        return -10;
    void *entry = stream(handle);
    if (!entry)
        return -8;
    at<Byte>(entry, 212) = Byte(value);
    sound::set_voice_volume(at<Dword>(entry, 4), static_cast<int>(value));
    return 0;
}

int fade_stream_volume(Dword handle, int milliseconds, Dword value) {
    if (!sound::control.initialized)
        return -10;
    void *entry = stream(handle);
    if (!entry)
        return -8;
    at<Byte>(entry, 212) = Byte(value);
    sound::fade_voice_volume(at<Dword>(entry, 4), milliseconds / 10, static_cast<int>(value));
    return 0;
}
} // namespace nfs::game::runtime::race_audio
