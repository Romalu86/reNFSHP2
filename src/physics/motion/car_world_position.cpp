#include "../../ai/entities/ai_entity_runtime.hpp"
#include "../bodies/shape_body_runtime.hpp"
#include "nfs/audio/vehicles/vehicle_audio.hpp"
#include <cstring>
namespace nfs::game::runtime::physics {

void clear_body_motion(void *body) {
    for (unsigned field = 8; field <= 28; field += 4)
        at<Dword>(body, field) = 0;
    at<Dword>(body, 108) = at<Dword>(body, 116) = 0;
    at<float>(body, 112) = 1;
    at<std::uint16_t>(body, 596) = 0;
    kernel::memory_zero(bytes(body) + 604, 72);
    at<Dword>(body, 732) = 0;
    at<Byte>(body, 730) = 0;
    clear_ground_contact(bytes(body) + 508);
}

void refresh_car_track_point(void *car) {
    void *body = at<void *>(car, 1768);
    float position[] = {at<float>(body, 680), at<float>(body, 684), -at<float>(body, 688)};
    at<int>(car, 7984) = ai::lifecycle::pending::nearest_track_point(track_collision, position);
}
} // namespace nfs::game::runtime::physics
namespace nfs::game::runtime::ai::lifecycle::pending {

void position_entity(void *entity, const float *position, const float *rotation, Byte keep_path) {
    void *car = at<void *>(entity, 368);
    at<Dword>(entity, 392) = 0;
    void *body = invoke<void *>(entities::interface_at(entity, 3), 1);
    std::memcpy(bytes(body) + 680, position, 12);
    std::memcpy(bytes(body) + 692, rotation, 36);
    matrices::transpose(reinterpret_cast<const float *>(bytes(body) + 692),
                        reinterpret_cast<float *>(bytes(body) + 72));
    physics::reset_body_ground(body, 16);
    physics::clear_body_motion(body);
    physics::contacts_pending::clear_car_controls(car);
    if (!keep_path)
        clear_path(bytes(entity) + 220);
    invoke(entity, 31);
    physics::refresh_car_track_point(car);
}
} // namespace nfs::game::runtime::ai::lifecycle::pending
