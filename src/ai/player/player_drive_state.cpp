#include "player_drive_runtime.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
#include <cstring>
namespace nfs::game::runtime::ai::cars {
namespace {
std::int32_t signed_ticks(Dword value) {
    std::int32_t result;
    std::memcpy(&result, &value, sizeof(result));
    return result;
}
}
// Callers consume AL.
Byte player_needs_reverse(void *entity) {
    void *path = bytes(entity) + 220;
    if (!entities::path_has_section(path))
        return 0;
    void *body = invoke<void *>(entities::interface_at(entity, 3), 0);
    if (!body || !at<Byte>(body, 728))
        return 0;
    void *car = at<void *>(entity, 368);
    if (at<std::uint16_t>(at<void *>(car, 1768), 596) > 32)
        return 1;
    float pose[21], forward[3];
    lifecycle::sample_path(path, 0, pose);
    pose[5] = float(-static_cast<long double>(pose[5]));
    std::memcpy(forward, bytes(at<void *>(car, 1768)) + 716, 12);
    const auto alignment = hardware::dot_product(forward, pose + 3);
    const long double threshold = 0.5L;
    if (at<float>(at<void *>(car, 1768), 592) > float_word(1087804172) && threshold < alignment)
        return 0;
    return alignment < threshold;
}

void update_player_drive_state(void *entity) {
    auto set_state = [&](Dword state) {
        if (at<Dword>(entity, 600) != state) {
            at<Dword>(entity, 600) = state;
            at<Dword>(entity, 604) = graphics::unresolved::camera_state;
        }
    };
    if (!race_started)
        set_state(0);
    else {
        const Dword kind = invoke<Dword>(entities::interface_at(entity, 1), 0);
        bool selected = false;
        if (kind != 5 && !invoke<Byte>(at<void *>(system(92), 4), 29,
                                       at<Dword>(at<void *>(entity, 368), 1772))) {
            set_state(5);
            selected = true;
        }
        if (!selected) {
            if (player_needs_reverse(entity))
                set_state(2);
            else if (signed_ticks(graphics::unresolved::camera_state) <
                     signed_ticks(start_deadline + Dword{360}))
                set_state(3);
            else {
                bool recover = false;
                void *body = invoke<void *>(entities::interface_at(entity, 3), 0);
                if (body && at<Byte>(body, 728) &&
                    invoke<long double>(entity, 22) < float_word(1096192780)) {
                    body = invoke<void *>(entities::interface_at(entity, 3), 0);
                    if (at<float>(body, 592) < float_word(1096192780)) {
                        const auto speed = invoke<long double>(entity, 22);
                        const auto threshold =
                            static_cast<long double>(at<float>(entity, 624)) * 0.5f;
                        recover = speed < threshold;
                    }
                }
                set_state(recover ? 4 : 1);
            }
        }
    }
    const float *position = invoke<const float *>(entity, 0);
    if (local_player_distance(position) > 300.f) {
        void *body = invoke<void *>(entities::interface_at(entity, 3), 1);
        at<Dword>(body, 732) = 16;
    }
}

} // namespace nfs::game::runtime::ai::cars
