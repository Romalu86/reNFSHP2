#include "../driving/ai_route_planner.hpp"
#include "nfs/audio/speech/speech_game.hpp"
#include <cmath>
namespace nfs::game::runtime::ai::entities::pending {

void finish_escaped_chase(void *chase) {
    at<Dword>(chase, 92) = 2;
    at<Dword>(chase, 88) = 0;
    float nearest = 10000.f;
    long double last_distance = 0;
    for (Dword index = 0, remaining = at<Dword>(chase, 40); remaining; ++index, --remaining)
        if (void *pursuer = at<void *>(chase, 28 + index * 4)) {
            last_distance = ai::pair_distance(at<void *>(chase, 44), pursuer);
            if (last_distance < nearest)
                nearest = float(last_distance);
        }
    // The saved event selection uses the last queried distance, not the minimum.
    const Dword event = last_distance > 200.f ? 8 : 9;
    Dword *rules = system(72);
    if (at<int>(rules, 152) == 10 || at<int>(rules, 152) == 4)
        return;
    void *target = at<void *>(chase, 44);
    int voice;
    if (at<int>(rules, 176) == 1) {
        if (!invoke<Dword>(target, 11))
            return;
        voice = at<int>(system(84), 64);
        at<Byte>(system(84), 89) = 0;
    } else {
        if (!invoke<Dword>(target, 11))
            return;
        voice = at<int>(system(84), target == at<void *>(system(76), 84164) ? 64 : 68);
    }
    if (voice != -1)
        speech::game::dispatch_police_event(system(84), event, voice, 0, 0, 0, 0, 0, 0, 2, 0, 0);
}

Dword set_police_chase_outcome(void *chase, Dword outcome) {
    at<Dword>(chase, 60) = outcome;
    const Dword now = graphics::unresolved::camera_state;
    if (!outcome) {
        at<Dword>(chase, 92) = 0;
        return 0;
    }
    if (outcome == 2)
        finish_escaped_chase(chase);
    else if (outcome == 1) {
        at<Dword>(chase, 92) = 1;
        at<Dword>(chase, 88) = 3;
        at<Dword>(chase, 4) = now;
    }
    const Dword result = at<Dword>(chase, 92);
    invoke(interface_at(at<void *>(chase, 44), 4), 2, result);
    for (Dword index = 0, remaining = at<Dword>(chase, 40); remaining; ++index, --remaining)
        invoke(interface_at(at<void *>(chase, 28 + index * 4), 4), 2, result);
    return outcome;
}

void return_helicopter_to_patrol(void *entity) {
    at<Dword>(entity, 408) = 0;
    at<Dword>(entity, 380) = 2;
    speech::game::dispatch_police_event(system(84), 26, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    at<void *>(entity, 436) = bytes(entity) + 424;
    select_helicopter_destination(entity);
    at<Byte>(entity, 349) = at<Byte>(entity, 1660) = 0;
    for (unsigned offset : {1668u, 1676u, 1652u, 352u, 344u})
        at<Dword>(entity, offset) = 0;
    at<float>(entity, 1672) = -1;
}

void release_chase_helicopter(void *chase) {
    at<Byte>(chase, 20) = 0;
    if (void *entity = at<void *>(chase, 48)) {
        return_helicopter_to_patrol(entity);
        at<Dword>(chase, 48) = 0;
    }
}
} // namespace nfs::game::runtime::ai::entities::pending
