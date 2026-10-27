#include "../session/race_runtime.hpp"
namespace nfs::application::pending {
namespace {
void advance_quota_score(void *race, Dword slot) {
    ++at<Dword>(race, 34488 + slot * 4);
    const Dword remaining = at<Dword>(race, 34584) - 1;
    at<Dword>(race, 34584) = remaining;
    bool defer = false;
    if (static_cast<int>(remaining) >= 0) {
        void *rules = system(72);
        const Dword first = at<Dword>(race, 34488 + at<Dword>(rules, 204) * 4);
        const Dword second = at<Dword>(race, 34488 + at<Dword>(rules, 208) * 4);
        defer = static_cast<int>(first + remaining) > static_cast<int>(second) &&
                static_cast<int>(second + remaining) > static_cast<int>(first);
    }
    if (!defer) {
        at<Byte>(race, 34632) = 0;
        at<Dword>(race, 34628) = race_clock() + 240;
    }
    at<Byte>(race, 34634) = 1;
}
} // namespace

void __fastcall quota_arrest(void *race, void *, Dword attacker, Dword victim, Dword) {
    if (at<Dword>(system(72), 140) || participant_blocked(race, victim))
        return;
    if (static_cast<int>(victim) >= 0) {
        auto *record = participant(race, victim);
        record_arrest(record);
        record->checkpoint_stamp = 0;
    }
    if (static_cast<int>(attacker) >= 0) {
        auto *record = participant(race, attacker);
        record_takedown(record);
        record->checkpoint_stamp = 0;
    }
    set_participant_blocked(race, victim, 1);
    advance_quota_score(race, attacker);
}

void __fastcall quota_collision(void *race, void *, Dword victim, Dword attacker, Dword severity) {
    if (at<Dword>(system(72), 140) || participant_blocked(race, victim))
        return;
    RaceParticipant *record = participant(race, attacker);
    ++record->losses;
    if (bonus_display_enabled(race))
        add_participant_reward(
            record, 2,
            collision_reward(bytes(race) + 164, int(severity), at<int>(system(72), 192)));
    record->checkpoint_stamp = 0;
    set_participant_blocked(race, victim, 1);
    advance_quota_score(race, attacker);
}

void __fastcall tick_quota_event(void *race, void *) {
    if (!at<Byte>(race, 34634))
        return;
    const Dword remaining = at<Dword>(race, 34584);
    if (static_cast<int>(remaining) >= 0) {
        void *rules = system(72);
        const Dword first_slot = at<Dword>(rules, 204), second_slot = at<Dword>(rules, 208);
        const Dword first = at<Dword>(race, 34488 + first_slot * 4),
                   second = at<Dword>(race, 34488 + second_slot * 4);
        if (static_cast<int>(first + remaining) <= static_cast<int>(second) ||
            static_cast<int>(second + remaining) <= static_cast<int>(first)) {
            RaceParticipant *first_record = participant(race, first_slot);
            while (static_cast<int>(first_record->takedowns) < static_cast<int>(first))
                record_takedown(first_record);
            rules = system(72);
            const Dword slot = at<Dword>(rules, 208);
            const int score = at<int>(race, 34488 + slot * 4);
            RaceParticipant *second_record = participant(race, slot);
            Dword losses = second_record->losses;
            if (static_cast<int>(losses) < score) {
                do {
                    ++losses;
                } while (static_cast<int>(losses) < score);
                second_record->losses = losses;
            }
            invoke(race, 3, Dword(4));
            --at<Dword>(race, 34584);
        }
    }
    if (deadline_expired(race)) {
        invoke(race, 2);
        void *rules = system(72);
        const Dword first = at<Dword>(rules, 204), second = at<Dword>(rules, 208);
        at<volatile Dword>(race, 34584) = at<Dword>(rules, 232);
        at<volatile Dword>(race, 34628) = Dword(-1);
        at<volatile Byte>(race, 34632) = 0;
        at<volatile Byte>(race, 34634) = 0;
        at<volatile Dword>(race, 34488 + first * 4) = 0;
        at<volatile Dword>(race, 34488 + second * 4) = 0;
    }
    const int deadline = at<int>(race, 34628);
    if (deadline >= 0 && deadline <= static_cast<int>(race_clock()) && !at<Byte>(race, 34632)) {
        void *frontend = system(96);
        at<volatile Byte>(race, 34632) = 1;
        at<volatile Byte>(race, 34633) = 0;
        at<volatile Byte>(race, 34634) = 0;
        at<volatile Dword>(frontend, 36) = 17;
    }
    if (!at<Byte>(race, 34633)) {
        // These two raw score reads and writes are part of the native transition.
        void *rules = system(72);
        for (unsigned offset = 204; offset <= 208; offset += 4) {
            volatile Dword &score = at<volatile Dword>(race, 34488 + at<Dword>(rules, offset) * 4);
            const Dword value = score;
            score = value;
        }
        at<volatile Byte>(race, 34633) = 1;
        at<volatile Byte>(race, 34634) = 0;
    }
}
} // namespace nfs::application::pending
