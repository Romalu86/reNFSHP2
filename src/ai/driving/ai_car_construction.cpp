#include "ai_car_runtime.hpp"
namespace nfs::game::runtime::ai::cars {

Dword *construct_car_entity(Dword *entity, void *profile, int virtual_bases) {
    if (virtual_bases) {
        entity[1] = word_pointer(car_offsets);
        entity[141] = word_pointer(entities::entity_kind_base_methods);
        entity[143] = word_pointer(entities::entity_cast_base_methods);
        entity[145] = word_pointer(entities::body_base_methods);
    }
    void *profile_base = profile ? bytes(profile) + at<const int *>(profile, 0)[1] : nullptr;
    entities::construct_path_entity(entity, profile_base, 0);
    entity[89] = at<Dword>(profile, 148);
    const Dword profile_value = at<Dword>(profile, 156);
    entity[90] = at<Dword>(profile, 152);
    entity[93] = Dword(-1);
    entity[94] = Dword(-1);
    entity[103] = Dword(-1);
    entity[105] = Dword(-1);
    entity[106] = Dword(-1);
    entity[95] = 0;
    entity[96] = 0;
    entity[97] = 0;
    entity[91] = profile_value;
    entity[92] = 0;
    entity[98] = 0;
    entity[99] = 0;
    entity[100] = 0;
    entity[101] = 0;
    at<Byte>(entity, 408) = 0;
    at<Byte>(entity, 409) = 0;
    entity[104] = 1200;
    entity[139] = 0;
    entity[140] = 0;
    bind_tables(entity, car_methods, car_kind_methods, car_casts_methods, car_body_methods);
    set_displacement(entity, 2, 568);
    set_displacement(entity, 3, 576);
    at<Byte>(entity, 540) = 0;
    for (unsigned word = 0; word < 28; ++word)
        entity[107 + word] = 0;
    for (unsigned name = 0; name < 7; ++name)
        std::strcpy(reinterpret_cast<char *>(bytes(entity) + 428 + name * 16),
                    reinterpret_cast<const char *>(bytes(profile) + 4 + name * 16));
    at<Byte>(entity, 352) = 0;
    at<char>(entity, 320) = '\0';
    const Dword name_address = word_pointer(profile) + 116;
    if (name_address == 0)
        at<char>(entity, 320) = '\0';
    else
        std::strncpy(reinterpret_cast<char *>(bytes(entity) + 320),
                     reinterpret_cast<const char *>(name_address), 32);
    return entity;
}

Dword *construct_perpetrator(Dword *perpetrator, int virtual_bases) {
    if (virtual_bases) {
        perpetrator[1] = word_pointer(perpetrator_offsets);
        perpetrator[8] = word_pointer(entities::entity_cast_base_methods);
    }
    perpetrator[2] = 0;
    perpetrator[3] = 0;
    perpetrator[4] = 0;
    perpetrator[5] = 0;
    at<Byte>(perpetrator, 24) = 0;
    at<Byte>(perpetrator, 25) = 0;
    perpetrator[0] = word_pointer(perpetrator_methods);
    at<const Dword *>(entities::interface_at(perpetrator, 1), 0) = perpetrator_casts_methods;
    set_displacement(perpetrator, 1, 28);
    return perpetrator;
}

Dword *construct_cop_base(Dword *entity, void *profile, int virtual_bases) {
    if (virtual_bases) {
        entity[1] = word_pointer(cop_base_offsets);
        entity[152] = word_pointer(cop_base_perpetrator_offsets);
        entity[142] = word_pointer(entities::entity_kind_base_methods);
        entity[144] = word_pointer(entities::entity_cast_base_methods);
        entity[146] = word_pointer(entities::body_base_methods);
        construct_perpetrator(entity + 151, 0);
    }
    construct_car_entity(entity, profile, 0);
    entity[141] = 8;
    bind_tables(entity, cop_base_methods, cop_base_kind_methods, cop_base_casts_methods,
                cop_base_body_methods, cop_base_perpetrator_methods);
    set_displacement(entity, 2, 572);
    set_displacement(entity, 3, 580);
    set_displacement(entity, 4, 600);
    return entity;
}

Dword *construct_racer(Dword *entity, void *profile, int virtual_bases) {
    if (virtual_bases) {
        entity[1] = word_pointer(racer_offsets);
        entity[160] = word_pointer(racer_perpetrator_offsets);
        entity[150] = word_pointer(entities::entity_kind_base_methods);
        entity[152] = word_pointer(entities::entity_cast_base_methods);
        entity[154] = word_pointer(entities::body_base_methods);
        construct_perpetrator(entity + 159, 0);
    }
    construct_car_entity(entity, profile, 0);
    entity[141] = 3212836864u;
    entity[143] = 0;
    at<Byte>(entity, 580) = 0;
    entity[146] = 0;
    entity[148] = 0;
    at<Byte>(entity, 596) = 0;
    entity[142] = Dword(-1);
    bind_tables(entity, racer_methods, racer_kind_methods, racer_casts_methods, racer_body_methods,
                racer_perpetrator_methods);
    set_displacement(entity, 2, 604);
    set_displacement(entity, 3, 612);
    set_displacement(entity, 4, 632);
    return entity;
}
// Padding at +2 is not copied by the body.
void copy_racer_traits(void *traits, const void *source) {
    at<std::uint16_t>(traits, 0) = at<const std::uint16_t>(source, 0);
    at<Dword>(traits, 4) = at<const Dword>(source, 4);
    const Dword third = at<const Dword>(source, 8);
    const Dword fourth = at<const Dword>(source, 12);
    at<Dword>(traits, 8) = third;
    at<Dword>(traits, 12) = fourth;
}

Dword *construct_pursuit_car(Dword *entity, void *profile, int virtual_bases) {
    if (virtual_bases) {
        entity[1] = word_pointer(pursuit_offsets);
        entity[171] = word_pointer(pursuit_perpetrator_offsets);
        entity[161] = word_pointer(entities::entity_kind_base_methods);
        entity[163] = word_pointer(entities::entity_cast_base_methods);
        entity[165] = word_pointer(entities::body_base_methods);
        construct_perpetrator(entity + 170, 0);
        copy_racer_traits(entity + 177, profile ? bytes(profile) + 160 : nullptr);
    }
    construct_racer(entity, profile, 0);
    entity[150] = 1;
    entity[151] = 0;
    entity[152] = 0;
    entity[156] = 0;
    entity[153] = 0;
    entity[154] = 0;
    entity[155] = 0;
    bind_tables(entity, pursuit_methods, pursuit_kind_methods, pursuit_casts_methods,
                pursuit_body_methods, pursuit_perpetrator_methods);
    set_displacement(entity, 1, 640);
    set_displacement(entity, 2, 648);
    set_displacement(entity, 3, 656);
    set_displacement(entity, 4, 676);
    entity[157] = 0;
    entity[158] = 0;
    entity[159] = 0;
    return entity;
}

Dword *construct_player(Dword *entity, void *profile, int virtual_bases) {
    if (virtual_bases) {
        entity[1] = word_pointer(player_offsets);
        entity[172] = word_pointer(player_perpetrator_offsets);
        entity[162] = word_pointer(entities::entity_kind_base_methods);
        entity[164] = word_pointer(entities::entity_cast_base_methods);
        entity[166] = word_pointer(entities::body_base_methods);
        construct_perpetrator(entity + 171, 0);
        lifecycle::pending::initialize_racer_traits(entity + 178, 0, 2, 0, 0);
    }
    construct_pursuit_car(entity, profile, 0);
    bind_tables(entity, player_methods, player_kind_methods, player_casts_methods,
                player_body_methods, player_perpetrator_methods);
    set_displacement(entity, 1, 644);
    set_displacement(entity, 2, 652);
    set_displacement(entity, 3, 660);
    set_displacement(entity, 4, 680);
    return entity;
}

Dword *construct_ai_cop(Dword *entity, void *profile, int virtual_bases) {
    if (virtual_bases) {
        entity[1] = word_pointer(ai_cop_offsets);
        entity[178] = word_pointer(ai_cop_perpetrator_offsets);
        entity[168] = word_pointer(entities::entity_kind_base_methods);
        entity[170] = word_pointer(entities::entity_cast_base_methods);
        entity[172] = word_pointer(entities::body_base_methods);
        construct_perpetrator(entity + 177, 0);
        lifecycle::pending::initialize_racer_traits(entity + 184, 0, 2, 0, 0);
    }
    construct_pursuit_car(entity, profile, 0);
    at<Byte>(entity, 640) = 0;
    at<Byte>(entity, 656) = 1;
    entity[165] = 0;
    entity[166] = 0;
    bind_tables(entity, ai_cop_methods, ai_cop_kind_methods, ai_cop_casts_methods,
                ai_cop_body_methods, ai_cop_perpetrator_methods);
    set_displacement(entity, 1, 668);
    set_displacement(entity, 2, 676);
    set_displacement(entity, 3, 684);
    set_displacement(entity, 4, 704);
    entities::set_path_entity_active(entity, nullptr, 0);
    entity[101] = 4;
    entity[163] = 0;
    return entity;
}

Dword *construct_autonomous_cop(Dword *entity, void *profile, int virtual_bases) {
    if (virtual_bases) {
        entity[1] = word_pointer(autonomous_cop_offsets);
        entity[475] = word_pointer(autonomous_cop_perpetrator_offsets);
        entity[465] = word_pointer(entities::entity_kind_base_methods);
        entity[467] = word_pointer(entities::entity_cast_base_methods);
        entity[469] = word_pointer(entities::body_base_methods);
        construct_perpetrator(entity + 474, 0);
    }
    construct_cop_base(entity, profile, 0);
    entity[142] = 0;
    entity[143] = 0;
    at<Byte>(entity, 576) = 0;
    entity[145] = 0;
    at<Byte>(entity, 584) = 0;
    entity[147] = 0;
    entity[148] = 0;
    entity[149] = 0;
    at<std::uint16_t>(entity, 1800) = 0;
    entity[452] = 1;
    at<Byte>(entity, 1804) = 0;
    at<Byte>(entity, 1840) = 1;
    entity[453] = 0;
    entity[454] = 0;
    entity[455] = 0;
    entity[456] = 0;
    entity[457] = 0;
    entity[458] = 0;
    entity[459] = 0;
    entity[461] = Dword(-1);
    entity[462] = 0;
    entity[463] = 0;
    bind_tables(entity, autonomous_cop_methods, autonomous_cop_kind_methods,
                autonomous_cop_casts_methods, autonomous_cop_body_methods,
                autonomous_cop_perpetrator_methods);
    set_displacement(entity, 1, 1856);
    set_displacement(entity, 2, 1864);
    set_displacement(entity, 3, 1872);
    set_displacement(entity, 4, 1892);
    return entity;
}

Dword *construct_network_car(Dword *entity, void *profile, int virtual_bases) {
    if (virtual_bases) {
        entity[1] = word_pointer(network_car_offsets);
        entity[159] = word_pointer(network_car_perpetrator_offsets);
        entity[149] = word_pointer(entities::entity_kind_base_methods);
        entity[151] = word_pointer(entities::entity_cast_base_methods);
        entity[153] = word_pointer(entities::body_base_methods);
        construct_perpetrator(entity + 158, 0);
    }
    construct_cop_base(entity, profile, 0);
    entity[142] = 0;
    at<Byte>(entity, 572) = 0;
    at<Byte>(entity, 573) = 0;
    at<Byte>(entity, 574) = 0;
    entity[144] = 0;
    at<Byte>(entity, 580) = 0;
    at<Byte>(entity, 581) = 0;
    at<Byte>(entity, 582) = 0;
    at<Byte>(entity, 583) = 0;
    entity[146] = 3212836864u;
    bind_tables(entity, network_car_methods, network_car_kind_methods, network_car_casts_methods,
                network_car_body_methods, network_car_perpetrator_methods);
    set_displacement(entity, 1, 592);
    set_displacement(entity, 2, 600);
    set_displacement(entity, 3, 608);
    set_displacement(entity, 4, 628);
    entity[141] = 8;
    return entity;
}
} // namespace nfs::game::runtime::ai::cars
namespace nfs::game::runtime::ai::lifecycle::pending {

Dword *construct_traffic_car(Dword *entity, Dword profile_address, int virtual_bases) {
    void *profile = reinterpret_cast<void *>(profile_address);
    if (virtual_bases) {
        entity[1] = cars::word_pointer(cars::traffic_offsets);
        entity[148] = cars::word_pointer(entities::entity_kind_base_methods);
        entity[150] = cars::word_pointer(entities::entity_cast_base_methods);
        entity[152] = cars::word_pointer(entities::body_base_methods);
    }
    cars::construct_car_entity(entity, profile, 0);
    entity[143] = at<Dword>(profile, 164);
    entity[144] = at<Dword>(profile, 160);
    entity[141] = 0;
    entity[142] = 0;
    entity[145] = Dword(-1);
    entity[146] = 0;
    cars::bind_tables(entity, cars::traffic_methods, cars::traffic_kind_methods,
                      cars::traffic_casts_methods, cars::traffic_body_methods);
    cars::set_displacement(entity, 1, 588);
    cars::set_displacement(entity, 2, 596);
    cars::set_displacement(entity, 3, 604);
    return entity;
}
} // namespace nfs::game::runtime::ai::lifecycle::pending
