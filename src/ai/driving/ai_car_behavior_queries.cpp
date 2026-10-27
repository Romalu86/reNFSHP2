#include "ai_car_behavior.hpp"
#include "nfs/network/session/network_runtime.hpp"
#include "nfs/core/services/service_lifecycle.hpp"
namespace nfs::application::pending {
void *race_participant_owner_view(void *, unsigned);
}
namespace nfs::network {
kernel::Dword race_pending(kernel::Dword *);
}
namespace nfs::game::runtime::ai::cars {

void *autonomous_cop_entity(void *entity) {
    return invoke<Dword>(entities::interface_at(entity, 1), 0) == 3 ? entity : nullptr;
}

void *traffic_entity(void *entity) {
    return invoke<Dword>(entities::interface_at(entity, 1), 0) == 4 ? entity : nullptr;
}

void *spike_entity(void *entity) {
    return invoke<Dword>(entities::interface_at(entity, 1), 0) == 8 ? entity : nullptr;
}

void *mutable_spike_entity(void *entity) {
    return invoke<Dword>(entities::interface_at(entity, 1), 0) == 8 ? entity : nullptr;
}

void *projectile_entity(void *entity) {
    return invoke<Dword>(entities::interface_at(entity, 1), 0) == 9 ? entity : nullptr;
}

int contact_entity_count(void *entity) {
    return at<int>(entity, 8);
}

Dword car_is_traffic(void *car) {
    return at<void *>(car, 7988) && traffic_entity(at<void *>(car, 7988));
}

void clear_path_history(void *path) {
    at<Dword>(path, 32) = 0;
    at<Dword>(path, 28) = 0;
    at<Dword>(path, 36) = 0;
}

float path_side_width(void *path, const void *node) {
    return at<float>(node, at<int>(path, 20) > 0 ? 16 : 12);
}
// Signed comparisons use wrapped saved tick arithmetic.
Byte recently_stunned(void *entity, Byte player_window) {
    if (player_window && invoke<Dword>(entity, 10)) {
        const Dword tick = at<Dword>(entity, 584);
        return std::int32_t(tick) > 0 &&
               std::int32_t(tick + 300) > std::int32_t(graphics::unresolved::camera_state);
    }
    const Dword tick = at<Dword>(entity, 584);
    if (std::int32_t(tick) > 0) {
        using kernel::numeric::Extended;
        const auto format = kernel::numeric::current_format();
        const auto delay =
            (Extended(at<float>(system(76), 84604), format) * -60.f).truncate_to_integer();
        if (std::int32_t(tick - Dword(delay)) >=
            std::int32_t(graphics::unresolved::camera_state))
            return 1;
        at<Dword>(entity, 584) = 0;
    }
    return 0;
}

Dword participant_completed_checkpoints(void *manager, Dword slot) {
    return at<Dword>(at<void *>(manager, 4), 600 + slot * 988);
}

Byte camera_mode_flashes(void *camera) {
    const int mode = at<int>(camera, 312 + at<Dword>(camera, 356) * 4);
    if (at<Byte>(camera, 172))
        return 0;
    return mode == 1 || mode == 0 || mode == 2 || mode == 3;
}

Dword should_draw_network_car(Dword slot) {
    void *car = at<void *>(system(68), 20 + slot * 4);
    if (car && services::pending::car_is_remote(car) &&
        (at<Dword>(car, 3400) == Dword(-1) || !(at<Byte>(car, 3476) & 8)))
        return 0;
    if (at<Dword>(system(72), 152) == 1) {
        void *participant = application::pending::race_participant_owner_view(system(92), slot);
        if (participant && (network::race_pending(static_cast<Dword *>(participant)) & 255))
            return 0;
    }
    return 1;
}
} // namespace nfs::game::runtime::ai::cars
