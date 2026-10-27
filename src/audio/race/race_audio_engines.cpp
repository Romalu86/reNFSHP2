#include "nfs/physics/solver/solver_runtime.hpp"
namespace nfs::game::runtime::race_audio {
// Complete shipped body is RET; arguments are caller-cleaned.
void debug_sound(int, int, const char *, ...) {
}

Dword engine_active(int slot, int listener) {
    return at<Dword>(pending::engine_state, listener * 120 + slot * 4);
}

void stop_engine(int slot, int listener) {
    Byte *state = pending::engine_state + listener * 120 + slot * 4;
    at<Dword>(state, 0) = at<Dword>(state, 20) = at<Dword>(state, 100) = 0;
    at<Dword>(state, 40) = 240;
    at<Dword>(state, 60) = 120;
    stop_event((slot == 4 ? 2 : Dword(slot) + 78) | (listener ? 256 : 0));
    debug_sound(slot + 6, 1, "%d off", slot);
}
// Slot-major order is observable through audio calls.
void pending::reset_engine_events() {
    for (int slot = 0; slot < 5; ++slot)
        for (int listener = 0; listener < 2; ++listener)
            if (engine_active(slot, listener))
                stop_engine(slot, listener);
}

void start_engine(int slot, int force, int listener) {
    using namespace pending;
    Byte *state = engine_state + listener * 120 + slot * 4;
    if (at<Dword>(state, 0) || engines_disabled)
        return;
    at<Dword>(state, 0) = 1;
    Dword event;
    if (force) {
        at<Dword>(state, 20) = 1;
        at<Dword>(state, 60) = Dword(-1);
        event = 9;
    } else if (emitter_random_word() % 100 > 50) {
        at<Dword>(state, 20) = 1;
        at<Dword>(state, 60) = emitter_random_word() % 60 + 120;
        event = 1;
    } else {
        at<Dword>(state, 20) = 0;
        at<Dword>(state, 40) = emitter_random_word() % 120 + 240;
        event = 0;
    }
    Dword pan = Dword(listener);
    if (at<Dword>(system(72), 176) == 2)
        pan = listener ? 16383 : 49152;
    play_event((slot == 4 ? 2 : Dword(slot) + 78) | (listener ? 256 : 0), event, 64,
                        0, pan, slot == 4);
    at<Dword>(state, 80) = event;
}
// Integer gain multiplication wraps before signed division.
void advance_engine(int slot, Dword gain, Dword fixed_volume, Dword pan, int listener) {
    using namespace pending;
    Byte *state = engine_state + listener * 120 + slot * 4;
    if (!at<Dword>(state, 0) || engines_disabled)
        return;
    const bool sounding = at<Dword>(state, 20) != 0;
    Dword timer = ++at<Dword>(state, 100);
    if (timer == at<Dword>(state, sounding ? 60 : 40)) {
        Dword event = sounding ? 0 : 1;
        at<Dword>(state, 20) = event;
        at<Dword>(state, 100) = 0;
        Dword channel = (slot == 4 ? 2 : Dword(slot) + 78) | (listener ? 256 : 0);
        stop_event(channel);
        if (sounding)
            at<Dword>(state, 40) = emitter_random_word() % 120 + 240;
        else
            at<Dword>(state, 60) = emitter_random_word() % 60 + 120;
        Dword transition_pan = gain;
        if (at<Dword>(system(72), 176) == 2)
            transition_pan = listener ? 16383 : 49152;
        play_event(channel, event, 64, 0, transition_pan, slot == 4);
        at<Dword>(state, 80) = event;
    }
    int volume = static_cast<int>(fixed_volume * 64) / 65536;
    if (volume < 0)
        volume = 0;
    else if (volume > 127)
        volume = 127;
    if (at<Dword>(system(72), 176) == 2)
        pan = listener ? 16383 : 49152;
    play_event((slot == 4 ? 2 : Dword(slot) + 78) | (listener ? 256 : 0),
                        at<Dword>(state, 80), Dword(volume), gain, pan, slot == 4);
    debug_sound(slot + 6, 1, "%d: azi %d, vol %d", slot, pan, gain);
}
} // namespace nfs::game::runtime::race_audio
