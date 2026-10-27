#include "police_chase_x87.hpp"
#include "chase_runtime.hpp"
namespace nfs::game::runtime::ai::lifecycle::pending {

void bind_chase_helicopter(void *chase, void *helicopter) {
    const Dword delay = helicopter_activation_ticks;
    if (helicopter) {
        at<void *>(chase, 48) = helicopter;
        at<Dword>(chase, 112) = delay + now();
    }
}

void *reserve_chase_helicopter(void *owner) {
    for (int index = 0; index < at<int>(owner, 8); ++index) {
        void *helicopter = at<void *>(owner, Dword(index) * 4);
        if (helicopter && !at<Byte>(helicopter, 348)) {
            at<Byte>(helicopter, 348) = 1;
            return helicopter;
        }
    }
    return nullptr;
}

void reset_helicopter_position(void *helicopter, const float *position) {
    void *body = invoke<void *>(entities::interface_at(helicopter, 1, 320), 1);
    for (unsigned offset : {8u, 20u})
        std::memcpy(bytes(body) + offset, runtime::saved::zero_vector, 12);
    for (unsigned offset : {356u, 384u})
        std::memcpy(bytes(helicopter) + offset, runtime::saved::zero_vector, 12);
    at<float>(body, 680) = position[0];
    at<float>(body, 684) = position[1];
    at<float>(body, 688) = -position[2];
    physics::clear_ground_contact(bytes(body) + 508);
    physics::pending::locate_ground_entry(bytes(body) + 508,
                                             reinterpret_cast<float *>(bytes(body) + 680),
                                             reinterpret_cast<float *>(bytes(body) + 484));
    synchronize_track_vehicle(at<void *>(helicopter, 368));
    lifecycle::clear_path(bytes(helicopter) + 220);
    invoke(helicopter, 31);
    for (unsigned offset : {372u, 376u})
        if (void *shape = at<void *>(helicopter, offset))
            if (void *attachment_body = at<void *>(shape, 80))
                at<Byte>(attachment_body, 729) = 0;
}

void activate_chase_helicopter(void *helicopter, void *chase) {
    police_frame::activate_chase_helicopter_retained(helicopter, chase);
}

void update_chase_helicopter(void *, void *chase) {
    void *helicopter = at<void *>(chase, 48);
    if (!helicopter) {
        helicopter = reserve_chase_helicopter(ai::collision_state());
        if (!helicopter)
            return;
        bind_chase_helicopter(chase, helicopter);
        void *target = at<void *>(chase, 44);
        if (invoke<Dword>(target, 11))
            speech::game::dispatch_police_event(system(84), 13, player_target_listener(target), 0,
                                                0, 0, 0, 0, 0, 2, 0, 0);
    }
    if (!at<Byte>(helicopter, 349)) {
        if (at<int>(chase, 112) <= int(now()))
            activate_chase_helicopter(helicopter, chase);
    } else if (at<Dword>(helicopter, 380) == 1)
        update_helicopter_announcement(system(84), chase);
}
} // namespace nfs::game::runtime::ai::lifecycle::pending
