#include "../animation/animation_resource_runtime.hpp"
#include "model_render_runtime.hpp"
#include "nfs/physics/frame/simulation_runtime.hpp"
#include "nfs/physics/frame/dynamics_runtime.hpp"
#include "nfs/world/resources/runtime_resources.hpp"
#include "actor_pose_math.hpp"
#include <cstring>
namespace nfs::game::world::actor_resources {
namespace {
const float handedness_mask[16] = {1,  1,  -1, 1,  1, 1, -1, 1,
                                   -1, -1, 1,  -1, 1, 1, -1, 1};
const float forward_vector[3] = {0, 0, 1};
const float *forward_direction{};
} // namespace

void initialize_actor_forward_direction() {
    forward_direction = forward_vector;
}

void compose_actor_transform(float *matrix, const float *rotation, const float *position) {
    for (unsigned row = 0; row < 3; ++row) {
        for (unsigned column = 0; column < 3; ++column)
            matrix[row * 4 + column] = rotation[row * 3 + column];
        matrix[row * 4 + 3] = 0;
    }
    for (unsigned column = 0; column < 4; ++column)
        matrix[12 + column] = position[column];
}

void bind_actor_model_matrix(void *model, const float *matrix) {
    auto *output = reinterpret_cast<Dword *>(offset(model, 12));
    auto *input = reinterpret_cast<const Dword *>(matrix);
    for (unsigned index = 0; index < 16; ++index)
        output[index] = input[index];
}

void multiply_matrix_components(float *matrix, const float *coefficients) {
    pending::apply_actor_matrix_coefficients(matrix, coefficients);
}

void append_actor_scale(float *matrix, float x, float y, float z, float w) {
    float scale[16];
    graphics::math::diagonal(scale, x, y, z, w);
    graphics::math::append(matrix, scale);
}
void prepend_actor_rotation(float *, float, float, float, float);

void update_actor_pose(void *actor) {
    float position[3];
    actor_world_position(actor, position);
    if (!at<Byte>(actor, 372)) {
        position[2] = -position[2];
        if (!at<Byte>(actor, 373)) {
            runtime::physics::surface_height(offset(actor, 180), position);
            pending::add_actor_ground_offset(position, actor);
        }
    }

    auto *actor_matrix = reinterpret_cast<float *>(offset(actor, 272));
    graphics::math::copy(
        actor_matrix, reinterpret_cast<const float *>(vehicle_render::animation_matrix_scratch()));
    multiply_matrix_components(actor_matrix, handedness_mask);
    const float rotation_degrees = pending::actor_heading_degrees(actor);
    prepend_actor_rotation(actor_matrix, rotation_degrees, 0, 1, 0);
    append_actor_scale(actor_matrix, 1, 1, -1, 1);
    actor_matrix[12] = position[0];
    actor_matrix[13] = position[1];
    actor_matrix[14] = position[2];

    // Use the actor transform computed above for the pose matrix.
     
    float pose_matrix[16], combined[16];
    graphics::math::copy(pose_matrix, actor_matrix);
    graphics::math::multiply(
        reinterpret_cast<const float *>(vehicle_render::animation_matrix_scratch()), pose_matrix,
        combined);
    graphics::math::copy(pose_matrix, combined);

    float direction[3];
    graphics::math::transform_position(forward_direction, pose_matrix, direction);
    at<Dword>(actor, 152) = 0;
    runtime::normalize_vector(direction, reinterpret_cast<float *>(offset(actor, 148)));

    const float up[3] = {0, 1, 0};
    float up_direction[3];
    graphics::math::transform_position(up, pose_matrix, up_direction);
    runtime::normalize_vector(up_direction, reinterpret_cast<float *>(offset(actor, 160)));

    const auto *animation = reinterpret_cast<const char *>(offset(at<void *>(actor, 384), 40));
    if (!std::strncmp(animation, "Cam", 3))
        graphics::math::copy(reinterpret_cast<float *>(offset(subsystem(48), 16)), actor_matrix);
}

void __fastcall pending::draw_actor(void *actor, void *, Dword view, Dword flags) {
    if (!at<Byte>(actor, 84) || at<Byte>(actor, 60))
        return;
    if (void *child = at<void *>(actor, 264))
        world::invoke(child, 2, view, flags);
    void *model_owner = at<void *>(actor, 88);
    if (!model_owner)
        return;
    Byte cached = at<Byte>(actor, 373);
    if (!cached) {
        vehicle_render::restore_skeleton_pose(at<void *>(model_owner, 24),
                                              vehicle_render::animation_pose_scratch(), nullptr);
        if (!cached && at<Dword>(actor, 384)) {
            evaluate_actor_channel(offset(actor, 52), 0,
                                   vehicle_render::animation_pose_scratch());
            evaluate_actor_channel(offset(actor, 52), 1,
                                   vehicle_render::animation_pose_scratch());
        }
    }
    Dword *pose = vehicle_render::animation_pose_scratch();
    pose[10] = 0;
    pose[9] = 0;
    pose[8] = 0;
    float position[4];
    actor_world_position(actor, position);
    float matrix[16];
    if (at<Byte>(actor, 372)) {
        position[3] = 1;
        compose_actor_transform(matrix, reinterpret_cast<float *>(offset(actor, 336)), position);
    } else {
        position[2] = -position[2];
        if (!at<Byte>(actor, 373)) {
            runtime::physics::surface_height(offset(actor, 180), position);
            pending::add_actor_ground_offset(position, actor);
        }
        graphics::math::translation(matrix, position[0], position[1], -position[2]);
        float scale[16], product[16];
        float size = at<float>(actor, 120);
        graphics::math::diagonal(scale, size, size, size, 1);
        float orientation[16] = {at<float>(actor, 144),  0, at<float>(actor, 136), 0, 0, 1, 0, 0,
                                 -at<float>(actor, 136), 0, at<float>(actor, 144), 0, 0, 0, 0, 1};
        graphics::math::multiply(orientation, matrix, product);
        graphics::math::copy(matrix, product);
        graphics::math::multiply(scale, matrix, product);
        graphics::math::copy(matrix, product);
    }
    vehicle_render::evaluate_skeleton(at<void *>(actor, 88),
                                      vehicle_render::animation_pose_scratch());
    void *model = at<void *>(at<void *>(actor, 392), 128);
    bind_actor_model_matrix(model, matrix);
    if (std::strncmp(reinterpret_cast<const char *>(offset(at<void *>(actor, 384), 40)), "Cam",
                     3))
        vehicle_render::pending::draw_model(model,
                                            reinterpret_cast<const Dword *>(offset(model, 12)));
    update_actor_pose(actor);
}
} // namespace nfs::game::world::actor_resources
#pragma section(".CRT$XCU_NFS_006902CC", read)
extern "C" __declspec(allocate(".CRT$XCU_NFS_006902CC")) void(__cdecl *const nfs_actor_direction_startup)() =
    nfs::game::world::actor_resources::initialize_actor_forward_direction;
#pragma comment(linker, "/include:_nfs_actor_direction_startup")
