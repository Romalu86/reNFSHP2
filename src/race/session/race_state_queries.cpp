#include "race_runtime.hpp"
#include "nfs/ai/lifecycle/ai_lifecycle.hpp"
namespace nfs::application::pending {
// Callers consume AL only.
Byte bonus_display_enabled(void *race) {
    if (!game::runtime::physics::split_screen() && at<Dword>(system(72), 148) != 5 &&
        !network::session_active())
        return invoke<Byte>(race, 35);
    return 0;
}
// Only the AL result is part of the predicate contract.
Byte deadline_expired(void *race) {
    const int deadline = at<int>(race, 34596);
    return deadline >= 0 && deadline <= static_cast<int>(race_clock());
}

void __fastcall tick_race_deadline(void *race, void *) {
    if (deadline_expired(race))
        invoke(race, 2);
}

Byte __fastcall event_complete(void *race, void *) {
    if (network::session_active())
        return !invoke<Byte>(race, 25, Byte(0));
    for (int i = 0; i < at<int>(system(72), 168); ++i)
        if (participant(race, Dword(i))->completed_checkpoints > 100)
            return 1;
    return !invoke<Byte>(race, 25, Byte(0));
}

Byte any_active_participant(void *race) {
    return invoke<Byte>(race, 25, Byte(0));
}

Byte lap_capacity_exceeded(void *race) {
    for (int i = 0; i < at<int>(system(72), 168); ++i)
        if (participant(race, Dword(i))->completed_checkpoints > 100)
            return 1;
    return 0;
}

Byte __fastcall participant_last_position(void *race, void *, Dword slot, int checkpoint) {
    int position =
        checkpoint < 0 ? -1 : at<int>(race, 1040 + (slot * 247 + Dword(checkpoint)) * 4);
    return position ==
           static_cast<int>(at<Dword>(system(72), 168) - at<Dword>(system(72), 172) - 1);
}

Byte __fastcall participant_can_continue(void *race, void *, Dword slot) {
    RaceParticipant *record = participant(race, slot);
    if (record->retired || record->disabled || record->retirement_tick >= 0 ||
        participant_has_finished(record) || record->disconnected)
        return 0;
    return static_cast<int>(Dword(record->last_checkpoint) + 1) < at<int>(system(72), 228);
}

Byte __fastcall race_uses_rewards(void *race, void *) {
    return at<Byte>(race, 34593);
}

Dword __fastcall elapsed_race_ticks(void *, void *) {
    return game::race_ticks;
}

int __fastcall participant_elapsed_time(void *race, void *, Dword slot) {
    const int stored = participant(race, slot)->finish_time;
    if (stored >= 0)
        return stored;
    if (invoke<int>(race, 22))
        return invoke<int>(race, 22);
    return 0;
}

int __fastcall participant_current_split(void *race, void *, Dword slot) {
    RaceParticipant *record = participant(race, slot);
    int finish = record->finish_time;
    if (finish >= 0) {
        if (record->last_checkpoint > 0)
            finish = participant_checkpoint_duration(record, record->last_checkpoint);
        return finish;
    }
    if (record->completed_checkpoints > 0) {
        const Dword checkpoint = Dword(record->completed_checkpoints) - 1;
        if (static_cast<int>(checkpoint) < 0)
            return static_cast<int>(Dword(invoke<int>(race, 22)) + 1);
        const Dword previous = at<Dword>(record, 56 + checkpoint * 4);
        return static_cast<int>(Dword(invoke<int>(race, 22)) - previous);
    }
    return invoke<int>(race, 22);
}

int __fastcall participant_remaining_time(void *race, void *, Dword slot) {
    if (participant(race, slot)->finish_time < 0) {
        void *rules = system(72);
        const Dword elapsed = Dword(invoke<int>(race, 22));
        return static_cast<int>(at<Dword>(rules, 164) - elapsed);
    }
    return 0;
}

Byte __fastcall participant_reached_final_lap(void *race, void *, Dword slot) {
    const Dword configured = at<Dword>(system(72), 228);
    const int last = configured ? static_cast<int>(configured - 1) : 0;
    if (!last || last <= participant(race, slot)->completed_checkpoints)
        return 1;
    void *tracks = system(40);
    void *track = at<void *>(tracks, 12 + at<Dword>(tracks, 8) * 4);
    return !at<Byte>(track, 24);
}

Byte __fastcall race_has_active_player(void *race, void *, Byte local_only) {
    void *rules = system(72);
    const Dword configured = at<Dword>(rules, 228);
    const Dword checkpoint = configured ? configured - 1 : 0;
    for (int index = 0; index < at<int>(rules, 168); ++index) {
        const Dword slot = Dword(index);
        RaceParticipant *record = participant(race, slot);
        if (!game::runtime::ai::lifecycle::pending::participant_enabled(race, slot))
            continue;
        void *entry = slot < 24 ? bytes(rules) + 248 + slot * 84 : nullptr;
        if (!participant_is_cop(race, slot) && (!local_only || !at<Dword>(entry, 4)) &&
            !record->disabled && !record->disconnected &&
            at<int>(race, 24296 + (slot + checkpoint * 24) * 4) < 0)
            return 1;
    }
    return 0;
}
// The code copies and destroys each participant before counting.
int __fastcall count_remaining_racers(void *race, void *) {
    int count = 0;
    void *rules = system(72);
    for (int i = 0; i < at<int>(rules, 168); ++i) {
        RaceParticipant *source = participant(race, Dword(i));
        if (source->kind == 3 || source->kind == 2)
            continue;
        alignas(4) RaceParticipant copy;
        network::world::copy_race_player(&copy, source);
        if (!copy.disconnected)
            ++count;
        network::world::destroy_race_player(&copy);
    }
    return count;
}

int __fastcall count_local_racers(void *race, void *) {
    int count = 0;
    if (network::session_active()) {
        void *rules = system(72);
        for (int i = 0; i < at<int>(rules, 168); ++i) {
            RaceParticipant *source = participant(race, Dword(i));
            if (source->kind == 3 || source->kind == 2 || source->disabled || source->disconnected)
                continue;
            alignas(4) RaceParticipant copy;
            network::world::copy_race_player(&copy, source);
            if (!copy.retired && !participant_has_finished(&copy) && copy.retirement_tick < 0)
                ++count;
            network::world::destroy_race_player(&copy);
        }
        return count;
    }
    for (unsigned i = 0; i < 2; ++i) {
        const Dword slot = at<Dword>(system(72), 204 + i * 4);
        if (slot != Dword(-1) && !participant_is_cop(race, slot))
            ++count;
    }
    return static_cast<int>(at<Dword>(system(72), 180) + Dword(count));
}

int __fastcall participant_position(void *race, void *, Dword slot) {
    RaceParticipant *record = participant(race, slot);
    if (record->finish_time < 0 ||
        (network::session_active() && at<std::int16_t>(system(104), 8) != 6)) {
        if (network::session_active() && record->disconnected)
            return 0;
        if (invoke<Byte>(race, 29, slot))
            return count_racers_ahead(race, int(slot)) + 1;
    }
    if (at<Dword>(race, 34620) == 1 && participant_has_finished(record)) {
        if (network::session_active())
            return static_cast<int>(Dword(invoke<int>(race, 24)) - Dword(record->finish_position) +
                                    1);
        return static_cast<int>(at<Dword>(system(72), 180) + at<Dword>(system(72), 176) -
                                Dword(record->finish_position) + 1);
    }
    if (record->last_checkpoint < 0)
        return 0;
    return static_cast<int>(Dword(record->checkpoint_positions[record->last_checkpoint]) + 1);
}
} // namespace nfs::application::pending
