#include "ai_car_spawn_runtime.hpp"
#include "nfs/network/vehicles/network_snapshots.hpp"
#include <cstdlib>
namespace nfs::game::runtime::ai::cars {
namespace {

Dword *construct_ai_racer(Dword *entity, void *profile) {
    entity[1] = word_pointer(ai_racer_offsets);
    entity[162] = word_pointer(ai_racer_perpetrator_offsets);
    entity[152] = word_pointer(entities::entity_kind_base_methods);
    entity[154] = word_pointer(entities::entity_cast_base_methods);
    entity[156] = word_pointer(entities::body_base_methods);
    construct_perpetrator(entity + 161, 0);
    construct_racer(entity, profile, 0);
    at<Byte>(entity, 600) = 0;
    bind_tables(entity, ai_racer_methods, ai_racer_kind_methods, ai_racer_casts_methods,
                ai_racer_body_methods, ai_racer_perpetrator_methods);
    set_displacement(entity, 1, 604);
    set_displacement(entity, 2, 612);
    set_displacement(entity, 3, 620);
    set_displacement(entity, 4, 640);
    return entity;
}
} // namespace

void finalize_player_name_comparer() {
    network::players::destroy_slot(&lifecycle::state<network::players::Visitor>(7352960));
}

Dword register_population_cop(void *controller, void *entity) {
    const auto count = at<std::uint16_t>(controller, 80);
    if (count > 7)
        return 0;
    at<void *>(controller, Dword(count) * 4) = entity;
    ++at<std::uint16_t>(controller, 80);
    at<Byte>(entity, 576) = 0;
    at<Dword>(entity, 564) = count;
    return 1;
}

Dword register_special_population_cop(void *controller, void *entity) {
    const auto count = at<std::uint16_t>(controller, 82);
    if (count > 3)
        return 0;
    at<Dword>(entity, 564) = Dword(count) + 100000;
    at<void *>(controller, 32 + Dword(count) * 4) = entity;
    ++at<std::uint16_t>(controller, 82);
    at<Byte>(entity, 576) = 1;
    return 1;
}
} // namespace nfs::game::runtime::ai::cars
namespace nfs::game::runtime::ai::lifecycle::pending {

void spawn_racer(Dword *owner, Dword participant, int player, Dword model, Dword palette) {
    Dword profile_index = 0;
    Dword *profile = cars::next_profile(profile_index, 1);
    void *racer_profile = invoke<void *>(profile, 3);
    auto *memory = static_cast<Dword *>(gui::application_allocate(672));
    Dword *entity;
    try {
        entity = memory ? cars::construct_ai_racer(memory, racer_profile) : nullptr;
    } catch (...) {
        gui::application_release(memory);
        throw;
    }
    cars::append_race_entity(owner, entity);
    entity[89] = dependencies::validate_car_model(int(model));
    invoke(entity, 6, Dword{1});
    cars::configure_racer_spawn(owner, entity, participant, Dword(player), palette);
    cars::register_spawn(entity, participant);
}

void spawn_player(Dword *owner, Dword participant, Dword model, Dword palette) {
    if (!(state<Dword>(7352964) & 1)) {
        state<Dword>(7352964) |= 1;
        state<network::players::Visitor>(7352960).methods = network::players::slot_methods;
        std::atexit(cars::finalize_player_name_comparer);
    }
    Dword profile_index = 0;
    Dword *profile = cars::next_profile(profile_index, 6);
    void *player_profile = invoke<void *>(profile, 7);
    auto *memory = static_cast<Dword *>(gui::application_allocate(728));
    Dword *entity;
    try {
        entity = memory ? cars::construct_player(memory, player_profile, 1) : nullptr;
    } catch (...) {
        gui::application_release(memory);
        throw;
    }
    cars::append_race_entity(owner, entity);
    entity[89] = dependencies::validate_car_model(int(model));
    invoke(entity, 6, Dword{1});
    const char *model_name = dependencies::car_model_name(entity[89]);
    void *descriptor = participant < 24 ? bytes(system(72)) + 248 + participant * 84 : nullptr;
    Dword load = 0;
    if (at<int>(system(108), 44) != 1 || at<int>(descriptor, 68) == 1)
        load = 1;
    void *car = world::create_car(system(68), model_name, palette, cars::word_pointer(entity), 0,
                                  load, participant);
    void *body = at<void *>(car, 1768);
    const Dword *position = reinterpret_cast<const Dword *>(bytes(system(68)) + 700 +
                                                          at<Dword>(descriptor, 20) * 12);
    at<Dword>(body, 680) = position[0];
    at<Dword>(body, 684) = position[1];
    at<Dword>(body, 688) = position[2];
    at<Dword>(body, 8) = 0;
    at<Dword>(body, 12) = 0;
    at<Dword>(body, 16) = 0;
    at<Dword>(body, 20) = 0;
    at<Dword>(body, 24) = 0;
    at<Dword>(body, 28) = 0;
    at<Dword>(car, 3160) = 0;
    at<Dword>(car, 3164) = 0;
    at<Dword>(car, 3168) = 0;
    physics::refresh_car_track_point(car);
    void *renderable = at<void *>(car, 3372);
    entity[91] = renderable ? cars::renderable_palette_kind(renderable, palette) : 0;
    at<std::uint16_t>(entities::interface_at(entity, 5), 0) = 9;
    // The saved stack record has unwritten fields. Only its explicit stores are made here.
    alignas(4) Byte player_record[136];
    at<Dword>(player_record, 32) = 0;
    at<Dword>(player_record, 36) = 0;
    at<Dword>(player_record, 40) = 0;
    at<Dword>(player_record, 44) = 0;
    at<Dword>(player_record, 48) = 0;
    at<Dword>(player_record, 52) = Dword(-1);
    at<Dword>(player_record, 88) = 1065353216;
    at<Dword>(player_record, 92) = 0;
    at<Dword>(player_record, 96) = 0;
    at<Dword>(player_record, 100) = 0;
    network::snapshots::clear_vector3(bytes(player_record) + 104);
    const Dword key = participant;
    at<Byte>(player_record, 56) = 0;
    Dword *queue = reinterpret_cast<Dword *>(entities::interface_at(system(104), 1));
    if (nfs::lifecycle::construction::network::queue::copy_matching(
            queue, cars::word_pointer(&key), cars::word_pointer(player_record),
            &state<Dword>(7352960)))
        std::strncpy(reinterpret_cast<char *>(entity + 80),
                     reinterpret_cast<const char *>(player_record + 4), 32);
    cars::register_spawn(entity, participant);
}

void spawn_remote_car(Dword *owner, Dword participant, int player, Dword model, int, Byte special) {
    Dword *entity;
    if (player == -1) {
        Dword profile_index = 0;
        cars::next_profile(profile_index, 3);
        profile_index = 0;
        Dword *profile = cars::next_profile(profile_index, 3);
        Dword count = 0;
        while (profile) {
            if (profile_index < state<Dword>(7353148)) {
                ++profile_index;
                profile = cars::next_profile(profile_index, 3);
            } else
                profile = nullptr;
            ++count;
        }
        const Dword random = emitter_random_word();
        profile_index = 0;
        profile = cars::next_profile(profile_index, 3);
        for (Dword ordinal = 0; ordinal < random % count; ++ordinal) {
            if (!profile)
                break;
            if (profile_index < state<Dword>(7353148)) {
                ++profile_index;
                profile = cars::next_profile(profile_index, 3);
            } else
                profile = nullptr;
        }
        void *cop_profile = invoke<void *>(profile, 5);
        auto *memory = static_cast<Dword *>(gui::application_allocate(1924));
        try {
            entity = memory ? cars::construct_autonomous_cop(memory, cop_profile, 1) : nullptr;
        } catch (...) {
            gui::application_release(memory);
            throw;
        }
    } else {
        Dword profile_index = 0;
        Dword *profile = cars::next_profile(profile_index, 2);
        void *network_profile = invoke<void *>(profile, 4);
        auto *memory = static_cast<Dword *>(gui::application_allocate(660));
        try {
            entity = memory ? cars::construct_network_car(memory, network_profile, 1) : nullptr;
        } catch (...) {
            gui::application_release(memory);
            throw;
        }
    }
    entity[89] = dependencies::validate_car_model(int(model));
    entity[90] = Dword(dependencies::validate_skin_variant(15));
    if (player == -1) {
        const char *model_name = dependencies::car_model_name(entity[89]);
        void *car = world::create_car(system(68), model_name, entity[90],
                                      cars::word_pointer(entity), 3, 1, Dword(-1));
        if (!special)
            cars::register_population_cop(ai::pending::path_controller, entity);
        else
            cars::register_special_population_cop(ai::pending::path_controller, entity);
        at<float>(car, 564) = float(
            kernel::numeric::Extended(at<float>(car, 520), kernel::numeric::current_format()) *
            0.01f);
    } else {
        cars::configure_racer_spawn(owner, entity, participant, Dword(player), 15);
        invoke(entity, 6, Dword{1});
    }
    cars::register_spawn(entity, participant);
}
} // namespace nfs::game::runtime::ai::lifecycle::pending
