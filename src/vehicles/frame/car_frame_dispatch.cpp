#include "car_frame_runtime.hpp"
#include <cctype>
namespace nfs::game::runtime::ai::cars {
using namespace physics::car_frame;
// Active branches forward to the shared frame handler.
void dispatch_physical_car_frame(void *entity) {
    const Dword kind = invoke<Dword>(entities::interface_at(entity, 1), 0);
    if (kind == 6) {
        void *car = at<void *>(entity, 368);
        if (!car)
            return;
        const auto mode = at<std::int16_t>(car, 7940);
        if (mode != 3 && mode != 4) {
            if (mode == 6) {
                const Dword position = invoke<Dword>(entity, 29);
                invoke(entity, 16, at<Dword>(entity, 400), position);
            }
            update_kinematic_car_physics(car);
            return;
        }
        void *body = invoke<void *>(entities::interface_at(entity, 3), 0);
        if (body && at<Byte>(body, 728)) {
            update_active_car_physics(car);
            return;
        }
    } else {
        void *body = invoke<void *>(entities::interface_at(entity, 3), 0);
        if (body && at<Byte>(body, 728)) {
            update_active_car_physics(at<void *>(entity, 368));
            return;
        }
    }
    const Dword position = invoke<Dword>(entity, 29);
    invoke(entity, 16, at<Dword>(entity, 400), position);
}

Byte participant_inactive_racer(void *entity) {
    void *race = at<void *>(system(92), 4);
    const Byte active = invoke<Byte>(race, 29, at<Dword>(at<void *>(entity, 368), 1772));
    return Byte(1 - (active != 0));
}

void update_car_siren_control(void *entity, Dword *controls) {
    const Dword kind = invoke<Dword>(entities::interface_at(entity, 1), 0);
    void *car = at<void *>(entity, 368);
    if (kind == 6 && car && at<std::int16_t>(car, 7940) != 4)
        *controls = at<Byte>(car, 3476) & 1 ? *controls | 512 : *controls & ~512u;
    if (invoke<Dword>(entities::interface_at(entity, 2), 4) || invoke<Dword>(entity, 11))
        return;
    void *racer = invoke<void *>(entity, 9);
    if (racer && participant_inactive_racer(racer))
        return;
    Dword cursor = at<Dword>(entity, 556);
    if (!cursor)
        return;
    const Dword now = graphics::unresolved::camera_state;
    if (int(now) > int(at<Dword>(entity, 560) + cursor))
        return;
    for (Dword index = 0; int(cursor) < int(now); ++index) {
        const int duration = std::toupper(at<Byte>(entity, 540 + index));
        cursor += (Dword(duration) - 64u) * 5u;
        *controls = index & 1 ? *controls & ~512u : *controls | 512;
    }
}

void __fastcall car_collision(void *entity, void *, Dword skip_physics) {
    void *car = at<void *>(entity, 368);
    if (!car)
        return;
    at<Dword>(entity, 400) = 0;
    entities::update_entity_path_sample(entity);
    Dword controls[2];
    nfs::lifecycle::runtime::reset_replay_record(controls);
    controls[0] = 0;
    controls[1] = 0;
    update_car_siren_control(entity, controls);
    const Dword mode = at<Dword>(entity, 404);
    invoke(entity, mode == 4 ? 44 : mode == 5 ? 43 : 40, controls);
    services::input_events::dispatch_car_actions(services::pending::input_event, entity, controls);
    if (!Byte(skip_physics)) {
        const Dword kind = invoke<Dword>(entities::interface_at(entity, 1), 0);
        submit_nearby_audio_car(car, kind);
        dispatch_physical_car_frame(entity);
        update_car_statistics(car);
    }
}

void __fastcall network_car_collision(void *entity, void *, Dword skip_physics) {
    void *population = system(76);
    at<float>(entity, 584) = -1000000.f;
    const Dword index = invoke<Dword>(entity, 15);
    void *progress = bytes(population) + 4 + index * 42080;
    if (!(at<float>(progress, 16864) < 0)) {
        void *cursor = bytes(entity) + 236;
        void *section = lifecycle::path_current_section(cursor);
        const int sample = entities::path_sample_index(cursor);
        if (section && sample >= 0) {
            float *output = vector_at(entity, 584);
            const float *position = invoke<const float *>(entity, 0);
            sample_race_progress(progress, section, sample, position, output);
        }
    }
    car_collision(entity, nullptr, skip_physics);
}

void __fastcall player_collision(void *entity, void *, Dword skip_physics) {
    if (!at<void *>(entity, 368))
        return;
    update_racer_progress(entity);
    car_collision(entity, nullptr, skip_physics);
}
} // namespace nfs::game::runtime::ai::cars
