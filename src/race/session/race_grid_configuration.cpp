#include "race_session.hpp"
#include "nfs/core/configuration/launch_options.hpp"
#include <cstdlib>
namespace nfs::application::pending {

void __fastcall configure_race_population(void *race, void *) {
    Dword *rules = system(72);
    int humans = at<int>(rules, 176), racers = at<int>(rules, 180), cops = racers;
    const int requested_racers = racers;
    bool derive_cops = true;
    if (at<Dword>(rules, 152) != 6) {
        if (!invoke<Byte>(race, 9)) {
            if (!(services::special_race_mode(rules) & 255)) {
                if (at<Dword>(rules, 244)) {
                    if (at<Dword>(rules, 152) != 1) {
                        if (racers > 0)
                            racers = static_cast<int>(Dword(6) - Dword(humans));
                        if (requested_racers == 1 && racers > 1)
                            racers = 1;
                    }
                } else if (at<Dword>(rules, 152) == 1 || requested_racers < 2) {
                    cops = 0;
                    derive_cops = false;
                } else {
                    racers = static_cast<int>(Dword(8) - Dword(humans));
                    cops = 0;
                    derive_cops = false;
                }
            } else if (at<Dword>(system(72), 148) == 4) {
                cops = 0;
                derive_cops = false;
            }
            if (derive_cops) {
                cops = static_cast<int>(Dword(12) - Dword(humans) - Dword(racers));
                if (cops > 7)
                    cops = 8;
            }
            if (at<Dword>(rules, 152) == 1 &&
                static_cast<int>(network::packets::rule_value(rules, 13)) > 1) {
                network::packets::pending::set_game_rule(system(72), 7,
                                                             at<Dword>(system(72), 228) - 1);
                racers = static_cast<int>(Dword(racers) - 1);
            }
            if (launch::field<Dword>(8627812))
                cops = std::atoi(launch::field<char[24]>(8628940));
        } else {
            if (!(services::special_race_mode(rules) & 255))
                racers = 6;
            cops = static_cast<int>(Dword(12) - Dword(humans) - Dword(racers));
            if (cops > 7)
                cops = 8;
        }
    } else {
        cops = static_cast<int>(Dword(12) - Dword(humans) - Dword(racers));
        if (cops > 7)
            cops = 8;
    }
    network::packets::pending::set_game_rule(rules, 14, Dword(racers));
    at<int>(rules, 172) = cops;
    at<Dword>(rules, 184) = at<Dword>(rules, 200) ? 4 : 0;
    if (launch::field<Dword>(8627880)) {
        network::packets::pending::set_game_rule(rules, 12, 3);
        at<Dword>(rules, 184) = 4;
    }
    at<Dword>(rules, 168) = at<Dword>(rules, 180) + at<Dword>(rules, 176) + at<Dword>(rules, 172);
}

void __fastcall configure_race_grid(void *race, void *, Dword retain_grid) {
    invoke(race, 7);
    Dword *rules = system(72);
    const int racers = at<int>(rules, 180), cops = at<int>(rules, 172),
              traffic = at<int>(rules, 184), humans = at<int>(rules, 176);
    const Dword selection = at<Dword>(rules, 188);
    GridCarPool pool;
    for (unsigned i = 0; i < 8; ++i) {
        pool.cars[i] = -1;
        pool.totals[i] = pool.humans[i] = pool.racers[i] = pool.cops[i] = pool.special_cops[i] = 0;
    }
    Dword slot = 0;
    Dword grid_position = Dword(7) - Dword(racers);
    for (; static_cast<int>(slot) < humans; ++slot, --grid_position) {
        void *entry = grid_entry(rules, slot);
        at<Dword>(entry, 0) = 1;
        at<Dword>(entry, 16) = 0;
        at<Dword>(entry, 20) = at<Dword>(entry, 8) ? slot + 8 : grid_position;
        const int car = at<int>(entry, 60);
        if (grid_car_slot(&pool, car) == Dword(-1)) {
            const Dword empty = grid_free_slot(&pool);
            if (empty != Dword(-1))
                pool.cars[empty] = car;
        }
        const Dword selected = grid_car_slot(&pool, car);
        if (selected != Dword(-1)) {
            ++pool.totals[selected];
            ++pool.humans[selected];
        }
        if (!network::session_active())
            at<Dword>(entry, 4) = 0;
    }
    if (slot < 24) {
        do {
            void *entry = grid_entry(rules, slot);
            at<Dword>(entry, 0) = 0;
            if (!network::session_active())
                at<Dword>(entry, 4) = 0;
        } while (++slot < 24);
    }
    invoke(race, 11, &pool, racers, cops, selection);
    slot = 0;
    for (; static_cast<int>(slot) < humans; ++slot) {
        const Dword selected = grid_car_slot(&pool, at<int>(grid_entry(rules, slot), 60));
        // The code indexes with the returned slot without a missing-car guard.
        --pool.humans[selected];
    }
    for (int i = 0; i < cops; ++i, ++slot) {
        void *entry = grid_entry(rules, slot);
        at<Dword>(entry, 16) = 0;
        at<Dword>(entry, 12) = 0;
        at<Dword>(entry, 8) = 1;
        at<Dword>(entry, 20) = Dword(i) + 10;
        if (i >= static_cast<int>(Dword(cops) - 2))
            at<Dword>(entry, 12) = 1;
        invoke(race, 13, &pool, slot, Byte(at<Dword>(entry, 12) != 0));
    }
    if (!retain_grid) {
        int accepted = 0;
        Dword candidate = slot;
        while (accepted < racers) {
            invoke(race, 10, candidate);
            const Dword selected = at<Dword>(grid_entry(rules, candidate), 52);
            bool duplicate = false;
            for (Dword previous = slot; previous < candidate; ++previous)
                if (at<Dword>(grid_entry(rules, previous), 52) == selected) {
                    duplicate = true;
                    break;
                }
            if (!duplicate) {
                ++accepted;
                ++candidate;
            }
        }
    }
    grid_position = 7;
    for (int i = 0; i < racers; ++i, ++slot, --grid_position) {
        void *entry = grid_entry(rules, slot);
        at<Dword>(entry, 8) = 0;
        at<Dword>(entry, 16) = 0;
        at<Dword>(entry, 20) = grid_position;
        invoke(race, 12, &pool, slot);
    }
    for (int i = 0; i < traffic; ++i, ++slot) {
        void *entry = grid_entry(rules, slot);
        at<Dword>(entry, 8) = 0;
        at<Dword>(entry, 16) = 1;
        at<Dword>(entry, 20) = Dword(-1);
    }
}
} // namespace nfs::application::pending
