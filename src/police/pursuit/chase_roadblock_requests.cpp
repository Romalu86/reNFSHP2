#include "chase_runtime.hpp"
#include "../../graphics/vehicles/car_wheel_effects_runtime.hpp"
namespace nfs::game::runtime::ai::lifecycle::pending {

Byte request_roadblock(void *owner, void *chase) {
    void *location = invoke<void *>(bytes(owner) + 120, 1, at<void *>(chase, 44));
    if (!location)
        return 0;
    const Dword count = at<std::uint16_t>(owner, 82);
    for (Dword ordinal = 0; ordinal < count; ++ordinal) {
        void *cop = at<void *>(owner, 32 + ordinal * 4);
        if (at<Dword>(cop, 572))
            continue;
        ai::pending::dispatch_vehicle_event(static_cast<Dword *>(cop), 9);
        const Dword index = place_roadblock_cop(location, cop);
        at<Dword>(chase, 96) = index;
        if (index == Dword(-1))
            return 1;
        const int strategy = at<int>(chase, 16);
        if (strategy == 1 || strategy == 2) {
            for (Dword candidate = 0; candidate < count; ++candidate) {
                void *additional = at<void *>(owner, 32 + candidate * 4);
                if (at<Dword>(additional, 572))
                    continue;
                ai::pending::dispatch_vehicle_event(static_cast<Dword *>(additional), 9);
                place_roadblock_cop(location, additional);
                break;
            }
        }
        if (strategy == 0 || strategy == 2)
            if (void *spike = reserve_spike_entity(bytes(owner) + 136))
                place_roadblock_spikes(location, spike);
        void *announcement_location = bytes(at<void *>(owner, 132)) + index * 64;
        float position[3];
        std::memcpy(position, bytes(announcement_location) + 16, 12);
        position[2] = -position[2];
        const Dword sound = sound_location(position);
        Dword event;
        if (!at<Dword>(announcement_location, 44))
            event = 4;
        else if (!at<Byte>(announcement_location, 60))
            event = 18 - Dword(runtime::emitter_random_word() % 100 > 30);
        else
            event = 19 - 2 * Dword(runtime::emitter_random_word() % 100 > 30);
        speech::game::dispatch_police_event(system(84), event, 3, 0, 0, 0, 0, 0, 0, 2, 0,
                                            int(sound));
        return 1;
    }
    at<Byte>(location, 28) = 0;
    return 0;
}

Byte passed_roadblock(void *location, void *target) {
    int distance = int(at<Dword>(location, 8) - at<Dword>(at<void *>(target, 368), 7984));
    if (distance < 0)
        distance = int(Dword(distance) + Dword(track_point_count(runtime::track_collision)));
    return track_point_count(runtime::track_collision) / 2 < distance;
}

void update_spike_announcement(void *owner, void *chase) {
    void *location = bytes(at<void *>(owner, 132)) + at<Dword>(chase, 96) * 64;
    if (at<Byte>(location, 48))
        return;
    void *target = at<void *>(chase, 44);
    if (!passed_roadblock(location, target))
        return;
    void *wheels = bytes(at<void *>(target, 368)) + 7952;
    bool punctured = false;
    for (Byte wheel : {Byte(0), Byte(1), Byte(3), Byte(2)})
        if (physics::contacts_pending::wheel_is_punctured(wheels, wheel)) {
            punctured = true;
            break;
        }
    if (!punctured) {
        const Dword event = 6 - Dword(at<Byte>(target, 580) != 0);
        int listener;
        const int mode = at<int>(system(72), 152);
        bool announce = true;
        if (mode == 10 || mode == 4) {
            listener = pursuit_listener(chase);
            if (listener == -1)
                listener = 0;
        } else {
            if (at<int>(system(72), 176) == 1) {
                if (invoke<Dword>(target, 11))
                    listener = at<int>(system(84), 64);
                else {
                    listener = at<int>(system(84), 68);
                    if (at<Byte>(system(84), 89))
                        announce = false;
                }
            } else {
                if (!invoke<Dword>(target, 11)) {
                    listener = -1;
                    announce = false;
                } else
                    listener = at<int>(system(84),
                                       target == at<void *>(system(76), 84164) ? 64 : 68);
            }
            if (listener == -1)
                announce = false;
        }
        if (announce)
            speech::game::dispatch_police_event(system(84),
                                                event, listener, 0,
                                                0, 0, 0, 0, 0, 2, 0, -1);
    }
    at<Byte>(location, 48) = 1;
}
} // namespace nfs::game::runtime::ai::lifecycle::pending
