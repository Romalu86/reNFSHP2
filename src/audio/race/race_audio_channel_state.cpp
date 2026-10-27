#include "nfs/physics/frame/simulation_runtime.hpp"
#include "nfs/audio/backend/sound_runtime.hpp"
#include "nfs/core/frame/game_step.hpp"
#include "nfs/graphics/resources/eagl_graphics.hpp"
#include "nfs/audio/music/music_clients.hpp"
namespace nfs::game::runtime::race_audio {
// The saved (id & 255) != -1 test is always true.
void stop_event(Dword id) {
    if (!network::graphics_enabled)
        return;
    const Dword index = (((id >> 8) & 255) * 124 + (id & 255)) * 5;
    if (pending::race_audio_event_channels[index] == Dword(-1))
        return;
    sound::lock();
    sound::fade_voice_volume(pending::race_audio_event_channels[index], 5, -1);
    pending::race_audio_event_channels[index] = pending::race_audio_event_channels[index + 1] =
        Dword(-1);
    sound::unlock();
}

void retire_unused_channels() {
    const int listener = pending::listener_index;
    Dword used[12]{};
    const int count = at<int>(pending::listener_state, 384 + listener * 2760);
    for (int i = 0; i < count; ++i) {
        const Dword channel = at<Dword>(pending::listener_state,
                                      2152 + pending::listener_index * 2760 + i * 56);
        for (unsigned slot = 0; slot < 12; ++slot)
            if (channel == at<Dword>(pending::listener_state,
                                    388 + pending::listener_index * 2760 + slot * 4))
                used[slot] = 1;
    }
    for (unsigned slot = 0; slot < 12; ++slot)
        if (!used[slot])
            at<Dword>(pending::listener_state, 388 + listener * 2760 + slot * 4) =
                Dword(-1);
}

void play_due_events() {
    if (!pending::delayed_count)
        return;
    for (unsigned offset = 0; offset < 640; offset += 32) {
        Byte *entry = pending::delayed_events + offset;
        if (at<Dword>(entry, 0) == Dword(-1) ||
            static_cast<int>(graphics::unresolved::camera_state) < at<int>(entry, 4))
            continue;
        Dword pan = at<Dword>(entry, 16);
        if (pan == Dword(-1))
            pan = pending::spatial_pan(pending::listener_state, entry + 20);
        pending::play_event(113, at<Dword>(entry, 0), at<Dword>(entry, 8),
                                     at<Dword>(entry, 12), pan, 0);
        at<Dword>(entry, 0) = Dword(-1);
        --pending::delayed_count;
    }
}
} // namespace nfs::game::runtime::race_audio
