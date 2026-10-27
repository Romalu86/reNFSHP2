#include "nfs/physics/solver/solver_runtime.hpp"
#include "nfs/graphics/resources/eagl_assets.hpp"
#include "nfs/core/lifecycle/system_runtime.hpp"
#include "../../math/precision/retained_particle.hpp"
namespace nfs::game::runtime::particles {

Dword *construct_pool(Dword *pool, Dword image, Dword flags) {
    at<volatile Dword>(pool, 400) = 0;
    at<volatile Dword>(pool, 804) = 0;
    at<volatile Dword>(pool, 808) = image;
    at<volatile Dword>(pool, 812) = flags;
    return pool;
}
namespace pending {

Dword *construct_type(Dword *object) {
    at<Dword>(object, 196) = 0;
    return object;
}

Dword *construct_emitter(Dword *object) {
    at<const float *>(object, 32) = saved::zero_vector;
    at<const float *>(object, 36) = saved::zero_vector;
    at<const float *>(object, 40) = saved::identity_matrix;
    at<Dword>(object, 104) = 0;
    constexpr Dword unit_time_scale_bits = 1065353216;
    at<volatile Dword>(object, 228) = unit_time_scale_bits;
    at<volatile Byte>(object, 232) = 0;
    at<volatile Byte>(object, 0) = 0;
    at<volatile Byte>(object, 233) = 0;
    return object;
}
// The complete destructor is RET.
void destroy_emitter(Dword *) {
}

void remove_type_particles(Dword *pool, Dword *type) {
    int index = 0;
    while (index < at<int>(pool, 72000)) {
        Dword *particle = at<Dword *>(pool, 68000 + index * 4);
        if (at<Dword *>(particle, 60) == type) {
            const int last = --at<int>(pool, 72000);
            if (index != last) {
                at<Dword *>(pool, 68000 + index * 4) = at<Dword *>(pool, 68000 + last * 4);
                at<Dword *>(pool, 68000 + last * 4) = particle;
            }
        } else
            ++index;
    }
}

void destroy_type(Dword *object) {
    Dword *manager = lifecycle::runtime::particle_owner;
    if (at<Dword>(manager, 4))
        remove_type_particles(at<Dword *>(manager, 4), object);
    if (Dword *image = at<Dword *>(object, 196)) {
        graphics::tar::destroy(image);
        kernel::memory_free(image);
        at<Dword>(object, 196) = 0;
    }
}

Dword *allocate_type(Dword *pool) {
    if (at<int>(pool, 400) > 99)
        return nullptr;
    Dword *object =
        static_cast<Dword *>(lifecycle::allocate(340, "particleEffect", at<Dword>(pool, 812)));
    try {
        if (object)
            construct_type(object);
    } catch (...) {
        track::pending::release_named(object, "particleEffect", at<Dword>(pool, 812));
        throw;
    }
    at<Dword *>(pool, at<Dword>(pool, 400) * 4) = object;
    ++at<Dword>(pool, 400);
    return object;
}

Dword *allocate_emitter(Dword *pool) {
    if (at<int>(pool, 804) > 99)
        return nullptr;
    Dword *object =
        static_cast<Dword *>(lifecycle::allocate(256, "particleEmitter", at<Dword>(pool, 812)));
    try {
        if (object)
            construct_emitter(object);
    } catch (...) {
        track::pending::release_named(object, "particleEmitter", at<Dword>(pool, 812));
        throw;
    }
    at<Dword *>(pool, 404 + at<Dword>(pool, 804) * 4) = object;
    ++at<Dword>(pool, 804);
    return object;
}

Dword *emitter_type(Dword *object, int index) {
    return at<Dword *>(object, 128 + index * 24);
}
// Only AL is consumed by the callers.
Byte attach_type(Dword *emitter, Dword *type) {
    Dword count = at<Dword>(emitter, 104);
    if (static_cast<int>(count) > 4)
        return 0;
    Byte *clock = bytes(emitter) + 108 + count * 24;
    at<Dword>(emitter, 104) = count + 1;
    at<Dword *>(clock, 20) = type;
    at<volatile float>(clock, 4) = float(retained_emitter_period(type));
    at<volatile Dword>(clock, 0) = 0;
    const Dword value = emitter_random_word();
    const long double fraction = static_cast<long double>(value & 65535);
    at<volatile Dword>(clock, 8) = 0;
    at<volatile Byte>(clock, 16) = 1;
    at<volatile float>(clock, 12) =
        float(fraction * 0.0000152587890625f * at<const volatile float>(type, 12) +
              at<const volatile float>(type, 4));
    return 1;
}
// Two passes over the MPB7 variable-length emitter records.
void load_pool_entry(Dword *pool, Dword *data) {
    if (data[0] != 7)
        return;
    int counts[100], indices[100];
    Dword *start = data + data[1] * 31 + 3, *record = start;
    for (int i = 0; i < static_cast<int>(data[2]) && i < 100; ++i) {
        Dword *links = record + 26;
        if (!record[24] && !record[25])
            indices[i] = -1;
        else {
            indices[i] = at<int>(pool, 804);
            counts[i] = static_cast<int>(record[24]);
            Dword *emitter = allocate_emitter(pool);
            initialize_emitter(emitter, record);
            for (Dword j = 0; j < record[24]; ++j) {
                Dword definition = *links++;
                Dword *type = allocate_type(pool);
                initialize_type(type, record, data + definition * 31 + 3, pool);
                prepare_type(type);
                attach_type(emitter, type);
            }
            links += record[25];
        }
        record = links;
    }
    record = start;
    const int record_count = static_cast<int>(data[2]);
    for (int i = 0; i < record_count; ++i) {
        if (i > 99)
            return;
        Dword *links = record + 26;
        if (indices[i] != -1) {
            Dword *emitter = at<Dword *>(pool, 404 + indices[i] * 4);
            links += record[24];
            const Dword linked_emitter_count = record[25];
            for (Dword j = 0; j < linked_emitter_count; ++j) {
                Dword other = *links++;
                if (indices[other] != -1) {
                    Dword *source = at<Dword *>(pool, 404 + indices[other] * 4);
                    for (int k = 0; k < counts[other]; ++k)
                        attach_type(emitter, emitter_type(source, k));
                }
            }
        }
        record = links;
    }
}
} // namespace pending Counts are deliberately not reset by the destructor.
 
void destroy_pool(Dword *pool) {
    for (int i = 0; i < at<int>(pool, 804); ++i) {
        Dword *object = at<Dword *>(pool, 404 + i * 4);
        pending::destroy_emitter(object);
        kernel::memory_free(object);
        at<Dword>(pool, 404 + i * 4) = 0;
    }
    for (int i = 0; i < at<int>(pool, 400); ++i) {
        Dword *object = at<Dword *>(pool, i * 4);
        pending::destroy_type(object);
        kernel::memory_free(object);
        at<Dword>(pool, i * 4) = 0;
    }
}

void enable_emitters(Dword *pool) {
    const int count = at<int>(pool, 804);
    for (int i = 0; i < count; ++i)
        at<Byte>(at<void *>(pool, 404 + i * 4), 232) = 1;
}
} // namespace nfs::game::runtime::particles
