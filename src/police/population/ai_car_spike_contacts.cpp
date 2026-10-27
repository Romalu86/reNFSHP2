#include "../../ai/driving/ai_car_behavior.hpp"
#include "../../graphics/vehicles/car_wheel_effects_runtime.hpp"
namespace nfs::game::runtime::race_audio::pending {
void update_stream_emitter(const void *);
}
namespace nfs::game::runtime::ai::cars {

void puncture_wheel(void *damage, Byte wheel) {
    const Byte mask = at<Byte>(damage, 0);
    if (!mask)
        at<Dword>(damage, 4) = graphics::unresolved::camera_state;
    at<Byte>(damage, 0) = Byte(1u << (wheel & 31)) | mask;
}
// By-value position and velocity arguments form a 48-byte emitter.
void play_car_contact_sound(Dword flags, Dword identifier, Dword event, Dword radius,
                            Dword maximum_volume, Dword minimum_volume, ContactSoundVector velocity,
                            ContactSoundVector position) {
    Dword emitter[12];
    emitter[7] = flags;
    emitter[6] = identifier;
    emitter[8] = event;
    emitter[9] = radius;
    emitter[5] = at<Dword>(&velocity, 8);
    emitter[3] = at<Dword>(&velocity, 0);
    emitter[4] = at<Dword>(&velocity, 4);
    emitter[2] = at<Dword>(&position, 8);
    emitter[0] = at<Dword>(&position, 0);
    emitter[1] = at<Dword>(&position, 4);
    emitter[10] = maximum_volume;
    emitter[11] = minimum_volume;
    race_audio::pending::update_stream_emitter(emitter);
}

Byte apply_spike_strip_hits(void *entity) {
    if (invoke<Dword>(entities::interface_at(entity, 1), 0) == 1 &&
        !at<void *>(entities::interface_at(entity, 4), 8))
        return 0;
    Byte hit = 0;
    if (!at<Byte>(entity, 212))
        return hit;
    void *car = at<void *>(entity, 368);
    for (int index = 0; index < contact_entity_count(entity); ++index) {
        void *spike = spike_entity(at<void *>(entity, 12 + Dword(index) * 4));
        if (!spike || !at<Byte>(spike, 212))
            continue;
        const Dword wheels = spike_wheel_hit_mask(spike, entity);
        if (!wheels)
            continue;
        hit = 1;
        at<Byte>(entity, 580) = 1;
        if (!(wheels & 9))
            at<Dword>(entity, 588) = 1;
        else
            at<Dword>(entity, 588) = 0;
        at<Dword>(entity, 592) = graphics::unresolved::camera_state;
        for (Byte wheel = 0; wheel < 4; ++wheel) {
            if (!(wheels & (1u << wheel)))
                continue;
            void *damage = bytes(car) + 7952;
            if (!physics::contacts_pending::wheel_is_punctured(damage, wheel)) {
                // The call snapshots both vectors as twelve stack arguments.
                ContactSoundVector position, velocity;
                std::memcpy(&position, bytes(at<void *>(car, 1768)) + 680, 12);
                std::memcpy(&velocity, bytes(car) + 3160, 12);
                play_car_contact_sound(2564, 0, 4, 25, 127, 127, velocity, position);
            }
            puncture_wheel(damage, wheel);
        }
    }
    return hit;
}
} // namespace nfs::game::runtime::ai::cars
