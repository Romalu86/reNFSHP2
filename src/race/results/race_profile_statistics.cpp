#include "../session/race_session.hpp"
#include "nfs/physics/frame/simulation_runtime.hpp"
#include "race_profile_metrics.hpp"
namespace nfs::application::pending {
struct CarRunStatistics {
    Byte flags, padding[3];
    Dword fields[31];
};
static_assert(sizeof(CarRunStatistics) == 128);
// Bytes1..3 are not assigned by the saved copy constructor.
CarRunStatistics *copy_car_run_statistics(CarRunStatistics *destination, const void *source) {
    destination->flags = at<Byte>(source, 0);
    for (unsigned offset = 4; offset < 128; offset += 4)
        at<Dword>(destination, offset) = at<Dword>(source, offset);
    return destination;
}
// Complete destructor leaf.
void destroy_car_run_statistics(CarRunStatistics *) {
}

void accumulate_car_run_statistics(void *profile, CarRunStatistics run, Dword car) {
    const int ticks = at<int>(&run, 64);
    if (ticks) {
        int &best = at<int>(profile, 5632 + car * 4);
        if (!best || ticks <= best)
            best = ticks;
    }
    at<Dword>(profile, 7236) += at<Dword>(&run, 16);
    destroy_car_run_statistics(&run);
}

void record_profile_lap(void *profile, Dword track, Dword car, int ticks) {
    int &best = at<int>(profile, 716 + (track * 86 + car) * 4);
    if (ticks < best || best < 1)
        best = ticks;
}

void persist_race_statistics(void *race) {
    const Dword track = at<Dword>(system(40), 8);
    for (Dword player = 0; player < 2; ++player) {
        const Dword vehicle_offset = 84164 + player * 4;
        const int slot = at<int>(system(72), 204 + player * 4);
        if (slot == -1)
            continue;
        void *profile = bytes(system(72)) + 2488 + player * 50088;
        if (!profile)
            continue;
        auto *record = participant(race, Dword(slot));
        void *car = game::runtime::physics::local_car(system(68), int(player));
        const Dword car_type = at<Dword>(at<void *>(system(76), vehicle_offset), 356);
        at<float>(profile, 7552) = float(static_cast<long double>(at<float>(car, 1840)) +
                                        at<float>(profile, 7552));
        record_profile_lap(profile, track, car_type, record->best_lap_ticks);
        at<Dword>(profile, 6716) += record->takedowns;
        float car_speeds[2];
        profile_metrics::select_speeds(&at<float>(profile, 5288 + car_type * 4),
                                      &at<float>(profile, 6372 + car_type * 4),
                                      &record->maximum_speed, &record->average_speed, car_speeds);
        record_profile_car_speeds(profile, car_type, car_speeds[0], car_speeds[1]);
        CarRunStatistics run;
        copy_car_run_statistics(&run, bytes(car) + 1820);
        accumulate_car_run_statistics(profile, run, car_type);
        float track_speeds[2];
        profile_metrics::select_speeds(&at<float>(profile, 5240 + track * 4),
                                      &at<float>(profile, 6324 + track * 4),
                                      &record->maximum_speed, &record->average_speed, track_speeds);
        const Dword track_rule = network::packets::rule_value(system(72), 5);
        record_profile_track_speeds(profile, track_rule, track_speeds[0], track_speeds[1]);
        profile_metrics::update_maximum(&at<float>(profile, 6320), &record->maximum_speed);
        auto add_points = [&](Dword value) {
            if (int(value) > 0) {
                at<Dword>(profile, 7240) += value;
                at<Dword>(profile, 7244 + at<Dword>(race, 34620) * 4) += value;
                at<Dword>(profile, 7300 + at<Dword>(race, 34616) * 4) += value;
            }
        };
        Dword points = 0;
        for (unsigned kind = 0; kind < 5; ++kind)
            if (kind != 3)
                points += record->awards[kind];
        add_points(points);
        add_points(record->awards[3]);
        ++at<Dword>(profile, 7232);
        if (at<Dword>(system(72), 152) != 2 ||
            event_complete_progress(active_event_progress(system(72)))) {
            ++at<Dword>(profile, 164 + at<Dword>(race, 34620) * 4);
            ++at<Dword>(profile, 220 + network::packets::rule_value(system(72), 1) * 4);
        }
        const int completed = int(Dword(record->completed_checkpoints) - 1);
        const Dword ticks = completed < 0 ? Dword(-1) : Dword(record->checkpoint_times[completed]);
        at<Dword>(profile, 6724) += ticks;
        const Dword event = at<Dword>(race, 34620);
        if (event == 5)
            at<Dword>(profile, 6720) += ticks;
        else if (event == 10)
            at<Dword>(profile, 6724) += network::packets::rule_value(system(72), 3);
        for (int count = int(record->arrests); count > 0; --count) {
            ++at<Dword>(profile, 4892 + car_type * 4);
            ++at<Dword>(profile, 4844 + track_rule * 4);
            ++at<Dword>(profile, 5236);
        }
        if (int(record->award_times[2]) > 0)
            at<Dword>(profile, 7556) += record->award_times[2];
        const Dword current_event = at<Dword>(system(72), 152);
        if (current_event == 10 ||
            (current_event == 2 && !event_complete_progress(active_event_progress(system(72)))))
            continue;
        const int position = at<int>(race, 34296 + Dword(slot) * 4);
        const Dword class_rule = network::packets::rule_value(system(72), 16);
        const int opponents = int(network::packets::rule_value(system(72), 14));
        if (opponents > 0)
            ++at<Dword>(profile, 704 + class_rule * 4);
        if (at<Dword>(race, 34616) == 5) {
            ++at<Dword>(profile, 6728 + event * 4);
            if (position == 0) {
                ++at<Dword>(profile, (event + 212) * 32);
                ++at<Dword>(profile, 244 + event * 32);
            }
        } else if (position == 0 && opponents > 0) {
            ++at<Dword>(profile, 244 + event * 32);
            ++at<Dword>(profile, 692 + class_rule * 4);
        }
    }
}
} // namespace nfs::application::pending
