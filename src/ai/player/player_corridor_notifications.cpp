#include "../corridor/car_corridor_runtime.hpp"
namespace nfs::game::runtime::ai::cars {

void __fastcall player_notify_vehicle(void *entity, void *, Dword context_word, Dword other_word,
                                      Dword) {
    void *context = reinterpret_cast<void *>(context_word);
    void *other = reinterpret_cast<void *>(other_word);
    void *owner = system(76);
    void *tuning = bytes(owner) + 84472;
    void *other_car_entity = invoke<void *>(entities::interface_at(other, 2), 0);
    void *car = other_car_entity ? at<void *>(other_car_entity, 368) : nullptr;
    void *body = invoke<void *>(entities::interface_at(entity, 3), 1);
    if (at<Dword>(body, 732))
        return;
    body = invoke<void *>(entities::interface_at(entity, 3), 0);
    if (!body || !at<Byte>(body, 728))
        return;
    using kernel::numeric::Extended;
    const auto format = kernel::numeric::current_format();
    auto speed_margin = [&](float scale) {
        return float(Extended::from_double(double(invoke<long double>(entity, 22)), format) *
                     scale * float_word(1055187230));
    };
    const Dword kind = invoke<Dword>(entities::interface_at(other, 1), 0);
    switch (kind) {
    case 0:
    case 5:
        corridor_apply_body(context, at<void *>(car, 1768), at<float>(tuning, 0), entity);
        corridor_apply_car(context, car, at<float>(owner, 84476), speed_margin(0.2f), entity);
        corridor_restrict_side(context, car, at<float>(owner, 84480), 3.f, 4.f);
        return;
    case 1:
    case 2: {
        if (!other_car_entity)
            return;
        const float *other_position = invoke<const float *>(other_car_entity, 0);
        const float *position = invoke<const float *>(entity, 0);
        const float distance = float(runtime::point_distance(position, other_position));
        bool repel, block_body, block_side, attract_side, attract_behind, repel_behind;
        switch (at<Dword>(entities::interface_at(entity, 5), 8)) {
        case 1:
            repel = true;
            attract_side = false;
            block_body = true;
            block_side = true;
            attract_behind = false;
            repel_behind = false;
            break;
        case 2:
            repel = false;
            attract_side = false;
            block_body = true;
            block_side = true;
            attract_behind = false;
            repel_behind = false;
            break;
        case 4:
            repel = false;
            attract_side = true;
            block_body = false;
            block_side = true;
            attract_behind = false;
            repel_behind = true;
            break;
        case 5:
            repel = false;
            attract_side = true;
            block_body = false;
            block_side = false;
            attract_behind = true;
            repel_behind = true;
            break;
        default:
            repel = false;
            attract_side = true;
            block_body = true;
            block_side = true;
            attract_behind = false;
            repel_behind = false;
            break;
        }
        const Dword difficulty = at<Dword>(system(72), 192);
        if (!difficulty) {
            attract_side = false;
            block_body = true;
            block_side = true;
            attract_behind = false;
            repel_behind = false;
        } else if (difficulty == 1) {
            block_body = true;
            attract_behind = false;
        } else if (difficulty == 2)
            repel = false;
        if (at<Dword>(entity, 600) == 3) {
            attract_side = false;
            attract_behind = false;
            repel_behind = false;
        }
        if (repel_behind && at<int>(entity, 632) > 0 && at<void *>(entity, 628) &&
            other_car_entity == at<void *>(entity, 628) && distance < 30.f) {
            corridor_restrict_side(context, car, at<float>(owner, 84484), 30.f, 3.f);
            corridor_restrict_behind(context, car, at<float>(owner, 84484), 20.f, 20.f);
            corridor_apply_body(context, at<void *>(car, 1768), at<float>(owner, 84484),
                                nullptr);
            corridor_apply_car(context, car, at<float>(owner, 84484), speed_margin(0.1f),
                               nullptr);
            return;
        }
        if (attract_behind)
            corridor_restrict_side(context, car, at<float>(owner, 84484), 3.f, 3.f);
        else if (block_side)
            corridor_restrict_side(context, car, at<float>(owner, 84480), 3.f, 3.f);
        void *notify = block_body ? entity : nullptr;
        corridor_apply_body(context, at<void *>(car, 1768), at<float>(tuning, 0), notify);
        corridor_apply_car(context, car, at<float>(owner, 84476), speed_margin(0.1f), notify);
        if (attract_side)
            corridor_restrict_behind(context, car, at<float>(owner, 84484), 6.f, 10.f);
        else if (repel)
            corridor_restrict_behind(context, car, at<float>(owner, 84480), 6.f, 10.f);
        return;
    }
    case 3:
        corridor_apply_body(context, at<void *>(car, 1768), at<float>(tuning, 0), entity);
        corridor_restrict_side(context, car, at<float>(owner, 84480), 3.f, 3.f);
        corridor_restrict_behind(context, car, at<float>(owner, 84480), 6.f, 10.f);
        return;
    case 4:
        corridor_apply_body(context, at<void *>(car, 1768), at<float>(tuning, 0), nullptr);
        return;
    case 8:
        corridor_apply_spike(context, mutable_spike_entity(other), -45.f);
        return;
    case 9:
        if (void *projectile = projectile_entity(other))
            corridor_apply_body(context, invoke<void *>(entities::interface_at(projectile, 3), 0),
                                -45.f, nullptr);
        return;
    }
}
} // namespace nfs::game::runtime::ai::cars
