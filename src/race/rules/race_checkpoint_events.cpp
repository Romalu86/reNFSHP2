#include "../session/race_runtime.hpp"
namespace nfs::application::pending {
int update_checkpoint_order(void *, Dword, int, int);
void notify_finished_participant(void *, Dword);

void record_participant_checkpoint(void *record, int checkpoint, int position, Dword time) {
    at<int>(record, 12) = checkpoint;
    at<Dword>(record, 16) = Dword(checkpoint) + 1;
    if (checkpoint < 0)
        return;
    if (checkpoint > 0)
        network::session_active();
    at<Dword>(record, 56 + Dword(checkpoint) * 4) = time;
    if (checkpoint)
        time -= at<Dword>(record, 52 + Dword(checkpoint) * 4);
    const int best = at<int>(record, 8);
    if (best != 0 && best >= 0 && best < static_cast<int>(time))
        time = Dword(best);
    at<Dword>(record, 8) = time;
    at<int>(record, 456 + Dword(checkpoint) * 4) = position;
}

Dword checkpoint_reward(void *rewards, int difficulty) {
    if (at<int>(system(72), 180) < 1 && !at<Dword>(system(72), 176))
        return 0;
    if (difficulty < 0)
        return at<Dword>(rewards, 408);
    if (difficulty > 2)
        difficulty = 2;
    return at<Dword>(rewards, 408 + Dword(difficulty) * 4);
}

void __fastcall record_checkpoint(void *race, void *, Dword slot, int completed, Dword time) {
    int laps = at<int>(system(72), 228);
    void *tracks = system(40);
    void *track = at<void *>(tracks, 12 + at<Dword>(tracks, 8) * 4);
    if (!at<Byte>(track, 24)) {
        completed = 1;
        laps = 1;
    } else {
        if (completed < 1)
            return;
        if (completed > 98) {
            if (!network::session_active())
                invoke(race, 3, Dword(4));
            else if (completed > 100)
                completed = 100;
        }
    }
    const int checkpoint = static_cast<int>(Dword(completed) - 1);
    RaceParticipant *record = participant(race, slot);
    if (checkpoint >= 0 && record->checkpoint_positions[checkpoint] != -1)
        return;
    const Dword position = at<Dword>(race, 33896 + Dword(checkpoint) * 4) + 1;
    at<Dword>(race, 33896 + Dword(checkpoint) * 4) = position;
    record_participant_checkpoint(record, checkpoint, int(position), time);
    int order = int(position);
    if (network::session_active())
        order = update_checkpoint_order(
            race, slot, int(at<Dword>(race, 33896 + Dword(checkpoint) * 4)), checkpoint);
    at<int>(race, 24296 + (slot + Dword(checkpoint) * 24) * 4) = order;
    if (!at<Dword>(race, 33896 + Dword(checkpoint) * 4) && at<Dword>(system(72), 180) &&
        bonus_display_enabled(race))
        add_participant_reward(record, 1,
                               checkpoint_reward(bytes(race) + 164, at<int>(system(72), 192)));
    if (game::runtime::ai::lifecycle::pending::participant_enabled(race, slot)) {
        int best = at<int>(race, 34604);
        if (best < 1 || participant_checkpoint_duration(record, checkpoint) <= best)
            best = participant_checkpoint_duration(record, checkpoint);
        at<int>(race, 34604) = best;
    }
    if (completed == laps) {
        if (at<Dword>(system(108), 44))
            notify_finished_participant(system(108), slot);
        finish_participant(race, slot);
    }
    if (invoke<Byte>(race, 4))
        invoke(race, 3, Dword(4));
}

void __fastcall record_collision(void *race, void *, Dword attacker, Dword victim, Dword) {
    bool finish = false;
    if (static_cast<int>(victim) >= 0) {
        RaceParticipant *record = participant(race, victim);
        record_arrest(record);
        record->checkpoint_stamp = 0;
        if (at<int>(system(72), 2264) < static_cast<int>(record->arrests)) {
            record->disabled = 1;
            if (game::runtime::ai::lifecycle::pending::participant_enabled(race, victim)) {
                finish_participant(race, victim);
                if (!invoke<Byte>(race, 25, Byte(1)))
                    finish = true;
            }
        }
    }
    if (static_cast<int>(attacker) >= 0) {
        auto *record = participant(race, attacker);
        record_takedown(record);
        record->checkpoint_stamp = 0;
    }
    const Dword delay = at<int>(system(72), 176) > 1 ? 3 : 0;
    if (finish)
        invoke(race, 3, delay);
}

void __fastcall timed_checkpoint(void *race, void *, Dword slot, int checkpoint, Dword time) {
    record_checkpoint(race, nullptr, slot, checkpoint, time);
    finish_participant(race, slot);
}

void __fastcall time_trial_checkpoint(void *race, void *, Dword slot, int checkpoint, Dword time) {
    if (at<int>(race, 34628) < 1)
        at<Dword>(race, 34628) = game::race_ticks;
    record_checkpoint(race, nullptr, slot, checkpoint, time);
}

void __fastcall event_two_collision(void *race, void *, Dword attacker, Dword victim, Dword severity) {
    if (participant_blocked(race, victim))
        return;
    if (!at<Dword>(system(72), 140)) {
        extern void mark_event_arrest(void *, Dword);
        mark_event_arrest(at<void *>(race, 148), victim);
    }
    record_collision(race, nullptr, attacker, victim, severity);
}
} // namespace nfs::application::pending
