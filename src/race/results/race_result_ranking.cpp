#include "../session/race_session.hpp"
#include "nfs/network/session/network_gui.hpp"
#include <cstdlib>
namespace nfs::application::pending {
int __cdecl compare_racer_results(const void *, const void *);
int __cdecl compare_cop_results(const void *, const void *);

Dword count_event_positions(void *progress, Dword slot, Dword position) {
    Dword count = 0;
    for (Dword stage = 0; stage < event_stage(progress); ++stage)
        if (at<Byte>(progress, 308 + slot + stage * 24) == position)
            ++count;
    return count;
}

int __cdecl compare_event_points(const void *first, const void *second) {
    void *progress = active_event_progress(system(72));
    const Dword a = *static_cast<const Dword *>(first), b = *static_cast<const Dword *>(second);
    const Dword points_a = event_points(progress, a), points_b = event_points(progress, b);
    if (points_a != points_b)
        return int(points_b - points_a);
    network::ui::CarSettings first_car, second_car;
    network::ui::initialize_car_settings(&first_car);
    network::ui::initialize_car_settings(&second_car);
    read_event_participant(progress, a, &first_car);
    read_event_participant(progress, b, &second_car);
    const Dword human_a = at<Dword>(&first_car, 0), human_b = at<Dword>(&second_car, 0);
    if ((human_a != 0) != (human_b != 0))
        return int(human_a == 0) * 2 - 1;
    for (Dword position = 0; position < 3; ++position) {
        const Dword left = count_event_positions(progress, a, position);
        const Dword right = count_event_positions(progress, b, position);
        if (left != right)
            return int(right - left);
    }
    return -1;
}

int __cdecl compare_arrest_results(const void *first, const void *second) {
    void *snapshot = at<void *>(system(92), 8);
    const int a = int(participant(snapshot, *static_cast<const Dword *>(first))->arrests);
    const int b = int(participant(snapshot, *static_cast<const Dword *>(second))->arrests);
    if (a < 0)
        return int(b < 0) - 1;
    if (b < 0)
        return 1;
    return int(Dword(b) - Dword(a));
}
namespace {
template <class Select>
void rank_selected(void *race, Select selected, int(__cdecl *compare)(const void *, const void *)) {
    Dword slots[24];
    unsigned count = 0;
    for (Dword slot = 0; slot < at<Dword>(system(72), 168); ++slot)
        if (selected(slot))
            slots[count++] = slot;
    if (count > 1)
        std::qsort(slots, count, 4, compare);
    void *rules = system(72);
    for (Dword slot = 0; slot < at<Dword>(rules, 168); ++slot)
        if (selected(slot))
            for (unsigned rank = 0; rank < count; ++rank)
                if (slots[rank] == slot)
                    at<Dword>(race, 34296 + slot * 4) = rank;
}
bool racer(void *race, Dword slot) {
    const int kind = participant(race, slot)->kind;
    return kind != 3 && kind != 2;
}
} // namespace

void rank_race_finishers(void *race) {
    rank_selected(
        race, [=](Dword slot) { return racer(race, slot); }, compare_event_points);
}

void rank_race_racers(void *race) {
    rank_selected(
        race, [=](Dword slot) { return racer(race, slot); }, compare_racer_results);
}

void rank_race_cops(void *race) {
    rank_selected(
        race, [=](Dword slot) { return !racer(race, slot); }, compare_arrest_results);
}

void rank_quota_players(void *race) {
    Dword slots[24];
    int count = 0;
    for (Dword slot = 0; slot < at<Dword>(system(72), 168); ++slot)
        if (game::runtime::ai::lifecycle::pending::participant_enabled(race, slot))
            slots[count++] = slot;
    if (count > 1)
        std::qsort(slots, unsigned(count), 4, compare_cop_results);
    // The admission query reloads session/rules state in both passes.
    for (Dword slot = 0; slot < at<Dword>(system(72), 168); ++slot)
        if (game::runtime::ai::lifecycle::pending::participant_enabled(race, slot))
            for (int rank = 0; rank < count; ++rank)
                if (slots[rank] == slot)
                    at<Dword>(race, 34296 + slot * 4) = Dword(rank);
}

void rank_elimination_finishers(void *race) {
    rank_selected(
        race, [=](Dword slot) { return racer(race, slot); }, compare_racer_results);
}
} // namespace nfs::application::pending
