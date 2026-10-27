#include "ai_population_runtime.hpp"
#include "nfs/resources/archives/archive_cache.hpp"
#include "../../math/vectors/retained_vector.hpp"
#include "ai_population_x87.hpp"
namespace nfs::game::runtime::ai::lifecycle::pending {
extern Dword location_reuse_ticks;
extern Dword traffic_reservation_queries;
void __fastcall bind_traffic_location(void *, void *, Dword, const Dword *);

const Dword traffic_spawn_location_methods[] = {reinterpret_cast<Dword>(bind_traffic_location)};
namespace {
void *car_interface(void *car, unsigned slot) {
    return bytes(car) + 4 + reinterpret_cast<const int *>(at<Dword>(car, 4))[slot];
}
bool circuit_track() {
    Dword *tracks = system(40);
    return at<Byte>(at<void *>(tracks, 12 + tracks[2] * 4), 24) != 0;
}
bool retain_location(void *selected, Byte selected_near, int selected_distance, Byte within_range,
                     int distance) {
    if (!selected)
        return true;
    if (!selected_near && within_range)
        return false;
    if (selected_near && !within_range)
        return true;
    return distance < selected_distance;
}
} // namespace

int track_point_count(void *owner) {
    void *track = at<void *>(owner, 28);
    return track ? at<int>(track, 20) : 0;
}
// Only the AL predicate is consumed.
Byte location_available(void *location) {
    const Dword released = at<Dword>(location, 32);
    return (!released || int(released + location_reuse_ticks) <
                             int(graphics::unresolved::camera_state)) &&
           !at<Dword>(location, 28);
}

const float *path_position(void *path, int distance) {
    void *reference;
    int node;
    locate_path_sample(path, distance, &reference, &node);
    return static_cast<float *>(dependencies::section_node(at<void *>(reference, 8), node));
}

void *construct_traffic_location(void *location) {
    construct_location(location);
    at<Dword>(location, 28) = at<Dword>(location, 32) = 0;
    at<const Dword *>(location, 0) = traffic_spawn_location_methods;
    return location;
}

void __fastcall bind_traffic_location(void *location, void *, Dword ordinal, const Dword *marker) {
    at<Dword>(location, 36) = 0;
    bind_location(location, nullptr, ordinal, marker);
}

void assign_traffic_location(void *location, Dword owner) {
    at<Dword>(location, 28) = owner;
}

void load_traffic_location_records(void *owner) {
    Dword *level = system(24);
    const int count = at<int>(level, 60 + at<Dword>(owner, 4) * 4);
    at<int>(owner, 8) = count;
    auto *locations = static_cast<Byte *>(files::archives::allocate_entry(
        int(Dword(count) * 40), "AI::LocationManager::m_locations", nfs::lifecycle::level_pool));
    if (locations) {
        try {
            for (int i = 0; i < count; ++i)
                construct_traffic_location(locations + Dword(i) * 40);
        } catch (...) {
            gui::application_release(locations);
            throw;
        }
    }
    at<void *>(owner, 12) = locations;
    const Dword total = at<Dword>(owner, 8);
    if (total) {
        const int type = at<int>(owner, 4);
        for (Dword i = 0; i < total; ++i)
            invoke(locations + i * 40, 0, i,
                   static_cast<const Dword *>(level_marker(level, type, int(i))));
    }
}

void __fastcall initialize_traffic_locations(void *owner, void *) {
    release_population_locations(owner);
    load_traffic_location_records(owner);
    Dword count = at<Dword>(owner, 8);
    Byte *location = bytes(at<void *>(owner, 12));
    while (count) {
        assign_traffic_location(location, 0);
        location += 40;
        --count;
    }
    at<Dword>(owner, 16) = at<Dword>(owner, 8);
}

Dword __fastcall reserve_roadblock_location(void *owner, void *, void *entity) {
    void *car = invoke<void *>(car_interface(entity, 2), 0);
    if (!car)
        return 0;
    const int current = at<int>(at<void *>(car, 368), 7984);
    const bool circuit = circuit_track();
    void *selected = nullptr;
    Byte selected_near;
    int selected_distance;
    Dword count = at<Dword>(owner, 8);
    Byte *location = bytes(at<void *>(owner, 12));
    while (count) {
        if (!at<Byte>(location, 28) && (current <= at<int>(location, 8) || circuit)) {
            float position[] = {at<float>(location, 16), at<float>(location, 20),
                                -at<float>(location, 24)};
            Byte within_range =
                player_within_range(system(76), position, at<float>(system(76), 84632));
            if (state<Dword>(7352732))
                within_range = 0;
            int distance = int(at<Dword>(location, 8) - Dword(current));
            if (distance < 0)
                distance =
                    int(Dword(distance) + Dword(track_point_count(game::runtime::track_collision)));
            if (!selected || retain_location(selected, selected_near, selected_distance,
                                             within_range, distance)) {
                selected = location;
                selected_near = within_range;
                selected_distance = distance;
            }
        }
        location += 64;
        --count;
    }
    if (selected)
        at<Byte>(selected, 28) = 1;
    return reinterpret_cast<Dword>(selected);
}

Dword __fastcall reserve_traffic_location(void *owner, void *, Dword requester) {
    return population_frame::reserve_traffic_location(owner, requester);
}

Dword __fastcall reserve_nearest_traffic_location(void *owner, void *, void *entity) {
    const Dword current = at<Dword>(at<void *>(entity, 368), 7984);
    void *selected = nullptr;
    int nearest_distance = -1;
    Dword remaining = at<Dword>(owner, 8);
    auto *location = bytes(at<void *>(owner, 12));
    while (remaining) {
        if (!at<Dword>(location, 28)) {
            int distance = int(at<Dword>(location, 8) - current);
            if (distance < 0)
                distance = int(Dword(distance) +
                               Dword(track_point_count(game::runtime::track_collision)));
            if (nearest_distance == -1 || distance < nearest_distance) {
                selected = location;
                nearest_distance = distance;
                at<Dword>(location, 36) = 0;
            }
        }
        location += 40;
        --remaining;
    }
    if (selected) {
        if (at<Dword>(owner, 16))
            --at<Dword>(owner, 16);
        at<void *>(selected, 28) = entity;
        ++at<Dword>(owner, 24);
    }
    return reinterpret_cast<Dword>(selected);
}
} // namespace nfs::game::runtime::ai::lifecycle::pending
