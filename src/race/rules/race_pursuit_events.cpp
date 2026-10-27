#include "../session/race_runtime.hpp"
#include "nfs/core/timing/frame_clock.hpp"
#include "race_rules_x87.hpp"
namespace nfs::application::pending {
// The saved epilogue consumes all three event words.
void __fastcall pursuit_takedown(void *race, void *, Dword slot, Dword, Dword) {
    RaceParticipant *record = participant(race, slot);
    record_takedown(record);
    if (bonus_display_enabled(race))
        add_participant_reward(record, 4,
                               takedown_reward(bytes(race) + 164, at<int>(system(72), 192)));
    record->checkpoint_stamp = 0;
    if (game::runtime::ai::lifecycle::pending::participant_enabled(race, slot)) {
        set_participant_blocked(race, slot, 1);
        const Dword elapsed = race_rules_x87::truncate(
            static_cast<long double>(frame_clock::elapsed) * -60.0f);
        at<Dword>(race, 52 + slot * 4) = 240 - Dword(elapsed);
    }
}
} // namespace nfs::application::pending
