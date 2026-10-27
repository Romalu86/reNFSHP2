#include "../../ai/driving/ai_car_spawn_runtime.hpp"
#include <stdexcept>

namespace nfs::game::runtime::ai::lifecycle::pending {
namespace {
void *select_spawn_profile(Dword requested_rank, Dword profile_kind, unsigned data_slot) {
    Dword cursor = 0;
    Dword *entry = cars::next_profile(cursor, profile_kind);
    if (!entry) {
        // Report missing profiles before reading profile data.
         
        throw std::runtime_error(profile_kind == 0
                                     ? "AI car loading failed: no racer profile is available"
                                     : "AI car loading failed: no perpetrator profile is available");
    }

    void *selected;
    for (;;) {
        selected = invoke<void *>(entry, data_slot);
        void *identity = entities::interface_at(selected, 1, 0);
        if (invoke<Dword>(identity, 12) == requested_rank)
            return selected;

        if (cursor >= state<Dword>(7353148))
            break;
        ++cursor;
        entry = cars::next_profile(cursor, profile_kind);
        if (!entry)
            break;
    }

    // No-rank-match paths retain the LAST visited profile data. Their second catalogue scan still
    // performs its virtual kind queries.
    cursor = 0;
    cars::next_profile(cursor, profile_kind);
    return selected;
}
} // namespace

 
void spawn_cop(Dword *owner, Dword participant, Dword rank, Dword model, Byte has_remote,
               Byte special_racer) {
    void *body_definition = bytes(system(52)) + model * 1228 + 8;
    Dword *entity;
    Dword palette;
    if (!has_remote) {
        void *profile = select_spawn_profile(rank, 0, 2);
        auto *memory = static_cast<Dword *>(gui::application_allocate(724));
        try {
            entity = memory ? cars::construct_pursuit_car(memory, profile, 1) : nullptr;
        } catch (...) {
            gui::application_release(memory);  
            throw;
        }
        const Dword colour = at<Dword>(entities::interface_at(entity, 5), 4);
        palette = cars::choose_body_palette(body_definition, colour);
        if (palette == 16 && (model == 14 || model == 23 || !special_racer))
            palette = 0;
    } else {
        void *profile = select_spawn_profile(rank, 5, 6);
        auto *memory = static_cast<Dword *>(gui::application_allocate(752));
        try {
            entity = memory ? cars::construct_ai_cop(memory, profile, 1) : nullptr;
        } catch (...) {
            gui::application_release(memory);  
            throw;
        }
        palette = emitter_random_word() % at<Dword>(body_definition, 60);
        if (palette == 16 && (model == 14 || model == 23))
            palette = 0;
    }

    const auto skill = at<std::uint16_t>(entities::interface_at(entity, 5), 0);
    entity[90] = Dword(dependencies::validate_skin_variant(int(palette)));
    entity[89] = dependencies::validate_car_model(int(model));
    void *descriptor =
        participant < 24 ? bytes(system(72)) + participant * 84 + 248 : nullptr;
    at<Dword>(descriptor, 64) = palette;
    invoke(entity, 6, Dword{1});
    float difficulty = 1;
    cars::configure_cop_spawn(owner, entity, 1, participant, &difficulty);
    at<std::uint16_t>(entities::interface_at(entity, 5), 0) = skill;

    void *race = at<void *>(system(92), 4);
    void *identity = entities::interface_at(entity, 1);
    const char *model_name = dependencies::car_model_name(entity[89]);
    const char *name = reinterpret_cast<const char *>(bytes(entity) + 320);
    const Dword kind = invoke<Dword>(identity, 0);
    cars::register_spawned_participant(race, participant, kind, name, model_name, 0, 0);
}
} // namespace nfs::game::runtime::ai::lifecycle::pending
