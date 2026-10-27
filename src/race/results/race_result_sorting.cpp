#include "../session/race_runtime.hpp"
#include <cstdlib>
namespace nfs::application::pending {
int compare_participant_criteria(void *, const void *);
Dword checkpoint_sort_index = 0;

Byte participant_ordered_after(void *first, const void *second) {
    const int forward = compare_participant_criteria(first, second);
    const int reverse = compare_participant_criteria(const_cast<void *>(second), first);
    return forward < reverse;
}

int __cdecl compare_racer_results(const void *first, const void *second) {
    const Dword first_slot = *static_cast<const Dword *>(first);
    void *snapshot = at<void *>(system(92), 8);
    auto *left = participant(snapshot, first_slot);
    auto *right = participant(snapshot, *static_cast<const Dword *>(second));
    if (participant_ordered_after(right, left))
        return 1;
    return -int(participant_ordered_after(left, right));
}

int __cdecl compare_cop_results(const void *first, const void *second) {
    void *snapshot = at<void *>(system(92), 8);
    const RaceParticipant *left = participant(snapshot, *static_cast<const Dword *>(first));
    const RaceParticipant *right = participant(snapshot, *static_cast<const Dword *>(second));
    const Dword a = left->losses + left->takedowns, b = right->losses + right->takedowns;
    if (static_cast<int>(a) < 0)
        return int(static_cast<int>(b) < 0) - 1;
    if (static_cast<int>(b) < 0)
        return 1;
    return static_cast<int>(b - a);
}

void __fastcall sort_participant_positions(void *race, void *, Dword *positions, int *count) {
    const Dword event = at<Dword>(system(72), 152);
    if (event != 4 && event != 6 && event != 10)
        invoke(race, 37, positions, count);
    else
        invoke(race, 38, positions, count);
}

void __fastcall sort_racer_positions(void *race, void *, Dword *positions, int *count) {
    void *rules = system(72);
    *count = 0;
    if (at<int>(rules, 168) > 0) {
        for (int slot = 0; slot < at<int>(rules, 168); ++slot)
            if (!participant_is_cop(race, Dword(slot)))
                positions[(*count)++] = Dword(slot);
        if (*count > 1)
            std::qsort(positions, unsigned(*count), 4, compare_racer_results);
    }
}

void __fastcall sort_cop_positions(void *race, void *, Dword *positions, int *count) {
    *count = 0;
    for (int slot = 0; slot < at<int>(system(72), 168); ++slot)
        if (game::runtime::ai::lifecycle::pending::participant_enabled(race, Dword(slot)))
            positions[(*count)++] = Dword(slot);
    if (*count > 1)
        std::qsort(positions, unsigned(*count), 4, compare_cop_results);
}

void *race_participant_owner_view(void *manager, Dword slot) {
    void *race = at<void *>(manager, 4);
    return race ? participant(race, slot) : nullptr;
}

int __cdecl compare_checkpoint_times(const void *first, const void *second) {
    void *left = race_participant_owner_view(system(92), *static_cast<const Dword *>(first));
    void *right = race_participant_owner_view(system(92), *static_cast<const Dword *>(second));
    if (!left || !right)
        return 0;
    const Dword index = checkpoint_sort_index;
    const Dword time = static_cast<int>(index) < 0 ? Dword(-1) : at<Dword>(left, 56 + index * 4);
    const Dword other = static_cast<int>(index) < 0 ? Dword(-1) : at<Dword>(right, 56 + index * 4);
    return static_cast<int>(time - other);
}

int update_checkpoint_order(void *race, Dword selected, int fallback, int checkpoint) {
    Dword slots[24];
    unsigned count = 0;
    for (int slot = 0; slot < at<int>(system(72), 168); ++slot)
        if (participant_completed_checkpoint(participant(race, Dword(slot)), checkpoint))
            slots[count++] = Dword(slot);
    checkpoint_sort_index = Dword(checkpoint);
    std::qsort(slots, count, 4, compare_checkpoint_times);
    for (unsigned i = 0; i < count; ++i) {
        if (checkpoint >= 0)
            participant(race, slots[i])->checkpoint_positions[checkpoint] = int(i);
        if (slots[i] == selected)
            fallback = int(i);
    }
    return fallback;
}
} // namespace nfs::application::pending
