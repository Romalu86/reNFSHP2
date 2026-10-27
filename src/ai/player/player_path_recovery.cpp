#include "player_drive_runtime.hpp"
#include "../driving/ai_drive_x87.hpp"
#include "../../network/vehicles/remote_car_runtime.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
#include <cstring>
namespace nfs::game::runtime::ai::cars {

void reverse_player_on_path(void *entity, Dword controls) {
    if (!invoke<Byte>(entity, 45, controls, float_word(1087804172))) {
        Dword point = 0;
        float target[3], pose[21];
        const float *position = invoke<const float *>(entity, 0);
        auto *speed = reinterpret_cast<float *>(bytes(entity) + 624);
        if (!entities::pending::sample_path_target(bytes(entity) + 220, target, speed, &point,
                                                   pose, 20.f, position, 0))
            return;
        drive_frame::discard_steering(entity, target, reinterpret_cast<void *>(controls));
        invoke(entity, 33, float_word(1087804172), controls);
    }
    void *body = invoke<void *>(entities::interface_at(entity, 3), 0);
    if (!body || !at<Byte>(body, 728) || at<void *>(entities::interface_at(entity, 4), 8))
        return;
    void *owner = system(76);
    const Dword elapsed = graphics::unresolved::camera_state - at<Dword>(entity, 604);
    std::int32_t signed_elapsed;
    std::memcpy(&signed_elapsed, &elapsed, sizeof(signed_elapsed));
    const auto seconds = static_cast<long double>(signed_elapsed) * float_word(1015580809);
    if (!(seconds > static_cast<long double>(at<float>(owner, 84508))))
        return;
    float reset_speed;
    bool use_default_speed = true;
    if (at<float>(owner, 84516) != 0.f) {
        const float *position = invoke<const float *>(entity, 0);
        const float distance = local_player_distance(position);
        // Position callbacks can change the tuning value; compare its current value.
        if (distance > at<float>(owner, 84516)) {
            const float *node = lifecycle::pending::path_position(bytes(entity) + 220, 0);
            if (node[6] > entities::pending::minimum_path_width)
                std::memcpy(&reset_speed, node + 6, sizeof(reset_speed));
            else
                std::memcpy(&reset_speed, &entities::pending::minimum_path_width,
                            sizeof(reset_speed));
            use_default_speed = false;
        }
    }
    if (use_default_speed)
        reset_speed = float(static_cast<long double>(at<float>(owner, 84512)) *
                            float_word(1055187230));
    physics::request_car_reset(at<void *>(entity, 368), reset_speed);
    if (at<Dword>(entity, 600) != 1) {
        at<Dword>(entity, 600) = 1;
        at<Dword>(entity, 604) = graphics::unresolved::camera_state;
    }
}

void recover_player_on_path(void *entity, Dword controls) {
    drive_player_path(entity, controls);
    void *body = invoke<void *>(entities::interface_at(entity, 3), 0);
    if (!body || !at<Byte>(body, 728))
        return;
    body = invoke<void *>(entities::interface_at(entity, 3), 1);
    if (at<std::int16_t>(body, 596)) {
        body = invoke<void *>(entities::interface_at(entity, 3), 1);
        if (at<std::uint16_t>(body, 596) < 12)
            return;
    }
    if (at<void *>(entities::interface_at(entity, 4), 8))
        return;
    void *owner = system(76);
    const Dword elapsed = graphics::unresolved::camera_state - at<Dword>(entity, 604);
    std::int32_t signed_elapsed;
    std::memcpy(&signed_elapsed, &elapsed, sizeof(signed_elapsed));
    const auto seconds = static_cast<long double>(signed_elapsed) * float_word(1015580809);
    if (!(seconds > static_cast<long double>(at<float>(owner, 84508))))
        return;
    float reset_speed;
    bool use_default_speed = true;
    if (at<float>(owner, 84516) != 0.f) {
        const float *position = invoke<const float *>(entity, 0);
        const float distance = local_player_distance(position);
        // Position callbacks can change the tuning value; compare its current value.
        if (distance > at<float>(owner, 84516)) {
            const float *node = lifecycle::pending::path_position(bytes(entity) + 220, 0);
            if (node[6] > entities::pending::minimum_path_width)
                std::memcpy(&reset_speed, node + 6, sizeof(reset_speed));
            else
                std::memcpy(&reset_speed, &entities::pending::minimum_path_width,
                            sizeof(reset_speed));
            use_default_speed = false;
        }
    }
    if (use_default_speed)
        reset_speed = float(static_cast<long double>(at<float>(owner, 84512)) *
                            float_word(1055187230));
    physics::request_car_reset(at<void *>(entity, 368), reset_speed);
    if (at<Dword>(entity, 600) != 1) {
        at<Dword>(entity, 600) = 1;
        at<Dword>(entity, 604) = graphics::unresolved::camera_state;
    }
}
} // namespace nfs::game::runtime::ai::cars
