#include "ai_population_runtime.hpp"
#include "nfs/resources/archives/archive_cache.hpp"
#include "nfs/core/errors/assertions.hpp"
#include "ai_population_x87.hpp"
namespace nfs::game::runtime::ai::lifecycle::pending {
void __fastcall bind_location(void *, void *, Dword, const Dword *);

const Dword location_base_methods[] = {reinterpret_cast<Dword>(bind_location)};
const Dword population_location_methods[] = {reinterpret_cast<Dword>(bind_location)};

int nearest_track_point(void *level, const float *position) {
    return population_frame::nearest_track_point(level, position);
}

void *construct_location(void *location) {
    for (unsigned offset = 4; offset <= 24; offset += 4)
        at<Dword>(location, offset) = 0;
    at<const Dword *>(location, 0) = location_base_methods;
    return location;
}

void __fastcall bind_location(void *location, void *, Dword ordinal, const Dword *marker) {
    at<Dword>(location, 4) = ordinal;
    at<const Dword *>(location, 12) = marker;
    if (marker) {
        at<Dword>(location, 16) = marker[0];
        at<Dword>(location, 20) = marker[1];
        at<Dword>(location, 24) = marker[2];
        float position[] = {at<float>(location, 16), at<float>(location, 20),
                            -at<float>(location, 24)};
        at<int>(location, 8) = nearest_track_point(game::runtime::track_collision, position);
    } else {
        at<Dword>(location, 16) = at<Dword>(location, 20) = at<Dword>(location, 24) = 0;
        at<Dword>(location, 8) = Dword(-1);
    }
}

void load_population_locations(void *owner) {
    Dword *level = system(24);
    const int count = at<int>(level, 60 + at<Dword>(owner, 4) * 4);
    at<int>(owner, 8) = count;
    auto *locations = static_cast<Byte *>(files::archives::allocate_entry(
        int(Dword(count) << 6), "AI::LocationManager::m_locations", nfs::lifecycle::level_pool));
    if (locations) {
        try {
            for (int i = 0; i < count; ++i) {
                void *location = locations + Dword(i) * 64;
                construct_location(location);
                at<Dword>(location, 52) = 0;
                at<Dword>(location, 56) = Dword(-1);
                at<Byte>(location, 60) = 0;
                at<Byte>(location, 28) = 0;
                at<Dword>(location, 44) = at<Dword>(location, 32) = 0;
                at<Byte>(location, 48) = 0;
                at<const Dword *>(location, 0) = population_location_methods;
            }
        } catch (...) {
            gui::application_release(locations);
            throw;
        }
    }
    at<void *>(owner, 12) = locations;
    const Dword total = at<Dword>(owner, 8);
    if (total) {
        const Dword type = at<Dword>(owner, 4);
        for (Dword i = 0; i < total; ++i)
            invoke(locations + i * 64, 0, i,
                   static_cast<const Dword *>(level_marker(level, int(type), int(i))));
    }
}

void clear_population_locations(void *owner) {
    Dword count = at<Dword>(owner, 8);
    Byte *location = bytes(at<void *>(owner, 12));
    while (count) {
        at<Byte>(location, 28) = 0;
        at<Dword>(location, 44) = at<Dword>(location, 32) = 0;
        at<Byte>(location, 48) = 0;
        location += 64;
        --count;
    }
}

void initialize_population_locations(void *owner) {
    if (void *locations = at<void *>(owner, 12)) {
        kernel::memory_free(locations);
        at<void *>(owner, 12) = nullptr;
    }
    at<Dword>(owner, 8) = 0;
    load_population_locations(owner);
    clear_population_locations(owner);
}

void release_population_locations(void *owner) {
    if (void *locations = at<void *>(owner, 12)) {
        kernel::memory_free(locations);
        at<void *>(owner, 12) = nullptr;
    }
    at<Dword>(owner, 8) = at<Dword>(owner, 24) = 0;
}

void release_population_location(void *location) {
    for (int i = 0, count = at<int>(location, 32); i < count; ++i)
        ai::pending::dispatch_vehicle_event(at<Dword *>(location, 36 + Dword(i) * 4), 7);
    if (void *spike = at<void *>(location, 44))
        deactivate_spike_entity(spike);
    at<Byte>(location, 28) = 0;
    at<Dword>(location, 44) = at<Dword>(location, 32) = 0;
    at<Byte>(location, 48) = 0;
}

void finish_population_load(Dword *owner) {
    Dword count = owner[2];
    Byte *location = bytes(reinterpret_cast<void *>(owner[3]));
    while (count) {
        bind_population_location_path(location);
        location += 64;
        --count;
    }
}
} // namespace nfs::game::runtime::ai::lifecycle::pending
