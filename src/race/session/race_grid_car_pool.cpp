#include "race_session.hpp"
namespace nfs::application::pending {
Dword event_car_order[24]{};
Dword event_car_order_count = 0;

Dword grid_free_slot(const GridCarPool *pool) {
    for (Dword i = 0; i < 8; ++i)
        if (pool->cars[i] == -1)
            return i;
    return Dword(-1);
}

Dword grid_car_slot(const GridCarPool *pool, int car) {
    for (Dword i = 0; i < 8; ++i) {
        if (pool->cars[i] == -1)
            return Dword(-1);
        if (pool->cars[i] == car)
            return i;
    }
    return Dword(-1);
}
// Catalogue receivers are the embedded VehicleManager+4 object, not VehicleManager itself.

Byte catalogue_car_is_traffic(void *catalogue, int car) {
    for (int i = 0; i < at<int>(catalogue, 0); ++i)
        if (at<int>(catalogue, 4 + Dword(i) * 1228) == car)
            return at<Byte>(catalogue, 36 + Dword(i) * 1228);
    return 0;
}

int catalogue_car_in_class(void *catalogue, int group, int ordinal) {
    int found = 0;
    for (int i = 0; i < at<int>(catalogue, 0); ++i)
        if (at<int>(catalogue, 32 + Dword(i) * 1228) == group) {
            if (found == ordinal)
                return at<int>(catalogue, 4 + Dword(i) * 1228);
            ++found;
        }
    return -1;
}

void collect_pursuit_cars(int group, int initial_group, int *cars, int *count) {
    if (initial_group == 38 && group == 39) {
        cars[*count] = 16;
        cars[*count + 1] = 8;
        *count += 2;
        return;
    }
    for (int ordinal = 0;; ++ordinal) {
        const int car = catalogue_car_in_class(bytes(system(52)) + 4, group, ordinal);
        cars[*count] = car;
        if (car == -1)
            break;
        void *entry = bytes(system(52)) + Dword(car) * 1228 + 8;
        if (!at<Byte>(entry, 32) && at<Byte>(entry, 1076))
            ++*count;
    }
}
namespace {
bool pursuit_car(void *catalogue, int car) {
    return at<Byte>(catalogue, Dword(car) * 1228 + 1084) != 0;
}
Dword pool_used(const GridCarPool *pool) {
    Dword count = 0;
    for (int car : pool->cars)
        if (car != -1)
            ++count;
    return count;
}
void add_pool_car(GridCarPool *pool, int car) {
    if (grid_car_slot(pool, car) != Dword(-1))
        return;
    const Dword slot = grid_free_slot(pool);
    if (slot != Dword(-1))
        pool->cars[slot] = car;
}
Dword car_total(const GridCarPool *pool, int car) {
    const Dword slot = grid_car_slot(pool, car);
    return slot == Dword(-1) ? 0 : pool->totals[slot];
}
void assign_pool_car(GridCarPool *pool, int car, Dword *counts) {
    const Dword slot = grid_car_slot(pool, car);
    if (slot != Dword(-1)) {
        ++pool->totals[slot];
        ++counts[slot];
    }
}
} // namespace
// Return-register scratch is not consumed by its virtual callers.
void __fastcall prepare_grid_car_pool(void *, void *, GridCarPool *pool, Dword racers, int cops,
                                      int selection) {
    void *catalogue = system(52);
    int initial_group = 42, last_human_car = 0;
    for (Dword i = 0; i < 8 && pool->cars[i] != -1; ++i) {
        const int car = pool->cars[i];
        const int group = at<int>(catalogue, Dword(car) * 1228 + 36);
        if (group < initial_group)
            initial_group = group;
        last_human_car = car;
    }
    int ordinary_cops = 0, special_cops = 0;
    if (cops) {
        ordinary_cops = cops - 2;
        special_cops = 2;
    }
    int reserved_car = -1;
    if (racers) {
        if (selection < 38)
            reserved_car = selection;
        if (selection == 44)
            reserved_car = last_human_car;
    }
    while (pool_used(pool) < 8) {
        Dword capacity = 0;
        for (Dword i = 0; i < 8 && pool->cars[i] != -1; ++i)
            if (pursuit_car(system(52), pool->cars[i]))
                capacity += 8 - pool->totals[i];
        if (reserved_car != -1 && pursuit_car(system(52), reserved_car) &&
            grid_car_slot(pool, reserved_car) != Dword(-1)) {
            Dword slot = grid_car_slot(pool, reserved_car);
            Dword available = slot == Dword(-1) ? 0 : 8 - pool->totals[slot];
            Dword reserved = racers;
            if (static_cast<int>(available) <= static_cast<int>(racers)) {
                slot = grid_car_slot(pool, reserved_car);
                reserved = slot == Dword(-1) ? 0 : 8 - pool->totals[slot];
            }
            capacity -= reserved;
        }
        if (cops <= static_cast<int>(capacity))
            break;
        const int direction = initial_group == 42 ? -1 : 1;
        bool inserted = false;
        for (int group = initial_group; group >= 38 && group <= 42 && !inserted;
             group += direction) {
            int candidates[16], count = 0;
            collect_pursuit_cars(group, initial_group, candidates, &count);
            if (group == initial_group) {
                while (count) {
                    const Dword index = network::rules::random() % Dword(count);
                    const int car = candidates[index];
                    candidates[index] = candidates[--count];
                    if (grid_car_slot(pool, car) == Dword(-1)) {
                        add_pool_car(pool, car);
                        inserted = true;
                        break;
                    }
                }
            } else {
                for (int i = 0; i < count; ++i)
                    if (grid_car_slot(pool, candidates[i]) == Dword(-1)) {
                        add_pool_car(pool, candidates[i]);
                        inserted = true;
                        break;
                    }
            }
        }
        // The path retries if no candidate can be inserted; no fallback is added.
    }
    bool same_group = true;
    for (Dword i = 0; i < 8 && pool->cars[i] != -1; ++i) {
        const int car = pool->cars[i];
        if (pursuit_car(catalogue, car) &&
            at<int>(catalogue, Dword(car) * 1228 + 36) != initial_group) {
            same_group = false;
            break;
        }
    }
    const int used = int(pool_used(pool));
    int cursor = 0;
    while (ordinary_cops > 0) {
        int selected = -1;
        if (same_group) {
            for (;; ++cursor) {
                const int slot = cursor % used;
                if (Dword(slot) >= 8)
                    continue;
                const int car = pool->cars[slot];
                if (car == -1 || !pursuit_car(catalogue, car))
                    continue;
                Dword total = car_total(pool, car);
                if (car == reserved_car)
                    total += racers;
                if (static_cast<int>(total) < 8) {
                    selected = car;
                    break;
                }
            }
        } else {
            for (Dword i = 0; i < 8 && pool->cars[i] != -1; ++i) {
                const int car = pool->cars[i];
                if (!pursuit_car(catalogue, car))
                    continue;
                Dword total = car_total(pool, car);
                if (car == reserved_car)
                    total += racers;
                if (static_cast<int>(total) < 8) {
                    selected = car;
                    break;
                }
            }
        }
        if (selected != -1)
            assign_pool_car(pool, selected, pool->cops);
        --ordinary_cops;
    }
    for (int assigned = 0; assigned < special_cops; ++assigned) {
        for (Dword i = 0; i < 8 && pool->cars[i] != -1; ++i) {
            const int car = pool->cars[i];
            if (!pursuit_car(catalogue, car))
                continue;
            Dword total = car_total(pool, car);
            if (car == reserved_car)
                total += racers;
            if (static_cast<int>(total) < 8) {
                assign_pool_car(pool, car, pool->special_cops);
                break;
            }
        }
    }
    if ((services::special_race_mode(system(72)) & 255) && event_car_order_count) {
        while (grid_free_slot(pool) != Dword(-1)) {
            int candidates[16];
            Dword count = 0;
            if (static_cast<int>(event_car_order_count) < 1)
                break;
            for (Dword i = 0; i < event_car_order_count; ++i)
                if (grid_car_slot(pool, int(event_car_order[i])) == Dword(-1))
                    candidates[count++] = int(event_car_order[i]);
            if (!count)
                break;
            add_pool_car(pool, candidates[network::rules::random() % count]);
        }
        int ordinal = -1;
        for (Dword assigned = 0; static_cast<int>(assigned) < static_cast<int>(racers); ++assigned) {
            ++ordinal;
            for (int attempt = 0; attempt < static_cast<int>(event_car_order_count);
                 ++attempt, ++ordinal) {
                const int car = int(event_car_order[Dword(ordinal % int(event_car_order_count))]);
                if (grid_car_slot(pool, car) != Dword(-1) &&
                    at<Dword>(catalogue, Dword(car) * 1228 + 68) && car_total(pool, car) < 8) {
                    assign_pool_car(pool, car, pool->racers);
                    break;
                }
            }
        }
        return;
    }
    if (racers == 1 && selection < 38) {
        if (grid_car_slot(pool, selection) == Dword(-1))
            add_pool_car(pool, selection);
        assign_pool_car(pool, selection, pool->racers);
        return;
    }
    if (!racers)
        return;
    if (selection > 44 || selection < 38)
        selection = 43;
    while (selection != 44 && grid_free_slot(pool) != Dword(-1)) {
        int candidates[16];
        Dword count = 0;
        for (int ordinal = 0;; ++ordinal) {
            const int group = selection > 42 ? initial_group : selection;
            const int car = catalogue_car_in_class(bytes(system(52)) + 4, group, ordinal);
            if (car == -1)
                break;
            if (!catalogue_car_is_traffic(bytes(system(52)) + 4, car) &&
                grid_car_slot(pool, car) == Dword(-1))
                candidates[count++] = car;
        }
        if (!count)
            break;
        add_pool_car(pool, candidates[network::rules::random() % count]);
    }
    const int total_cars = int(pool_used(pool));
    int ordinal = -1;
    for (Dword assigned = 0; static_cast<int>(assigned) < static_cast<int>(racers); ++assigned) {
        ++ordinal;
        for (int attempt = 0; attempt < 4; ++attempt, ++ordinal) {
            const int index = ordinal % total_cars;
            if (Dword(index) >= 8)
                continue;
            const int car = pool->cars[index];
            if (car == -1)
                continue;
            void *entry = bytes(catalogue) + Dword(car) * 1228 + 8;
            if (!at<Dword>(entry, 60))
                continue;
            const int group = at<int>(entry, 28);
            if (selection >= 38 && selection <= 42 && group != selection)
                continue;
            if (selection == 43 && group != initial_group)
                continue;
            if (selection == 44 && car != last_human_car)
                continue;
            const Dword slot = grid_car_slot(pool, car);
            if (slot != Dword(-1) && static_cast<int>(pool->totals[slot]) >= 8)
                continue;
            assign_pool_car(pool, car, pool->racers);
            break;
        }
    }
}
} // namespace nfs::application::pending
