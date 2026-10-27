#include "police_chase_x87.hpp"
#include "chase_runtime.hpp"
#include <cmath>
namespace nfs::game::runtime::ai::lifecycle::pending {
Dword threat_memory_ticks{};

Dword chase_elapsed_ticks(void *chase) {
    return now() - at<Dword>(chase, 24);
}

Dword update_chase_meter(void *chase, std::int16_t threats) {
    void *target = at<void *>(chase, 44);
    if (at<Byte>(target, 596)) {
        entities::pending::set_police_chase_outcome(chase, 2);
        return 2;
    }
    if (at<Dword>(chase, 40)) {
        void *cop = at<void *>(chase, 28);
        if (invoke<Dword>(entities::interface_at(cop, 1), 0) == 2) {
            if (invoke<Dword>(entities::interface_at(target, 1), 0) != 1)
                threats = 1;
        } else if (at<Byte>(at<void *>(target, 368), 7952))
            threats = 1;
    }
    Dword time = now();
    if (threats)
        at<Dword>(chase, 80) = time;
    if (at<Dword>(chase, 72) < at<Dword>(chase, 84) &&
        int(time - threat_memory_ticks) <= at<int>(chase, 80)) {
        if (at<Dword>(chase, 72))
            at<Dword>(chase, 76) = time;
    } else {
        bool exhausted = true;
        if (at<Dword>(chase, 72)) {
            const Dword prior = at<Dword>(chase, 76);
            at<Dword>(chase, 76) = time;
            const Dword remaining = at<Dword>(chase, 72) - (time - prior);
            at<Dword>(chase, 72) = remaining;
            exhausted = int(remaining) <= 0;
            if (exhausted) {
                at<Dword>(chase, 72) = 0;
                at<Dword>(chase, 76) = 0;
            }
        }
        if (exhausted)
            at<Dword>(chase, 60) = 2;
    }
    Dword outcome = at<Dword>(chase, 60);
    if (!outcome)
        return 0;
    if (state<Byte>(7352412) && outcome == 2)
        outcome = 1;
    entities::pending::set_police_chase_outcome(chase, outcome);
    return outcome;
}

Byte needs_chase_backup(void *chase) {
    return police_frame::needs_chase_backup_retained(chase);
}
// Receiver is the perpetrator interface, not the entity base.
void update_bust_timer(void *perp, float speed, Byte near_cop) {
    const Dword time = now();
    at<Byte>(perp, 24) = near_cop;
    if (near_cop)
        at<Dword>(perp, 20) = time;
    else if (at<int>(perp, 16) > 0 && at<int>(perp, 20) <= int(time - state<Dword>(7352416)))
        at<Dword>(perp, 16) = 0;
    const float threshold = at<float>(system(76), 84556);
    if (at<int>(perp, 16) > 0) {
        if (speed >= threshold)
            at<Dword>(perp, 16) = 0;
    } else if (speed < threshold)
        at<Dword>(perp, 16) = time;
}

Byte update_bust_speed(void *perp, float speed) {
    const Byte slow = speed < at<float>(system(76), 84556);
    at<Byte>(perp, 25) = slow;
    return slow;
}

Byte bust_timer_expired(void *perp) {
    return police_frame::bust_timer_expired_retained(perp);
}

Dword cop_threat_zone(void *target, void *cop) {
    return police_frame::cop_threat_zone_retained(target, cop);
}
} // namespace nfs::game::runtime::ai::lifecycle::pending
