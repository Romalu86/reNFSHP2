#include "nfs/core/lifecycle/scope_cleanup.hpp"
#include "ai_entity_runtime.hpp"
namespace nfs::game::runtime::ai::entities {
namespace {
void bind_entity_tables(void *entity, const Dword *methods, const Dword *kind, const Dword *casts,
                        const Dword *body, unsigned base = 4) {
    at<const Dword *>(entity, 0) = methods;
    at<const Dword *>(interface_at(entity, 1, base), 0) = kind;
    at<const Dword *>(interface_at(entity, 2, base), 0) = casts;
    if (body)
        at<const Dword *>(interface_at(entity, 3, base), 0) = body;
}
void set_displacement(void *entity, unsigned base, unsigned slot, int size) {
    const int displacement = at<const int *>(entity, base)[slot];
    at<int>(entity, base + displacement - 4) = displacement - size;
}
void initialize_path_tables(void *entity) {
    bind_entity_tables(entity, path_entity_methods, path_kind_methods, path_cast_methods,
                       path_body_methods);
    set_displacement(entity, 4, 2, 324);
    set_displacement(entity, 4, 3, 332);
}
void initialize_flight_tables(void *entity) {
    bind_entity_tables(entity, flight_methods, flight_body_methods, flight_kind_methods,
                       flight_cast_methods, 320);
    set_displacement(entity, 320, 1, 12);
    set_displacement(entity, 320, 3, 36);
}
void initialize_helicopter_tables(void *entity) {
    bind_entity_tables(entity, helicopter_methods, helicopter_body_methods, helicopter_kind_methods,
                       helicopter_cast_methods, 320);
    set_displacement(entity, 320, 1, 1364);
    set_displacement(entity, 320, 2, 1384);
    set_displacement(entity, 320, 3, 1392);
}
void initialize_shape_tables(void *entity) {
    bind_entity_tables(entity, shape_entity_methods, shape_kind_methods, shape_cast_methods,
                       shape_body_methods);
    set_displacement(entity, 4, 3, 236);
}
} // namespace The constructor does not consume the profile argument.
 
Dword *construct_entity(Dword *entity, void *, int virtual_bases) {
    if (virtual_bases) {
        entity[1] = reinterpret_cast<Dword>(entity_offsets);
        entity[55] = reinterpret_cast<Dword>(entity_kind_base_methods);
        entity[56] = reinterpret_cast<Dword>(entity_cast_base_methods);
    }
    at<Byte>(entity, 212) = 0;
    bind_entity_tables(entity, entity_methods, entity_kind_methods, entity_cast_methods, nullptr);
    Dword *registry = network::world::vehicle_registry;
    if (std::int32_t(*registry) > 49)
        entity[54] = Dword(-1);
    else {
        registry[*registry + 1] = reinterpret_cast<Dword>(entity);
        const Dword index = *registry;
        *registry = index + 1;
        entity[54] = index;
    }
    entity[2] = 0;
    return entity;
}
// The base destructor changes tables; it does not unregister the entity.
void destroy_entity(void *entity) {
    bind_entity_tables(entity, entity_methods, entity_kind_methods, entity_cast_methods, nullptr);
}

Dword *construct_path_entity(Dword *entity, void *profile, int virtual_bases) {
    if (virtual_bases) {
        entity[1] = reinterpret_cast<Dword>(path_entity_offsets);
        entity[80] = reinterpret_cast<Dword>(entity_kind_base_methods);
        entity[82] = reinterpret_cast<Dword>(entity_cast_base_methods);
        entity[84] = reinterpret_cast<Dword>(body_base_methods);
    }
    construct_entity(entity, profile, 0);
    try {
        lifecycle::pending::construct_population_path(bytes(entity) + 220);
    } catch (...) {
        destroy_entity(entity);
        throw;
    }
    entity[77] = entity[78] = entity[79] = 0;
    initialize_path_tables(entity);
    return entity;
}

void destroy_path_entity(void *entity) {
    initialize_path_tables(entity);
    kernel::lifecycle::Cleanup entity_cleanup([=] { destroy_entity(entity); });
    lifecycle::pending::destroy_population_path(bytes(entity) + 220);
}

void destroy_flight_entity(void *entity) {
    initialize_path_tables(entity);
    kernel::lifecycle::Cleanup entity_cleanup([=] { destroy_entity(entity); });
    lifecycle::pending::destroy_population_path(bytes(entity) + 220);
}

void destroy_helicopter(void *entity) {
    initialize_helicopter_tables(entity);
    initialize_path_tables(entity);
    kernel::lifecycle::Cleanup entity_cleanup([=] { destroy_entity(entity); });
    lifecycle::pending::destroy_population_path(bytes(entity) + 220);
}

void destroy_shape_entity(void *entity) {
    destroy_entity(entity);
}

void destroy_projectile(void *entity) {
    destroy_entity(entity);
}

void destroy_spikes(void *entity) {
    destroy_entity(entity);
}
Dword *construct_helicopter(Dword *entity, void *profile, int virtual_bases) {
    if (virtual_bases) {
        entity[80] = reinterpret_cast<Dword>(flight_offsets);
        entity[1] = reinterpret_cast<Dword>(helicopter_offsets);
        entity[421] = reinterpret_cast<Dword>(body_base_methods);
        entity[426] = reinterpret_cast<Dword>(entity_kind_base_methods);
        entity[428] = reinterpret_cast<Dword>(entity_cast_base_methods);
    }
    entity[81] = 0;
    at<const Dword *>(interface_at(entity, 1, 320), 0) = flight_body_base_methods;
    set_displacement(entity, 320, 1, 12);
    construct_path_entity(entity, profile, 0);
    initialize_flight_tables(entity);
    entity[82] = at<Dword>(profile ? bytes(profile) + 8 : nullptr, 0);
    entity[89] = entity[90] = entity[91] = 0;
    entity[96] = entity[97] = entity[98] = 0;
    entity[83] = 1;
    entity[86] = 0;
    at<Byte>(entity, 348) = 0;
    at<Byte>(entity, 349) = 0;
    entity[88] = entity[93] = entity[94] = entity[95] = entity[99] = 0;
    entity[100] = at<Dword>(profile, 8);
    entity[103] = 0;
    entity[104] = 0;
    at<Byte>(entity, 420) = 0;
    at<Byte>(entity, 421) = 0;
    at<Byte>(entity, 422) = 0;
    entity[106] = entity[107] = entity[108] = entity[109] = entity[110] = 0;
    entity[111] = Dword(-1);
    at<std::uint16_t>(entity, 1648) = 0;
    entity[413] = entity[414] = 0;
    at<Byte>(entity, 1660) = 0;
    entity[416] = 0;
    initialize_helicopter_tables(entity);
    entity[417] = 0;
    entity[418] = 3212836864u;
    entity[419] = 0;
    return entity;
}
Dword *construct_projectile(Dword *entity, void *profile, int virtual_bases) {
    if (virtual_bases) {
        entity[1] = reinterpret_cast<Dword>(projectile_offsets);
        entity[55] = reinterpret_cast<Dword>(projectile_flight_offsets);
        entity[67] = reinterpret_cast<Dword>(entity_kind_base_methods);
        entity[68] = reinterpret_cast<Dword>(entity_cast_base_methods);
        entity[70] = reinterpret_cast<Dword>(body_base_methods);
    }
    construct_entity(entity, profile, 0);
    entity[56] = 0;
    at<const Dword *>(interface_at(entity, 1, 220), 0) = flight_body_base_methods;
    set_displacement(entity, 220, 1, 12);
    initialize_shape_tables(entity);
    entity[57] = 0;
    at<Byte>(entity, 232) = 0;
    entity[59] = entity[60] = entity[61] = entity[62] = entity[63] = entity[64] = 0;
    at<Byte>(entity, 260) = 0;
    bind_entity_tables(entity, projectile_methods, projectile_kind_methods, projectile_cast_methods,
                       projectile_body_methods);
    set_displacement(entity, 4, 1, 264);
    set_displacement(entity, 4, 3, 276);
    return entity;
}
Dword *construct_spikes(Dword *entity, void *profile, int virtual_bases) {
    if (virtual_bases) {
        entity[1] = reinterpret_cast<Dword>(spike_offsets);
        entity[55] = reinterpret_cast<Dword>(spike_flight_offsets);
        entity[73] = reinterpret_cast<Dword>(entity_kind_base_methods);
        entity[74] = reinterpret_cast<Dword>(entity_cast_base_methods);
        entity[76] = reinterpret_cast<Dword>(body_base_methods);
    }
    construct_entity(entity, profile, 0);
    entity[56] = 0;
    at<const Dword *>(interface_at(entity, 1, 220), 0) = flight_body_base_methods;
    set_displacement(entity, 220, 1, 12);
    initialize_shape_tables(entity);
    entity[57] = 0;
    at<Byte>(entity, 232) = 0;
    at<Byte>(entity, 233) = 0;
    entity[71] = 0;
    bind_entity_tables(entity, spike_methods, spike_kind_methods, spike_cast_methods,
                       spike_body_methods);
    set_displacement(entity, 4, 1, 288);
    set_displacement(entity, 4, 3, 300);
    for (unsigned i = 59; i < 71; ++i)
        entity[i] = 0;
    return entity;
}
} // namespace nfs::game::runtime::ai::entities
namespace nfs::game::runtime::ai::lifecycle::pending {

Dword *construct_helicopter_entity(Dword *entity, void *profile, int virtual_bases) {
    return entities::construct_helicopter(entity, profile, virtual_bases);
}

Dword *construct_helicopter_projectile(Dword *entity, void *profile, int virtual_bases) {
    return entities::construct_projectile(entity, profile, virtual_bases);
}

Dword *construct_spike_entity(Dword *entity, void *profile, int virtual_bases) {
    return entities::construct_spikes(entity, profile, virtual_bases);
}
} // namespace nfs::game::runtime::ai::lifecycle::pending
