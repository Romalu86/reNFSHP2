#include "ai_population_runtime.hpp"
#include "nfs/audio/vehicles/vehicle_audio.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
#include <cstring>
namespace nfs::game::runtime::ai::lifecycle::pending {
namespace {
void *shape_at(int index) {
    void *shapes = game::runtime::track_collision;
    const int displacement = int(Dword(index) * 120);
    return displacement >= 0 && index < at<int>(shapes, 4)
               ? bytes(at<void *>(shapes, 16)) + displacement
               : nullptr;
}
void *spike_body_interface(void *entity) {
    return bytes(entity) + 4 + reinterpret_cast<const int *>(at<Dword>(entity, 4))[3];
}
void *find_profile(Dword &cursor, int kind) {
    while (cursor < state<Dword>(7353148)) {
        void *profile = state<void *>(7352968 + cursor * 4);
        if (!profile || invoke<int>(profile, 0) == kind)
            return profile;
        ++cursor;
    }
    return nullptr;
}
void attach_helicopter_blades(Dword *owner, void *shape, unsigned offset) {
    const int helicopters = int(owner[2]);
    for (int index = 0; index < helicopters; ++index)
        if (void *entity = reinterpret_cast<void *>(owner[index]))
            if (!at<Dword>(entity, offset)) {
                at<void *>(entity, offset) = shape;
                at<Byte>(shape, 98) = 1;
            }
}
} // namespace

void reset_helicopter_projectile(void *entity) {
    at<Byte>(entity, 232) = 0;
    invoke(entity, 6, Dword{0});
    at<Dword>(entity, 240) = 0;
    if (void *shape = at<void *>(entity, 236))
        if (void *body = at<void *>(shape, 80)) {
            at<Dword>(body, 676) = 0;
            at<Byte>(body, 728) = at<Byte>(body, 729) = 0;
        }
    at<Dword>(entity, 244) = at<Dword>(entity, 248) = at<Dword>(entity, 256) =
        at<Dword>(entity, 252) = 0;
    at<Byte>(entity, 260) = 0;
}

void reset_helicopter_population(Dword *owner) {
    const int helicopters = int(owner[2]);
    for (int i = 0; i < helicopters; ++i)
        if (owner[i])
            at<Byte>(reinterpret_cast<void *>(owner[i]), 348) = 0;
    const int projectiles = int(owner[8]);
    for (int i = 0; i < projectiles; ++i)
        if (owner[3 + i])
            reset_helicopter_projectile(reinterpret_cast<void *>(owner[3 + i]));
}

void deactivate_spike_entity(void *entity) {
    if (at<Byte>(entity, 232)) {
        at<Byte>(entity, 232) = 0;
        at<Dword>(entity, 284) = 0;
        if (invoke<void *>(spike_body_interface(entity), 1))
            at<Byte>(invoke<void *>(spike_body_interface(entity), 1), 729) = 0;
        invoke(entity, 6, Dword{0});
    }
}

void load_spike_population(Dword *owner) {
    owner[0] = owner[1] = 0;
    Dword cursor = 0;
    void *profile = find_profile(cursor, 8);
    const int shapes = at<int>(game::runtime::track_collision, 4);
    owner[2] = 0;
    for (int i = 0; i < shapes; ++i) {
        void *shape = shape_at(i);
        const char *name = invoke<const char *>(shape, 4);
        if (!std::strncmp(name, "spikestrip", 10) && int(owner[2]) < 2) {
            auto *entity = static_cast<Dword *>(gui::application_allocate(320));
            if (entity) {
                try {
                    entity = construct_spike_entity(entity, profile, 1);
                } catch (...) {
                    gui::application_release(entity);
                    throw;
                }
            }
            owner[owner[2]] = reinterpret_cast<Dword>(entity);
            at<void *>(reinterpret_cast<void *>(owner[owner[2]]), 228) = shape;
            at<Byte>(shape, 97) = 0;
            at<Byte>(shape, 98) = 1;
            ++owner[2];
        }
    }
}

void reset_spike_population(Dword *owner) {
    for (int i = 0; i < 2; ++i)
        if (void *entity = reinterpret_cast<void *>(owner[i])) {
            deactivate_spike_entity(entity);
            if (void *shape = at<void *>(entity, 228))
                if (bind_shape_body(shape, 1))
                    if (void *body = at<void *>(shape, 80)) {
                        at<void *>(entity, 224) = body;
                        at<Byte>(shape, 97) = 0;
                        at<Byte>(shape, 98) = 1;
                        at<Byte>(body, 728) = at<Byte>(body, 729) = 0;
                        invoke(entity, 6, Dword{0});
                    }
        }
}

void release_spike_population(Dword *owner) {
    for (int i = 1; i >= 0; --i)
        if (void *entity = reinterpret_cast<void *>(owner[i])) {
            deactivate_spike_entity(entity);
            owner[i] = 0;
        }
}
// Helicopter/projectile loader, formerly misnamed reset_car_contacts.
int load_helicopter_population(Dword *owner, int maximum) {
    if (maximum < 2)
        maximum = 2;
    const int requested = maximum;
    int remaining_projectiles = 5;
    owner[8] = owner[2] = 0;
    Dword cursor = 0;
    void *profile = find_profile(cursor, 7);
    Dword projectile_profile[2];
    projectile_profile[0] = reinterpret_cast<Dword>(projectile_profile_methods);
    const int count = at<int>(game::runtime::track_collision, 4);
    audio::vehicle::Progress progress;
    audio::vehicle::construct_progress(&progress, count);
    try {
        for (int i = 0; i < count && maximum > 0 && remaining_projectiles > 0; ++i) {
            audio::vehicle::advance_progress(&progress);
            void *shape = shape_at(i);
            const char *name = invoke<const char *>(shape, 4);
            if (!_stricmp(name, "helicopter")) {
                at<Byte>(shape, 97) = 0;
                at<Byte>(shape, 98) = 1;
                auto *entity = static_cast<Dword *>(
                    nfs::lifecycle::allocate(1716, "AIHelicopter", nfs::lifecycle::level_pool));
                if (entity) {
                    try {
                        entity = construct_helicopter_entity(entity, profile, 1);
                    } catch (...) {
                        game::runtime::track::pending::release_named(
                            entity, "AIHelicopter", nfs::lifecycle::level_pool);
                        throw;
                    }
                }
                owner[owner[2]] = reinterpret_cast<Dword>(entity);
                ++owner[2];
                at<void *>(entity, 368) = shape;
                --maximum;
                void *next = nullptr;
                if (cursor < state<Dword>(7353148)) {
                    ++cursor;
                    next = find_profile(cursor, 7);
                }
                if (!next) {
                    cursor = 0;
                    next = find_profile(cursor, 7);
                }
                profile = next;
            } else if (!_stricmp(name, "blades_main")) {
                attach_helicopter_blades(owner, shape, 372);
            } else if (!_stricmp(name, "blades_tail")) {
                attach_helicopter_blades(owner, shape, 376);
            } else if (!std::strncmp(name, "Tbarrel", 7)) {
                at<Byte>(shape, 97) = 0;
                at<Byte>(shape, 98) = 1;
                auto *entity = static_cast<Dword *>(nfs::lifecycle::allocate(
                    296, "AIHelicopterProjectile", nfs::lifecycle::level_pool));
                if (entity) {
                    try {
                        entity = construct_helicopter_projectile(entity, projectile_profile, 1);
                    } catch (...) {
                        game::runtime::track::pending::release_named(
                            entity, "AIHelicopterProjectile", nfs::lifecycle::level_pool);
                        throw;
                    }
                }
                const Dword index = owner[8];
                owner[3 + index] = reinterpret_cast<Dword>(entity);
                at<Dword>(entity, 228) = index;
                at<void *>(entity, 236) = shape;
                --remaining_projectiles;
                ++owner[8];
            }
        }
    } catch (...) {
        audio::vehicle::destroy_progress(&progress);
        throw;
    }
    audio::vehicle::destroy_progress(&progress);
    return requested - maximum;
}
} // namespace nfs::game::runtime::ai::lifecycle::pending
