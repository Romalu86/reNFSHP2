#include "race_session.hpp"
#include <cstdlib>
namespace nfs::application::pending {
int __cdecl compare_event_points(const void *, const void *);

void __fastcall reset_event_two_progress(void *race, void *) {
    if (!at<Byte>(race, 44))
        restore_event_grid(at<void *>(race, 148));
}

void __fastcall configure_event_two_grid(void *race, void *, Dword) {
    Dword *rules = system(72);
    at<Byte>(race, 34592) = 0;
    at<Byte>(race, 34593) = 1;
    void *progress = active_event_progress(rules);
    at<void *>(race, 148) = progress;
    if (!progress)
        return;
    save_event_grid(active_event_progress(system(72)));
    if (!event_active(progress)) {
        if (!event_complete_progress(progress)) {
            reset_event_progress(progress);
            for (Dword slot = 0; slot < 24; ++slot) {
                void *entry = grid_entry(rules, slot);
                at<Dword>(entry, 8) = 0;
                at<Dword>(entry, 16) = 0;
            }
            configure_race_grid(race, nullptr, 0);
            progress = at<void *>(race, 148);
            const Dword flags = at<Dword>(progress, 0);
            at<Dword>(progress, 0) = (((at<Dword>(rules, 232) * 2 - 3) ^ flags) & 14) ^ flags;
            write_event_humans(progress, at<int>(rules, 176));
            write_event_cops(progress, at<Dword>(rules, 172));
            write_event_racers(progress, at<Dword>(rules, 180));
            write_event_traffic(progress, at<Dword>(rules, 184));
            write_event_car_class(progress, at<Dword>(rules, 188));
            write_event_difficulty(progress, at<Dword>(rules, 192));
            write_event_helicopters(progress, at<Dword>(rules, 200));
            for (int i = 0; i < at<int>(rules, 168); ++i)
                write_event_participant(progress, Dword(i), grid_entry(rules, Dword(i)));
        }
    } else {
        const int humans = event_humans(progress);
        network::packets::pending::set_game_rule(rules, 13, Dword(humans));
        if (humans == 1) {
            at<Dword>(rules, 204) = 0;
            at<Dword>(rules, 208) = Dword(-1);
        } else if (humans == 2) {
            at<Dword>(rules, 204) = 0;
            at<Dword>(rules, 208) = 1;
        }
        at<Dword>(rules, 172) = event_cops(progress);
        network::packets::pending::set_game_rule(rules, 14, event_racers(progress));
        at<Dword>(rules, 168) =
            at<Dword>(rules, 180) + at<Dword>(rules, 176) + at<Dword>(rules, 172);
        at<Dword>(rules, 184) = event_traffic(progress);
        network::packets::pending::set_game_rule(rules, 15, event_car_class(progress));
        network::packets::pending::set_game_rule(rules, 16, event_difficulty(progress));
        network::packets::pending::set_game_rule(rules, 12, event_helicopters(progress));
        for (int i = 0; i < at<int>(rules, 168); ++i) {
            void *entry = grid_entry(rules, Dword(i));
            if (!at<Dword>(entry, 0))
                read_event_participant(progress, Dword(i), entry);
            else {
                at<Dword>(entry, 8) = 0;
                at<Dword>(entry, 16) = 0;
            }
        }
        configure_race_grid(race, nullptr, 1);
        void *saved_progress = at<void *>(race, 148);
        for (int i = 0; i < at<int>(rules, 168); ++i)
            write_event_participant(saved_progress, Dword(i), grid_entry(rules, Dword(i)));
        if (event_stage(at<void *>(race, 148))) {
            Dword slots[24];
            int count = 0;
            for (int i = 0; i < at<int>(system(72), 168); ++i) {
                void *entry = grid_entry(system(72), Dword(i));
                if (!at<Dword>(entry, 8) && !at<Dword>(entry, 16))
                    slots[count++] = Dword(i);
            }
            if (count > 1)
                std::qsort(slots, unsigned(count), 4, compare_event_points);
            int position = 7;
            for (int i = 0; i < count; ++i)
                at<int>(grid_entry(system(72), slots[i]), 20) = position--;
        }
    }
    progress = at<void *>(race, 148);
    if (!event_complete_progress(progress)) {
        network::packets::pending::set_game_rule(rules, 5,
                                                     event_track(progress, event_stage(progress)));
        Dword track[7]{};
        read_event_track(progress, event_stage(progress), track);
        track[3] = ((at<Dword>(progress, 0) >> 1) & 7) + 2;
        track[6] = Dword(event_cops(progress) != 0);
        for (unsigned i = 0; i < 7; ++i)
            at<Dword>(rules, 220 + i * 4) = track[i];
        network::packets::pending::select_track(system(40), track[0]);
    }
}
} // namespace nfs::application::pending
