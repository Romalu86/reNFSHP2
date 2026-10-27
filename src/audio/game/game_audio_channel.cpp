#include "nfs/audio/music/music_clients.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
namespace nfs::game::runtime::race_audio::pending {
// Keep the shared two-listener channel registry and its saved allocation order.
void update_emitter_channel(void *emitter) {
    using namespace game::audio;
    const Dword event = at<Dword>(emitter, 40);
    Dword pan = spatial_pan(listener_state, bytes(emitter) + 8);
    const int listener = listener_index;
    Dword flags = at<Dword>(emitter, 36);
    if (flags & 1024)
        pan = 0;
    if (race_audio_listener_count == 2)
        pan = listener == 0 ? 49152 : 16383;
    const Dword radius = at<Dword>(emitter, 44);
    Dword listener_channel = listener ? 256 : 0;
    int start = (flags & 16) ? 82 : 94, end = start + 12, selected = 0;
    bool found = false;
    if (flags == 258)
        selected = 107;
    else {
        for (int slot = 0; slot < 12; ++slot) {
            int &serial = at<int>(listener_state,
                                  388 + listener_index * 2760 + slot * 4);
            const Dword *events =
                race_audio_event_channels + listener_index * 620 + 94 * 5 + 1 + slot * 5;
            if (events[-60] == Dword(-1) && events[0] == Dword(-1))
                serial = -1;
            if (serial == at<int>(emitter, 32)) {
                unsigned history = 436 + (listener * 230 + slot) * 12;
                for (unsigned axis = 0; axis < 3; ++axis)
                    at<volatile float>(emitter, 20 + axis * 4) =
                        float(static_cast<long double>(at<float>(emitter, 8 + axis * 4)) -
                              at<float>(listener_state, history + axis * 4));
                for (unsigned axis = 0; axis < 3; ++axis)
                    at<Dword>(listener_state, history + axis * 4) =
                        at<Dword>(emitter, 8 + axis * 4);
                selected = start + slot;
                found = true;
                break;
            }
        }
        if (!found)
            for (int slot = 0; start + slot < end; ++slot) {
                if (at<int>(listener_state, 388 + listener * 2760 + slot * 4) == -1) {
                    at<Dword>(listener_state, 388 + listener * 2760 + slot * 4) =
                        at<Dword>(emitter, 32);
                    unsigned history = 436 + (listener * 230 + slot) * 12;
                    for (unsigned axis = 0; axis < 3; ++axis) {
                        at<Dword>(listener_state, history + axis * 4) =
                            at<Dword>(emitter, 8 + axis * 4);
                        at<Dword>(emitter, 20 + axis * 4) = 0;
                    }
                    selected = start + slot;
                    break;
                }
            }
    }
    int pitch = doppler(reinterpret_cast<float *>(bytes(emitter) + 8),
                        reinterpret_cast<float *>(bytes(emitter) + 20),
                        reinterpret_cast<float *>(listener_configuration));
    if (pitch >= 0) {
        flags = at<Dword>(emitter, 36);
        submit_distanced(listener_channel | Dword(selected), event, 64,
                         Dword(pitch), at<Dword>(emitter, 4), pan, radius * radius,
                         at<Dword>(emitter, 48), at<Dword>(emitter, 52),
                         (flags >> 11) & 4294967041u);
        event_flags[selected + listener_index * 124] = 1;
    }
}
} // namespace nfs::game::runtime::race_audio::pending
