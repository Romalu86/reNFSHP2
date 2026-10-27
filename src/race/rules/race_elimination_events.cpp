#include "../session/race_runtime.hpp"
#include "nfs/audio/vehicles/vehicle_audio.hpp"
namespace nfs::application::pending {
int count_elimination_candidates(void *, Dword *);
void notify_finished_participant(void *, Dword);
void notify_eliminated_participant(void *, Dword);
void mark_ai_retired(void *, Dword);

int __fastcall elimination_remaining_count(void *race, void *) {
    void *rules = system(72);
    int count = 0;
    for (int i = 0; i < at<int>(rules, 168); ++i) {
        if (participant_is_cop(race, Dword(i)))
            continue;
        alignas(4) RaceParticipant copy;
        network::world::copy_race_player(&copy, participant(race, Dword(i)));
        if (!copy.disconnected || participant_has_finished(&copy))
            ++count;
        network::world::destroy_race_player(&copy);
    }
    return count;
}

Byte __fastcall elimination_has_active_player(void *race, void *, Byte local_only) {
    void *rules = system(72);
    Dword lap = at<Dword>(rules, 228);
    if (lap)
        --lap;
    if (network::session_active())
        lap = 100;
    for (int index = 0; index < at<int>(rules, 168); ++index) {
        const Dword slot = Dword(index);
        if (!game::runtime::ai::lifecycle::pending::participant_enabled(race, slot) ||
            participant_is_cop(race, slot))
            continue;
        void *entry = slot < 24 ? bytes(rules) + 248 + slot * 84 : nullptr;
        if (local_only && at<Dword>(entry, 4))
            continue;
        if (participant_blocked(race, slot) || participant_disconnected(race, slot))
            continue;
        RaceParticipant *record = participant(race, slot);
        if (at<int>(race, 24296 + (slot + lap * 24) * 4) < 0 &&
            !participant_has_finished(record) && record->retirement_tick < 0)
            return 1;
    }
    return 0;
}
// Network branch accepts a clear disconnected byte.
Byte eliminate_last_checkpoint_participant(void *race, int checkpoint) {
    Dword candidate = 0;
    bool found = false;
    for (int i = 0; i < at<int>(system(72), 168); ++i) {
        const Dword slot = Dword(i);
        if (participant_is_cop(race, slot))
            continue;
        RaceParticipant *record = participant(race, slot);
        if (network::session_active()) {
            if (record->retired || record->disabled || record->retirement_tick >= 0 ||
                participant_has_finished(record) || record->disconnected)
                continue;
        } else if (!invoke<Byte>(race, 29, slot))
            continue;
        if (!participant_completed_checkpoint(record, checkpoint)) {
            candidate = slot;
            found = true;
            break;
        }
        const int position = checkpoint < 0 ? -1 : record->checkpoint_positions[checkpoint];
        if (position == at<int>(race, 33896 + Dword(checkpoint) * 4)) {
            candidate = slot;
            found = true;
        }
    }
    if (found) {
        invoke(race, 27, candidate);
        notify_eliminated_participant(system(108), candidate);
    }
    return Byte(found);
}

void __fastcall tick_elimination_event(void *race, void *) {
    if (network::session_active() && at<Dword>(system(108), 44) && at<int>(race, 34636) > 0 &&
        at<int>(race, 34640) > 0) {
        auto *signal = reinterpret_cast<kernel::Signal *>(bytes(race) + 156);
        if (kernel::signal_ready(signal)) {
            if (at<int>(race, 34632) <= 0) {
                Byte eliminated;
                do {
                    eliminated = eliminate_last_checkpoint_participant(
                        race, static_cast<int>(at<Dword>(race, 34640) - 1));
                    if (eliminated)
                        --at<Dword>(race, 34636);
                } while (at<Dword>(race, 34636) && eliminated);
                at<Dword>(race, 34632) = 0;
            } else
                --at<Dword>(race, 34632);
            kernel::signal_set(signal);
        }
    }
    if (deadline_expired(race))
        invoke(race, 2);
}

void __fastcall elimination_checkpoint(void *race, void *, Dword slot, int completed, Dword time) {
    record_checkpoint(race, nullptr, slot, completed, time);
    if (completed <= 0)
        return;
    const int checkpoint = static_cast<int>(Dword(completed) - 1);
    if (!network::session_active()) {
        if (!invoke<Byte>(race, 26, slot, checkpoint))
            return;
        RaceParticipant *source = participant(race, slot);
        for (int i = 0; i < at<int>(system(72), 168); ++i) {
            if (Dword(i) == slot || participant_is_cop(race, Dword(i)) ||
                participant(race, Dword(i))->finish_position != -1 ||
                participant_completed_checkpoint(participant(race, Dword(i)),
                                                 source->last_checkpoint))
                continue;
            invoke(race, 27, Dword(i));
            if (Dword(completed) == at<Dword>(system(72), 180) - 1 + at<Dword>(system(72), 176))
                invoke(race, 3, Dword(4));
            return;
        }
        return;
    }
    Dword locals = 0;
    const int candidates = count_elimination_candidates(race, &locals);
    const Dword delta = Dword(completed) - at<Dword>(race, 34640);
    Dword completed_count = 0;
    for (int i = 0; i < at<int>(system(72), 168); ++i)
        if (!participant_is_cop(race, Dword(i)) &&
            participant_completed_checkpoint(participant(race, Dword(i)), checkpoint))
            ++completed_count;
    if (static_cast<int>(delta) > 0 &&
        static_cast<int>(Dword(candidates) - delta) <=
            static_cast<int>(completed_count + at<Dword>(race, 34636))) {
        at<Dword>(race, 34632) = 600;
        at<Dword>(race, 34636) += delta;
        at<int>(race, 34640) = completed;
    }
    if (at<Dword>(race, 34636) && candidates <= static_cast<int>(completed_count))
        at<Dword>(race, 34632) = 0;
    if (static_cast<int>(Dword(candidates) - delta) < 1) {
        finish_participant(race, slot);
        if (at<Dword>(system(108), 44))
            notify_finished_participant(system(108), slot);
    }
}

void __fastcall eliminate_participant(void *race, void *, Dword slot) {
    RaceParticipant *record = participant(race, slot);
    if (participant_has_finished(record))
        return;
    const Dword position = at<Dword>(race, 34628) + 1;
    record->finish_position = int(position);
    at<Dword>(race, 34628) = position;
    mark_ai_retired(race, slot);
    finish_participant(race, slot);
    void *entry = slot < 24 ? bytes(system(72)) + 248 + slot * 84 : nullptr;
    if (game::runtime::ai::lifecycle::pending::participant_enabled(race, slot) &&
        (!at<Dword>(entry, 4) || (!network::session_active() && at<Dword>(system(72), 176) == 2)))
        game::audio::vehicle::request_lap_announcement(system(84), 2);
    if (invoke<Byte>(race, 4)) {
        const Dword delay = at<int>(system(72), 176) > 1 ? 3 : 0;
        if (static_cast<int>(record->arrests) > 0) {
            invoke(race, 3, delay);
            invoke(race, 3, delay);
        } else
            invoke(race, 3, Dword(3));
    }
}
} // namespace nfs::application::pending
