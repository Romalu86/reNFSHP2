#include "race_runtime.hpp"
#include "nfs/ai/lifecycle/ai_lifecycle.hpp"
namespace nfs::application::pending {
namespace {
void tick_timed_mode(void *race) {
    if (deadline_expired(race))
        invoke(race, 2);
    const int limit = at<int>(system(72), 164);
    if (limit < invoke<int>(race, 22))
        invoke(race, 3, Dword(4));
}
} // namespace

void __fastcall tick_event_eleven(void *race, void *) {
    tick_timed_mode(race);
}

void __fastcall tick_event_twelve(void *race, void *) {
    tick_timed_mode(race);
}

void __fastcall tick_event_thirteen(void *race, void *) {
    tick_timed_mode(race);
}

void __fastcall tick_paired_event(void *race, void *) {
    tick_timed_mode(race);
}

void __fastcall tick_event_five(void *race, void *) {
    if (deadline_expired(race))
        invoke(race, 2);
}

void __fastcall tick_event_nine(void *race, void *) {
    if (deadline_expired(race))
        invoke(race, 2);
}

void __fastcall tick_time_trial(void *race, void *) {
    if (deadline_expired(race))
        invoke(race, 2);
}

void __fastcall tick_pursuit_event(void *race, void *) {
    tick_timed_mode(race);
    for (Dword slot = 0; slot < 24; ++slot)
        if (game::runtime::ai::lifecycle::pending::participant_enabled(race, slot) &&
            participant_blocked(race, slot) &&
            static_cast<int>(at<Dword>(race, 52 + slot * 4) - race_clock()) < 0)
            set_participant_blocked(race, slot, 0);
}

void __fastcall event_twelve_arrest(void *race, void *, Dword, Dword victim, Dword) {
    RaceParticipant *record = participant(race, victim);
    record_arrest(record);
    record->checkpoint_stamp = 0;
}

void __fastcall paired_event_takedown(void *race, void *, Dword slot, Dword, Dword) {
    record_takedown(participant(race, slot));
    invoke(race, 3, Dword(4));
}

void __fastcall event_eleven_collision(void *race, void *, Dword attacker, Dword victim, Dword) {
    if (participant_is_cop(race, attacker) &&
        game::runtime::ai::lifecycle::pending::participant_enabled(race, attacker)) {
        RaceParticipant *record = participant(race, attacker);
        record_takedown(record);
        record->checkpoint_stamp = 0;
    }
    if (game::runtime::ai::lifecycle::pending::participant_enabled(race, victim)) {
        RaceParticipant *record = participant(race, victim);
        record_arrest(record);
        record->checkpoint_stamp = 0;
    }
}

void __fastcall elimination_arrest(void *race, void *, Dword, Dword victim, Dword) {
    RaceParticipant *record = participant(race, victim);
    record_arrest(record);
    record->checkpoint_stamp = 0;
    invoke(race, 27, victim);
}

void __fastcall race_collision_score(void *race, void *, Dword, Dword victim, Dword severity) {
    RaceParticipant *record = participant(race, victim);
    ++record->losses;
    if (bonus_display_enabled(race)) {
        const Dword reward =
            collision_reward(bytes(race) + 164, int(severity), at<int>(system(72), 192));
        add_participant_reward(record, 2, reward);
    }
    record->checkpoint_stamp = 0;
}

void __fastcall event_thirteen_takedown(void *race, void *, Dword slot, Dword, Dword) {
    RaceParticipant *record = participant(race, slot);
    record_takedown(record);
    if (bonus_display_enabled(race))
        add_participant_reward(record, 4,
                               takedown_reward(bytes(race) + 164, at<int>(system(72), 192)));
    record->checkpoint_stamp = 0;
}

void __fastcall event_zero_arrest(void *race, void *, Dword, Dword victim, Dword) {
    if (static_cast<int>(victim) < 0)
        return;
    set_participant_blocked(race, victim, 1);
    RaceParticipant *record = participant(race, victim);
    record_arrest(record);
    record->checkpoint_stamp = 0;
    if (game::runtime::ai::lifecycle::pending::participant_enabled(race, victim))
        finish_participant(race, victim);
    if (invoke<Byte>(race, 4))
        invoke(race, 3, at<int>(system(72), 176) < 2 ? Dword(0) : Dword(3));
}
} // namespace nfs::application::pending
