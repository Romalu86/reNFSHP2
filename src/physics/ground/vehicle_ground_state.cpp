#include "nfs/physics/solver/solver_runtime.hpp"
#include "../motion/world_motion_x87.hpp"
namespace nfs::game::runtime::physics::pending {

void update_ground_state(void *state) {
    auto *position = reinterpret_cast<float *>(bytes(state) + 680);
    auto *height = reinterpret_cast<float *>(bytes(state) + 484);
    void *contact = bytes(state) + 508;
    if (locate_ground_entry(contact, position, height)) {
        for (int i = 0; i < 3; ++i)
            at<Dword>(state, 492 + i * 4) = at<Dword>(state, 680 + i * 4);
        return;
    }
    if (at<Byte>(state, 5)) {
        for (int i = 0; i < 3; ++i)
            at<Dword>(state, 680 + i * 4) = at<Dword>(state, 492 + i * 4);
        if (!locate_ground_entry(contact, position, height) && at<Byte>(state, 5) == 1)
            reset_track_vehicle_entry(at<void *>(state, 0));
        return;
    }
    void *car = at<void *>(state, 0);
    if (car && car_is_racer(car)) {
        void *interface = at<void *>(car, 7988);
        at<Byte>(state, 728) = 0;
        if (interface)
            retire_vehicle(interface);
        return;
    }
    for (int i = 0; i < 3; ++i)
        at<Dword>(state, 680 + i * 4) = at<Dword>(state, 492 + i * 4);
    at<Dword>(state, 8) = at<Dword>(state, 16) = 0;
    locate_ground_entry(contact, position, height);
}

void finish_body(void *state) {
    auto X = [](float value) { return static_cast<long double>(value); };
    auto *position = reinterpret_cast<float *>(bytes(state) + 680);
    float height = 0;
    if (!locate_ground_entry(bytes(state) + 508, position, &height)) {
        bool retired = false;
        if (!at<Byte>(state, 5) && at<void *>(state, 0) && car_is_racer(at<void *>(state, 0))) {
            void *interface = at<void *>(at<void *>(state, 0), 7988);
            at<Byte>(state, 728) = 0;
            if (interface)
                retire_vehicle(interface);
            retired = true;
        }
        if (!retired) {
            const float impact = at<float>(state, 608);
            if (impact == 0) {
                const auto speed =
                    hardware::vector_length(reinterpret_cast<float *>(bytes(state) + 8));
                at<Dword>(state, 640) = at<Dword>(state, 680);
                at<float>(state, 608) = float(speed * at<float>(state, 32) * 0.5f);
                at<Dword>(state, 644) = at<Dword>(state, 684);
                at<Dword>(state, 652) = 1;
                at<Dword>(state, 656) = 31;
                at<Dword>(state, 604) = 0;
                at<Dword>(state, 648) = at<Dword>(state, 688);
                at<float>(state, 612) = float(X(at<float>(state, 32)) *
                                                at<float>(state, 8) * -0.5f);
                at<float>(state, 616) = float(X(at<float>(state, 12)) *
                                                at<float>(state, 32) * -0.5f);
                at<float>(state, 620) = float(X(at<float>(state, 16)) *
                                                at<float>(state, 32) * -0.5f);
            }
            for (int i = 0; i < 3; ++i)
                at<Dword>(state, 680 + i * 4) = at<Dword>(state, 492 + i * 4);
            at<Dword>(state, 8) = at<Dword>(state, 16) = 0;
            locate_ground_entry(bytes(state) + 508, position, &height);
        }
    }
    if (height < -at<float>(state, 132)) {
        at<float>(state, 684) = float(X(at<float>(state, 684)) - height +
                                        at<float>(state, 132));
        at<float>(state, 12) = at<float>(state, 12) * 0.95f;
    }
}
} // namespace nfs::game::runtime::physics::pending
