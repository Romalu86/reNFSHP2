#include "ai_car_spawn_runtime.hpp"
#include "../../race/rules/race_rules_x87.hpp"
namespace nfs::game::runtime::ai::cars {

Dword body_palette_kind(void *body, Dword palette) {
    if (palette == 16)
        return at<Dword>(body, 1028);
    if (palette == 15)
        return at<Dword>(body, 1080);
    return at<Dword>(body, 64 + palette * 48);
}

Dword renderable_palette_kind(void *renderable, Dword palette) {
    void *body = bytes(system(52)) + at<Dword>(renderable, 3952) * 1228 + 8;
    return body_palette_kind(body, palette);
}

Dword choose_body_palette(void *body, Dword kind) {
    const Dword count = at<Dword>(body, 60);
    Dword matches = 0;
    for (int index = 0; index < std::int32_t(count); ++index)
        if (at<Dword>(body, 64 + Dword(index) * 48) == kind)
            ++matches;
    if (std::int32_t(matches) > 1) {
        const Dword random = emitter_random_word();
        Dword ordinal = 0;
        for (int index = 0; index < std::int32_t(count); ++index) {
            if (at<Dword>(body, 64 + Dword(index) * 48) == kind) {
                if (random % matches == ordinal)
                    return Dword(index);
                ++ordinal;
            }
        }
    }
    for (int index = 0; index < std::int32_t(count); ++index)
        if (at<Dword>(body, 64 + Dword(index) * 48) == kind)
            return Dword(index);
    return kind == at<Dword>(body, 1028) ? 16 : emitter_random_word() % count;
}
namespace {
void copy_grid_position(void *car, Dword participant) {
    void *record = participant < 24 ? bytes(system(72)) + 248 + participant * 84 : nullptr;
    const Dword start_slot = at<Dword>(record, 20);
    const Dword *position =
        reinterpret_cast<const Dword *>(bytes(system(68)) + 700 + start_slot * 12);
    void *body = at<void *>(car, 1768);
    at<Dword>(body, 680) = position[0];
    const Dword position_y = position[1];
    const Dword position_z = position[2];
    at<Dword>(body, 684) = position_y;
    at<Dword>(body, 688) = position_z;
}
} // namespace

void configure_racer_spawn(Dword *owner, Dword *entity, Dword participant, Dword player, Dword palette) {
    Dword *race_settings = system(72);
    invoke(entity, 6, Dword{1});
    Dword car_slot;
    if (!network::session_active()) {
        car_slot = participant;
        if (std::int32_t(player) >= 0 && player < 2)
            at<Dword>(race_settings, 204 + player * 4) = participant;
    } else
        car_slot = at<Dword>(race_settings, 204 + player * 4);
    for (unsigned local = 0; local < 2; ++local)
        if (participant == at<Dword>(race_settings, 204 + local * 4))
            at<Dword *>(owner, 84164 + local * 4) = entity;
    void *preferences = player < 2 ? bytes(race_settings) + 2488 + player * 50088 : nullptr;
    const char *model = dependencies::car_model_name(entity[89]);
    const Dword load = (car_slot & 4294967040u) | Dword(at<int>(preferences, 52) == 0);
    void *car = world::create_car(system(68), model, palette, word_pointer(entity),
                                  Dword(palette == 16), load, car_slot);
    entity[91] = renderable_palette_kind(at<void *>(car, 3372), palette);
    copy_grid_position(car, participant);
    physics::refresh_car_track_point(car);
}

void configure_cop_spawn(Dword *owner, Dword *entity, int count, Dword participant,
                         float *difficulty) {
    const char *model = dependencies::car_model_name(entity[89]);
    invoke(entity, 6, Dword{1});
    append_race_entity(owner, entity);
    void *car =
        world::create_car(system(68), model, entity[90], word_pointer(entity), 2, 1, Dword(-1));
    copy_grid_position(car, participant);
    physics::refresh_car_track_point(car);
    const Dword palette_kind = renderable_palette_kind(at<void *>(car, 3372), entity[90]);
    const long double initial_difficulty = static_cast<long double>(*difficulty);
    entity[91] = palette_kind;
    const Dword converted = application::race_rules_x87::truncate(initial_difficulty * 9.f + 1.f);
    int skill = static_cast<std::int32_t>(converted);
    if (skill < 1)
        skill = 1;
    else if (skill > 10)
        skill = 10;
    at<std::int16_t>(entities::interface_at(entity, 5), 0) = std::int16_t(skill);
    if (count > 1)
        *difficulty = float(static_cast<long double>(*difficulty) -
                            1.f / static_cast<long double>(count - 1));
}
} // namespace nfs::game::runtime::ai::cars
