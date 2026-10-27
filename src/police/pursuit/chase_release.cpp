#include "chase_runtime.hpp"
namespace nfs::game::runtime::ai::lifecycle::pending {

void clear_police_chase(void *chase) {
    if (at<Dword>(chase, 92) == 1 &&
        (at<int>(system(72), 152) == 10 || at<int>(system(72), 152) == 4)) {
        void *cop = at<Dword>(chase, 40) ? at<void *>(chase, 28) : nullptr;
        const Dword cop_slot = cop ? network::world::vehicle_slot(cop) : Dword(-1);
        void *target = at<void *>(chase, 44);
        const Dword target_slot = target ? network::world::vehicle_slot(target) : Dword(-1);
        invoke(at<void *>(system(92), 4), 31, cop_slot, target_slot,
               now() - at<Dword>(chase, 24));
    }
    void *target = at<void *>(chase, 44);
    at<Dword>(target, 584) = now();
    invoke(perp_interface(target), 3, chase);
    at<void *>(chase, 44) = nullptr;
    for (Dword index = 0; index < at<Dword>(chase, 40); ++index) {
        invoke(perp_interface(at<void *>(chase, 28 + index * 4)), 3, chase);
        at<void *>(chase, 28 + index * 4) = nullptr;
    }
    if (void *helicopter = at<void *>(chase, 48)) {
        if (!at<Byte>(helicopter, 349)) {
            ai::collision_state();
            if (helicopter)
                at<Byte>(helicopter, 348) = 0;
        } else
            entities::pending::return_helicopter_to_patrol(helicopter);
        at<void *>(chase, 48) = nullptr;
    }
    const bool selected = chase == state<void *>(7353308);
    at<Dword>(chase, 40) = 0;
    at<Dword>(chase, 88) = 0;
    if (selected)
        state<void *>(7353308) = nullptr;
}

void finish_chase(void *owner, void *chase) {
    if (at<Dword>(chase, 96) != Dword(-1)) {
        release_population_location(bytes(at<void *>(owner, 132)) + at<Dword>(chase, 96) * 64);
        at<Dword>(chase, 96) = Dword(-1);
    }
    void *target = at<void *>(chase, 44);
    const Dword police = invoke<Dword>(target, 8);
    clear_police_chase(chase);
    --at<std::uint16_t>(owner, 88);
    if (police)
        state<Byte>(7352730) = 0;
    if (invoke<Dword>(target, 10) && at<Dword>(chase, 92) == 1) {
        void *cop = at<Dword>(chase, 40) ? at<void *>(chase, 28) : nullptr;
        void *race = at<void *>(system(92), 4);
        using FinishMethod = void(__thiscall *)(void *, Dword, Dword, Dword);
        const auto *methods = *reinterpret_cast<const FinishMethod **>(race);
        const Dword elapsed = chase_elapsed_ticks(chase);
        const Dword target_slot = network::world::vehicle_slot(target);
        const Dword cop_slot = cop ? network::world::vehicle_slot(cop) : Dword(-1);
        methods[31](race, cop_slot, target_slot, elapsed);
    }
}
} // namespace nfs::game::runtime::ai::lifecycle::pending
