#include "ai_car_behavior.hpp"
#include "../../graphics/cameras/camera_director_runtime.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
#include "../../math/vectors/retained_vector.hpp"
namespace nfs::game::runtime::ai::cars {
// AL is the saved result consumed by callers.
Dword __fastcall car_try_recovery_steering(void *entity, void *, Dword controls_word,
                                          Dword speed_word) {
    void *car = at<void *>(entity, 368);
    float pose[21];
    lifecycle::sample_path(bytes(entity) + 220, 0, pose);
    if (at<int>(entity, 240) == -1)
        dependencies::reverse_section_pose(pose);
    pose[11] = -pose[11];
    pose[8] = -pose[8];
    float forward[3];
    std::memcpy(forward, bytes(at<void *>(car, 1768)) + 716, 12);
    const float across = float(-runtime::retained_dot_product(forward, pose + 9));
    const float alignment = float(runtime::retained_dot_product(forward, pose + 6));
    const float lateral = float(game::world::camera::route_lateral_position(entity));
    if (alignment > 0.5f)
        return 0;
    void *controls = reinterpret_cast<void *>(controls_word);
    const float speed = float_word(speed_word);
    auto command = [&](Byte steering, float target_speed) -> Dword {
        at<Byte>(controls, 4) = steering;
        invoke(entity, 33, target_speed, controls_word);
        return 1;
    };
    if (alignment <= -0.7070000171661376953125f) {
        if (lateral < -0.25f)
            return command(129, -speed);
        if (lateral > 0.25f)
            return command(127, -speed);
        if (lateral > 0.f)
            return command(127, speed);
        if (lateral < 0.f)
            return command(129, speed);
    }
    if (lateral > 0.5f && across >= 0.f)
        return command(127, -speed);
    if (lateral < -0.5f && across <= 0.f)
        return command(129, -speed);
    if (across >= 0.f && lateral < -0.5f)
        return command(129, speed);
    if (across <= 0.f && lateral > 0.5f)
        return command(127, speed);
    if (lateral > -0.5f && across <= 0.f)
        return command(129, -speed);
    if (lateral < 0.5f && across >= 0.f)
        return command(127, -speed);
    return 0;
}
} // namespace nfs::game::runtime::ai::cars
