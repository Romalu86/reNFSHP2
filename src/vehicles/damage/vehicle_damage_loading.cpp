#include "../rendering/vehicle_renderable_resources.hpp"
#include "nfs/resources/files/file_cache.hpp"
#include "nfs/core/lifecycle/object_state.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include "nfs/core/configuration/launch_options.hpp"
#include "../../math/precision/extended_float.hpp"
#include <cstdio>
#include <cstring>
namespace nfs::game::world::vehicle_render {
#include "vehicle_damage_bounds.inc"
namespace {
float coefficient(Dword bits) {
    float value;
    std::memcpy(&value, &bits, 4);
    return value;
}
} // namespace The constructor writes this one field only.
 
void construct_damage_zone(void *zone) {
    at<Dword>(zone, 60) = 0;
}
// The second output uses the upper bound for both products.
void bind_damage_zone_bounds(void *zone, const float *lower, const float *upper) {
    const auto format = kernel::numeric::current_format();
    auto X = [&](float value) { return kernel::numeric::Extended(value, format); };
    Dword index = at<Dword>(zone, 4);
    for (unsigned axis = 0; axis < 3; ++axis) {
        float first = coefficient(damage_lower_coefficients[index * 3 + axis]);
        float second = coefficient(damage_upper_coefficients[index * 3 + axis]);
        at<float>(zone, 28 + axis * 4) = float(X(lower[axis]) * first + X(second) * upper[axis]);
    }
    for (unsigned axis = 0; axis < 3; ++axis) {
        float first = coefficient(damage_lower_coefficients[index * 3 + axis]);
        float second = coefficient(damage_upper_coefficients[index * 3 + axis]);
        at<float>(zone, 40 + axis * 4) = float(X(upper[axis]) * first + X(second) * upper[axis]);
    }
}

Byte bind_damage_animation(void *owner, Dword identifier, Dword rate, Dword time, Dword first_channel) {
    if (!identifier)
        return 0;
    void *entry = find_animation_entry(at<void *>(owner, 64), identifier);
    if (!entry)
        return 0;
    void *rotation = at<void *>(entry, 24), *translation = nullptr, *extra = nullptr;
    entry = find_animation_entry(at<void *>(owner, 64), identifier + 1000000);
    if (entry)
        translation = at<void *>(entry, 24);
    entry = find_animation_entry(at<void *>(owner, 64), identifier + 2000000);
    if (entry)
        extra = at<void *>(entry, 24);
    if (!translation)
        translation = rotation;
    bind_animation_channel(owner, first_channel, rotation, rate, time);
    bind_animation_channel(owner, first_channel + 1, translation, rate, time);
    bind_animation_channel_rotation(owner, first_channel + 2, extra, rotation, rate, time);
    return 1;
}

Byte pending::has_damage_bank(const char *resource) {
    char filename[256];
    std::sprintf(filename, "%s%s\\animbank.o", offset(subsystem(4), 2084), resource);
    return files::file::exists(filename);
}

void pending::load_damage(void *owner, const char *resource, const Dword *lower, const Dword *upper) {
    char filename[260];
    std::sprintf(filename, "%s%s\\damage.ini", offset(subsystem(4), 2084), resource);
    std::memcpy(offset(owner, 24), lower, 12);
    std::memcpy(offset(owner, 36), upper, 12);
    at<Byte>(owner, 20) = 1;
    auto *allocation = static_cast<Dword *>(
        lifecycle::allocate(724, "VEHICLE DAMAGE ZONE LIST", lifecycle::cars_pool));
    void *zones = nullptr;
    if (allocation) {
        *allocation = 10;
        try {
            lifecycle::construction::construct_array(allocation + 1, 72, 10,
                                                     construct_damage_zone, destroy_damage_zone);
        } catch (...) {
            kernel::memory_free(allocation);
            throw;
        }
        zones = allocation + 1;
    }
    at<void *>(owner, 76) = zones;
    for (unsigned index = 0; index < 10; ++index) {
        void *zone = offset(at<void *>(owner, 76), index * 72);
        at<Dword>(zone, 0) = index + 1;
        at<Dword>(zone, 4) = index;
        at<Byte>(zone, 53) = 0;
        bind_damage_zone_bounds(zone, reinterpret_cast<float *>(offset(owner, 24)),
                                reinterpret_cast<float *>(offset(owner, 36)));
        at<Dword>(zone, 8) = 3;
        for (int channel = 0; channel < at<int>(zone, 8); ++channel)
            at<Dword>(zone, 12 + Dword(channel) * 4) = at<Dword>(zone, 8) * index + Dword(channel);
    }
    at<Dword>(owner, 60) = 30;
    void *table = lifecycle::allocate(56, "VEHICLE RAW ANIM TABLE", lifecycle::cars_pool);
    if (table) {
        try {
            table = construct_damage_animation_table(table);
        } catch (...) {
            gui::application_release(table);
            throw;
        }
    }
    at<void *>(owner, 64) = table;
    load_damage_animation_table(table, resource, 8, lifecycle::cars_pool);
    allocate_animation_channels(owner, "VEHICLE ANIM CHANNEL LIST", at<Dword>(owner, 60),
                                lifecycle::cars_pool);
    for (unsigned index = 0; index < 10; ++index) {
        void *zone = offset(at<void *>(owner, 76), index * 72);
        at<Byte>(zone, 52) = bind_damage_animation(owner, at<Dword>(zone, 0), 1051372191, 0,
                                                     at<Dword>(zone, 8) * index);
        Dword duration = at<Dword>(at<void *>(owner, 12), 12 + at<Dword>(zone, 12) * 20);
        Dword maximum = launch::field<Dword>(8627920);
        at<Dword>(zone, 64) = at<Dword>(zone, 68) = duration;
        at<Dword>(zone, 60) = 0;
        if (maximum)
            at<Dword>(zone, 60) = duration;
    }
}
} // namespace nfs::game::world::vehicle_render
