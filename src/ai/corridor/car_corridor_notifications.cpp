#include "car_corridor_x87_private.hpp"
namespace nfs::game::runtime::ai::cars {

void __fastcall car_notify_vehicle(void *entity, void *, Dword context_word, Dword other_word, Dword) {
    void *context = reinterpret_cast<void *>(context_word);
    void *other = reinterpret_cast<void *>(other_word);
    void *other_car_entity = invoke<void *>(entities::interface_at(other, 2), 0);
    void *car = other_car_entity ? at<void *>(other_car_entity, 368) : nullptr;
    void *tuning = bytes(system(76)) + 84404;
    const int kind = invoke<int>(entities::interface_at(other, 1), 0);
    if (kind < 0)
        return;
    if (kind < 6) {
        corridor_apply_body(context, at<void *>(car, 1768), at<float>(tuning, 0), entity);
        const float margin = float(static_cast<long double>(invoke<long double>(entity, 22)) *
                                   0.1f * float_word(1055187230));
        corridor_apply_car(context, car, at<float>(tuning, 4), margin, entity);
        corridor_restrict_side(context, car, at<float>(tuning, 8), 3.f, 3.f);
    } else if (kind == 8)
        corridor_apply_spike(context, mutable_spike_entity(other), -45.f);
}

void __fastcall traffic_notify_vehicle(void *entity, void *, Dword context_word, Dword other_word,
                                       Dword) {
    void *context = reinterpret_cast<void *>(context_word);
    void *other = reinterpret_cast<void *>(other_word);
    void *other_car_entity = invoke<void *>(entities::interface_at(other, 2), 0);
    void *car = other_car_entity ? at<void *>(other_car_entity, 368) : nullptr;
    void *tuning = bytes(system(76)) + 84612;
    const int kind = invoke<int>(entities::interface_at(other, 1), 0);
    if (kind < 0)
        return;
    if (kind < 6) {
        corridor_apply_body(context, at<void *>(car, 1768), at<float>(tuning, 0), entity);
        corridor_apply_car(context, car, at<float>(tuning, 4), 50.f, entity);
        corridor_restrict_side(context, car, at<float>(tuning, 8), 3.f, 3.f);
    } else if (kind == 8)
        corridor_apply_spike(context, mutable_spike_entity(other), -45.f);
}

void __fastcall ai_cop_notify_vehicle(void *entity, void *, Dword context_word, Dword other_word,
                                      Dword flags) {
    void *context = reinterpret_cast<void *>(context_word);
    void *other = reinterpret_cast<void *>(other_word);
    void *owner = system(76);
    const int kind = invoke<int>(entities::interface_at(other, 1), 0);
    if (kind == 8) {
        if (!at<Byte>(entity, 656))
            return;
    } else {
        void *cop = autonomous_cop_entity(other);
        if (cop && at<Byte>(cop, 576)) {
            if (!at<Byte>(entity, 656))
                return;
            void *car = at<void *>(cop, 368);
            corridor_apply_body(context, at<void *>(car, 1768), -20.f, nullptr);
            corridor_restrict_side(context, car, at<float>(owner, 84480), 3.f, 3.f);
            return;
        }
    }
    player_notify_vehicle(entity, nullptr, context_word, other_word, flags);
}
} // namespace nfs::game::runtime::ai::cars
