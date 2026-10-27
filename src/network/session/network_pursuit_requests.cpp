#include "../../police/pursuit/police_chase_x87.hpp"
#include "network_pursuit_runtime.hpp"
namespace nfs::game::runtime::ai::cars {
using namespace lifecycle::pending;

Byte request_network_helicopter(void *entity) {
    void *chase = network_chase(entity);
    if (!chase)
        return 0;
    int listener = pursuit_listener(chase);
    if (listener == -1)
        listener = 0;
    if (!at<Dword>(chase, 48)) {
        if (!at<Byte>(system(84), 92 + listener))
            speech::game::dispatch_police_event(system(84), 13, listener, 0, 0, 0, 0, 0, 0, 2, 0,
                                                0);
        const Byte admitted = record_support_request(at<void *>(system(92), 4),
                                                     network::world::vehicle_slot(entity), 7);
        if (admitted) {
            void *helicopter = reserve_chase_helicopter(ai::collision_state());
            if (helicopter) {
                chase = network_chase(entity);
                bind_chase_helicopter(chase, helicopter);
                at<Byte>(chase, 20) = 1;
                at<Byte>(entity, 572) = 1;
                at<Dword>(entity, 576) = now();
                return 1;
            }
            release_support_request(at<void *>(system(92), 4),
                                    network::world::vehicle_slot(entity), 7);
        }
        if (!at<Byte>(system(84), 92 + listener)) {
            speech::game::dispatch_police_event(system(84), 14, listener, 0, 0, 0, 0, 0, 0, 2, 0,
                                                0);
            if (admitted)
                return 0;
            at<Byte>(system(84), 92 + listener) = 1;
            return 0;
        }
    }
    play_unavailable_action(listener);
    return 0;
}

Byte request_network_spikes(void *entity) {
    return police_frame::request_network_spikes_retained(entity);
}

Byte request_network_backup(void *entity) {
    void *chase = network_chase(entity);
    if (!chase)
        return 0;
    int listener = pursuit_listener(chase);
    if (listener == -1)
        listener = 0;
    if (at<Dword>(chase, 40) > 2 ||
        !record_support_request(at<void *>(system(92), 4), network::world::vehicle_slot(entity),
                                3)) {
        play_unavailable_action(listener);
        return 0;
    }
    if (!request_backup_car(chase, nullptr)) {
        release_support_request(at<void *>(system(92), 4), network::world::vehicle_slot(entity),
                                3);
        return 0;
    }
    void *cop = at<Dword>(chase, 40) ? at<void *>(chase, 28) : nullptr;
    float position[3];
    invoke(at<void *>(cop, 368), 0, position);
    position[2] = -position[2];
    const Dword location = sound_location(position);
    speech::game::dispatch_police_event(system(84), 0, listener, 0, 0, 0, 0, 0, 0, 0, 0,
                                        int(location));
    const Dword phrase = audio::vehicle::pending::create_speech_event(0, 0, 221);
    audio::vehicle::pending::speech_sink(phrase, 0);
    audio::vehicle::pending::flush_speech_events(0);
    speech::game::dispatch_police_event(system(84), 25, listener, 0, 0, 0, 0, 0, 0, 0, 0,
                                        int(location));
    at<Byte>(entity, 573) = 1;
    at<Dword>(entity, 576) = now();
    return 1;
}
} // namespace nfs::game::runtime::ai::cars
