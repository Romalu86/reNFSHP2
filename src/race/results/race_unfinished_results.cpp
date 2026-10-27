#include "../session/race_session.hpp"
#include <cstdlib>
#include "../rules/race_rules_x87.hpp"
namespace nfs::application::pending {
extern Dword checkpoint_sort_index;
void record_participant_checkpoint(void *, int, int, Dword);

int participant_average_checkpoint_time(void *record) {
    const int checkpoint = at<int>(record, 12);
    const int ticks = checkpoint < 0 ? -1 : at<int>(record, 56 + Dword(checkpoint) * 4);
    return checkpoint < 1 || ticks < 1 ? -1 : ticks / checkpoint;
}

int __cdecl compare_snapshot_checkpoint_times(const void *first, const void *second) {
    void *snapshot = at<void *>(system(92), 8);
    const int checkpoint = int(checkpoint_sort_index);
    const auto *left = participant(snapshot, *static_cast<const Dword *>(first));
    const auto *right = participant(snapshot, *static_cast<const Dword *>(second));
    const Dword a = checkpoint < 0 ? Dword(-1) : Dword(left->checkpoint_times[checkpoint]);
    const Dword b = checkpoint < 0 ? Dword(-1) : Dword(right->checkpoint_times[checkpoint]);
    return int(a - b);
}

void complete_unfinished_results(void *race) {
    const Dword event = at<Dword>(system(72), 152);
    if (event == 10 || event == 4)
        return;
    const int last_lap = final_lap();
    using race_rules_x87::retained;
    using race_rules_x87::truncate;
    Dword separation = 0;
    for (int slot = 0; slot < at<int>(system(72), 168); ++slot, separation += 11) {
        auto *record = participant(race, Dword(slot));
        if (game::runtime::ai::lifecycle::pending::participant_enabled(race, Dword(slot)) ||
            record->kind == 3 || record->kind == 2)
            continue;
        void *vehicle = network::packets::pending::find_race_vehicle(slot);
        if (!vehicle || participant_has_finished(record) || record->retirement_tick > -1 ||
            record->disconnected)
            continue;
        const float distance = at<float>(vehicle, 564);
        const int completed = record->completed_checkpoints;
        const int *offsets = at<const int *>(vehicle, 4);
        void *actor = invoke<void *>(bytes(vehicle) + 4 + offsets[2], 1);
        float speed = at<float>(at<void *>(actor, 368), 1856);
        if (speed < 10.0f)
            speed = float(static_cast<long double>(slot) + 10.0f);
        Dword finish = invoke<Dword>(race, 22);
        const Dword remaining = truncate(retained(distance) / retained(speed) * -60.0f);
        finish += separation - Dword(remaining);
        Dword interval = finish;
        if (completed > 0)
            interval -= Dword(participant_checkpoint_duration(record, completed - 1));
        const int average = participant_average_checkpoint_time(record);
        if (average > 0)
            interval = Dword(int(Dword(average) + interval) / 2);
        for (int lap = record->completed_checkpoints; lap <= last_lap; ++lap) {
            record_participant_checkpoint(record, lap, -1, finish);
            finish += interval;
            interval = truncate(static_cast<long double>(int(interval)) * 0.980000019073486328125f);
        }
    }
    for (int lap = 0; lap <= last_lap; ++lap) {
        Dword slots[24];
        unsigned count = 0;
        int maximum = 0;
        bool existing_position = false;
        for (int slot = 0; slot < at<int>(system(72), 168); ++slot) {
            auto *record = participant(race, Dword(slot));
            if (record->kind == 3 || record->kind == 2 || participant_has_finished(record) ||
                record->retirement_tick > -1 || record->disconnected)
                continue;
            const int position = record->checkpoint_positions[lap];
            if (position < 0 || position > 7)
                slots[count++] = Dword(slot);
            else {
                existing_position = true;
                if (position > maximum)
                    maximum = position;
            }
        }
        if (count) {
            checkpoint_sort_index = Dword(lap);
            if (count > 1)
                std::qsort(slots, count, 4, compare_snapshot_checkpoint_times);
            int position = maximum + int(existing_position);
            for (unsigned index = 0; index < count; ++index, ++position)
                participant(race, slots[index])->checkpoint_positions[lap] = position;
        }
    }
    if (at<Dword>(system(72), 152) == 1) {
        Dword slots[24];
        unsigned count = 0;
        for (int slot = 0; slot < at<int>(system(72), 168); ++slot) {
            auto *record = participant(race, Dword(slot));
            if (network::packets::pending::find_race_vehicle(slot) &&
                !participant_has_finished(record) && record->retirement_tick <= -1 &&
                (!record->disconnected || record->last_checkpoint > 0))
                slots[count++] = Dword(slot);
        }
        checkpoint_sort_index = Dword(last_lap);
        if (count > 1) {
            std::qsort(slots, count, 4, compare_snapshot_checkpoint_times);
            Dword rank = Dword(last_lap);
            for (unsigned index = 1; index < count; ++index, --rank)
                participant(race, slots[index])->finish_position = int(rank);
        }
    }
}
} // namespace nfs::application::pending
