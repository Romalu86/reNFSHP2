#include "ai_car_behavior.hpp"
#include "../../physics/bodies/shape_body_runtime.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
#include "nfs/physics/frame/dynamics_runtime.hpp"
#include "nfs/physics/contacts/vehicle_contacts.hpp"
namespace nfs::game::runtime::ai::cars {

Dword __fastcall car_recover_ground(void *entity, void *) {
    void *car = at<void *>(entity, 368);
    float pose[21];
    lifecycle::sample_path(bytes(entity) + 220, 1, pose);
    float position[3] = {0.f, 0.f, 0.f};
    using kernel::numeric::Extended;
    const auto format = kernel::numeric::current_format();
    auto E = [&](float value) { return Extended(value, format); };
    const Dword kind = invoke<Dword>(entities::interface_at(entity, 1), 0);
    float x, y, z;
    if (!kind) {
        x = float(E(pose[0]) + pose[18]);
        y = float(E(pose[1]) + pose[19]);
        z = float(E(pose[2]) + pose[20]);
    } else {
        y = pose[1];
        x = pose[0];
        z = pose[2];
    }
    position[2] = -z;
    position[1] = float(E(y) + 1.5f);
    position[0] = x;
    alignas(4) Byte ground[84];
    physics::clear_ground_contact(ground);
    if (!physics::surface_height(ground, position))
        return 0;
    float heading[3] = {pose[3], 0.f, -pose[5]};
    const long double length = physics::pending::normalize_collision_entry(heading);
    if (length == 0)
        return 0;
    if (at<int>(entity, 240) < 0)
        for (unsigned axis = 0; axis < 3; ++axis)
            heading[axis] = float(E(heading[axis]) * -1.f);
    float rotation[9];
    rotation[2] = -heading[0];
    rotation[8] = heading[2];
    rotation[0] = heading[2];
    rotation[6] = heading[0];
    void *body = at<void *>(car, 1768);
    at<Byte>(body, 728) = 1;
    at<Byte>(body, 729) = 1;
    position[1] = float(E(position[1]) + 1.5f);
    body = at<void *>(car, 1768);
    std::memcpy(bytes(body) + 680, position, 12);
    rotation[1] = 0.f;
    rotation[3] = 0.f;
    rotation[4] = 1.f;
    rotation[5] = 0.f;
    rotation[7] = 0.f;
    std::memcpy(bytes(body) + 692, rotation, 36);
    matrices::transpose(reinterpret_cast<const float *>(bytes(body) + 692),
                        reinterpret_cast<float *>(bytes(body) + 72));
    physics::reset_car_motion(static_cast<Dword *>(car));
    const float speed = at<float>(entity, 400);
    lifecycle::pending::set_car_speed(car, speed > 0.f ? speed : 0.f);
    return 1;
}

Dword __fastcall autonomous_cop_recover_ground(void *entity, void *) {
    if (!at<Byte>(entity, 576))
        return car_recover_ground(entity, nullptr);
    void *body = invoke<void *>(entities::interface_at(entity, 3), 1);
    float position[3];
    std::memcpy(position, bytes(body) + 680, 12);
    if (!physics::surface_height(bytes(body) + 508, position))
        return 0;
    at<Byte>(body, 728) = 1;
    at<Byte>(body, 729) = 1;
    return 1;
}
} // namespace nfs::game::runtime::ai::cars
