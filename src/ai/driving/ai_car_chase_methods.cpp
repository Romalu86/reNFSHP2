#include "ai_car_runtime.hpp"
#include "../lifecycle/ai_chase_tuning.hpp"
#include "nfs/graphics/resources/eagl_graphics.hpp"
namespace nfs::game::runtime::ai::cars {
namespace {
void *perpetrator_from_view(void *view, int offset) {
    void *base_pointer = bytes(view) - offset;
    return bytes(base_pointer) + at<const int *>(base_pointer, 0)[4];
}
} // namespace

void *chased_entity(void *entity) {
    void *chase = at<void *>(entities::interface_at(entity, 4), 8);
    return chase ? at<void *>(chase, 44) : nullptr;
}
// Complete RET4 leaf.
void perpetrator_chase_result(void *, Dword) {
}

void __fastcall pursuit_bind_chase(void *view, void *, Dword chase) {
    void *perpetrator = perpetrator_from_view(view, 632);
    at<Byte>(view, -56) = 0;
    perpetrator_bind(perpetrator, nullptr, chase);
}

void __fastcall pursuit_chase_outcome(void *view, void *, Dword outcome) {
    if (outcome == 1)
        invoke(bytes(view) - 636, 35);
}

void __fastcall pursuit_clear_chase(void *view, void *, Dword chase) {
    perpetrator_clear(perpetrator_from_view(view, 632), nullptr, chase);
    at<Dword>(view, -232) = 0;
    if (at<int>(reinterpret_cast<void *>(chase), 92) == 1)
        clear_tire_damage(bytes(at<void *>(view, -268)) + 7952);
}

void __fastcall cop_base_bind_chase(void *view, void *, Dword chase) {
    perpetrator_bind(perpetrator_from_view(view, 600), nullptr, chase);
    at<Byte>(at<void *>(view, -236), 1579) = 1;
}

void __fastcall cop_base_clear_chase(void *view, void *, Dword chase) {
    at<Byte>(at<void *>(view, -236), 1579) = 0;
    void *perpetrator = perpetrator_from_view(view, 600);
    at<Dword>(view, -200) = 0;
    perpetrator_clear(perpetrator, nullptr, chase);
}

void __fastcall cop_base_chase_outcome(void *view, void *, Dword outcome) {
    if (at<int>(system(72), 148) != 1 && at<int>(system(72), 152) != 10 && outcome == 1)
        invoke(bytes(view) - 604, 14, Dword{5});
    perpetrator_chase_result(perpetrator_from_view(view, 600), outcome);
}

void release_cop_reservation(void *controller, void *entity) {
    lifecycle::release_path_reservation(bytes(controller) + 92, entity);
}

void __fastcall autonomous_cop_bind_chase(void *view, void *, Dword chase) {
    auto *entity = reinterpret_cast<Dword *>(bytes(view) - 1896);
    release_cop_reservation(lifecycle::state<void *>(7352448), entity);
    lifecycle::dispatch_cop_event(entity, 6);
    cop_base_bind_chase(bytes(view) - 1292, nullptr, chase);
    invoke(entity, 46, Dword{0});
    if (void *target = chased_entity(entity)) {
        void *car = at<void *>(target, 368);
        at<Dword>(view, -48) = at<Dword>(car, 3136);
        at<Dword>(view, -44) = at<Dword>(car, 3144);
    }
}

void __fastcall autonomous_cop_clear_chase(void *view, void *, Dword chase) {
    if (at<void *>(perpetrator_from_view(view, 1892), 8)) {
        const Dword outcome = at<Dword>(reinterpret_cast<void *>(chase), 92);
        cop_base_clear_chase(bytes(view) - 1292, nullptr, chase);
        lifecycle::dispatch_cop_event(reinterpret_cast<Dword *>(bytes(view) - 1896),
                                      outcome == 1 ? 8 : 7);
    }
    at<Dword>(view, -1476) = Dword(-1);
    at<Dword>(view, -84) = 0;
    at<Dword>(view, -80) = 0;
    at<Dword>(view, -76) = 0;
    at<Dword>(view, -72) = 0;
    at<Dword>(view, -68) = 0;
    at<Dword>(view, -64) = 0;
    at<Dword>(view, -60) = 0;
    at<Byte>(view, -56) = 1;
    at<Dword>(view, -52) = Dword(-1);
}

void __fastcall network_car_bind_chase(void *view, void *, Dword chase) {
    at<Byte>(view, -60) = 0;
    at<Byte>(view, -59) = 0;
    at<Byte>(view, -58) = 0;
    at<Dword>(view, -56) = 0;
    cop_base_bind_chase(bytes(view) - 28, nullptr, chase);
    at<Byte>(at<void *>(view, -264), 1579) = 1;
}

void __fastcall network_car_clear_chase(void *view, void *, Dword chase) {
    if (at<void *>(perpetrator_from_view(view, 628), 8)) {
        at<Byte>(at<void *>(view, -264), 1579) = 0;
        cop_base_clear_chase(bytes(view) - 28, nullptr, chase);
        at<Dword>(view, -64) = 0;
    }
}

void __fastcall ai_racer_bind_chase(void *view, void *, Dword chase) {
    void *perpetrator = perpetrator_from_view(view, 640);
    at<Byte>(view, -44) = 0;
    perpetrator_bind(perpetrator, nullptr, chase);
}

void __fastcall ai_racer_clear_chase(void *view, void *, Dword chase) {
    perpetrator_clear(perpetrator_from_view(view, 640), nullptr, chase);
}

void __fastcall ai_cop_bind_chase(void *view, void *, Dword chase) {
    const long double fraction = lifecycle::random_fraction();
    at<Byte>(view, -52) = fraction <= lifecycle::pending::roadblock_evade_fraction;
    at<Dword>(view, -48) = 0;
    pursuit_bind_chase(bytes(view) - 72, nullptr, chase);
}

void __fastcall ai_cop_clear_chase(void *view, void *, Dword chase) {
    perpetrator_clear(perpetrator_from_view(view, 704), nullptr, chase);
    at<Dword>(perpetrator_from_view(view, 704), 16) = 0;
    at<Byte>(view, -52) = 1;
    at<Dword>(view, -48) = 0;
}

void __fastcall autonomous_cop_hold_controls(void *entity, void *, Dword duration) {
    lifecycle::dispatch_cop_event(static_cast<Dword *>(entity), 12);
    car_hold_controls(entity, nullptr, duration);
}

void __fastcall autonomous_cop_notify_vehicle(void *entity, void *, Dword event, Dword other,
                                              Dword parameter) {
    if (other != word_pointer(chased_entity(entity)))
        car_notify_vehicle(entity, nullptr, event, other, parameter);
}
} // namespace nfs::game::runtime::ai::cars
