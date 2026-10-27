#include "nfs/ai/lifecycle/ai_lifecycle.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
#include "nfs/graphics/resources/eagl_graphics.hpp"
#include "nfs/math/vectors/vector_arrays.hpp"
#include "../../math/vectors/retained_vector.hpp"
#include <cmath>
#include <cstring>
#include <initializer_list>

namespace nfs::game::runtime::ai::lifecycle {

void update_collision_vehicle(Dword *entity) {
    using kernel::numeric::Extended;
    const auto number = [](float value) {
        return Extended(value, kernel::numeric::current_format());
    };
    const Dword *offsets = at<const Dword *>(entity, 320);
    void *physical_interface = bytes(entity) + 320 + offsets[1];
    void *body = invoke<void *>(physical_interface, 1);
    constexpr float step = 0.01666666753590106964111328125f;
    const float pitch = float(number(at<float>(body, 20)) * step + at<float>(entity, 384));
    at<float>(entity, 384) = pitch;
    const auto yaw = number(at<float>(body, 24)) * step + at<float>(entity, 388);
    at<float>(entity, 388) = float(yaw);
    const float roll = float(number(at<float>(body, 28)) * step + at<float>(entity, 392));
    at<float>(entity, 392) = roll;
    // The FST to +184 does not round the ST0 value used by either wrap test.
    if (!(yaw < number(0.5f)) && !std::isnan(float(yaw)))
        at<float>(entity, 388) = float(yaw - 1.0f);
    else if (yaw < number(-0.5f))
        at<float>(entity, 388) = float(yaw + 1.0f);
    float pitch_matrix[9], yaw_matrix[9], roll_matrix[9], yaw_pitch[9], rotation[9], reverse[9],
        pose[9];
    matrices::rotate_x(pitch_matrix, pitch);
    matrices::rotate_y(yaw_matrix, at<float>(entity, 388));
    matrices::rotate_z(roll_matrix, roll);
    matrices::multiply(yaw_matrix, pitch_matrix, yaw_pitch);
    matrices::multiply(yaw_pitch, roll_matrix, rotation);
    matrices::rotate_y(reverse, 0.5f);
    matrices::multiply(rotation, reverse, pose);
    std::memcpy(bytes(body) + 692, pose, 36);
    matrices::transpose(reinterpret_cast<float *>(bytes(body) + 692),
                        reinterpret_cast<float *>(bytes(body) + 72));
    matrices::transpose(pose, invoke<float *>(entity, 25));
    float local_velocity[3];
    std::memcpy(local_velocity, invoke<const float *>(entity, 21), 12);
    float *velocity = reinterpret_cast<float *>(bytes(body) + 8);
    float transformed_velocity[3];
    matrices::transform_vectors(1, local_velocity, yaw_matrix, transformed_velocity);
    at<Dword>(body, 8) = at<Dword>(transformed_velocity, 0);
    at<Dword>(body, 12) = at<Dword>(transformed_velocity, 4);
    at<Dword>(body, 16) = at<Dword>(transformed_velocity, 8);
    float path_pose[21];
    sample_path(bytes(entity) + 220, 0, path_pose);
    const float *position = invoke<const float *>(entity, 0);
    float correction[3];
    subtract_constant_vector_array(1, path_pose, position, correction);
    const auto along = retained_dot_product(correction, path_pose + 3);
    correction[0] = float(number(correction[0]) - number(path_pose[3]) * along);
    correction[2] = float(number(path_pose[5]) * along - number(correction[2]));
    velocity[0] = float(number(4.0f) * correction[0] + velocity[0]);
    velocity[2] = float(number(4.0f) * correction[2] + velocity[2]);
    float displacement[3];
    scale_vectors(1, velocity, step, displacement);
    add_constant_vector_array(1, displacement, reinterpret_cast<float *>(bytes(body) + 680),
                              displacement);
    std::memcpy(bytes(body) + 680, displacement, 12);
    pending::synchronize_track_vehicle(at<void *>(entity, 368));
    const auto rotor = number(pending::helicopter_rotor_increment) + at<float>(entity, 1656);
    at<float>(entity, 1656) = float(rotor);
    if (!(rotor < number(360.0f)) && !std::isnan(float(rotor)))
        at<float>(entity, 1656) = float(rotor - 360.0f);
    pending::update_helicopter_attachments(entity);
    pending::update_helicopter_rendering(entity);
    if (graphics::unresolved::camera_state & 1) {
        for (unsigned offset : {372u, 376u}) {
            void *attachment = at<void *>(entity, offset);
            if (attachment && at<void *>(attachment, 80))
                at<Byte>(at<void *>(attachment, 80), 729) = 1;
        }
        float sound_position[3];
        std::memcpy(sound_position, bytes(body) + 680, 12);
        pending::position_helicopter_sound(sound_position, velocity);
        if (!physics::pending::locate_ground_entry(
                bytes(body) + 508, reinterpret_cast<float *>(bytes(body) + 680),
                reinterpret_cast<float *>(bytes(body) + 484)))
            at<std::uint16_t>(body, 584) = 0;
    }
}
} // namespace nfs::game::runtime::ai::lifecycle
namespace nfs::game::runtime::ai::pending {
void update_collision_vehicle_entry(Dword *entity) {
    lifecycle::update_collision_vehicle(entity);
}
} // namespace nfs::game::runtime::ai::pending
