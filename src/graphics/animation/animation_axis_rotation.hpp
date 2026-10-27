#pragma once
#include "animation_channel_values.hpp"
namespace nfs::game::world::actor_resources::animation_runtime {
struct AxisRotationDescriptor {
    float first_angle, second_angle, first_offset, second_offset, first_scale, second_scale;
    Byte axis;
};
static_assert(offsetof(AxisRotationDescriptor, axis) == 24);
void __fastcall decode_axis_rotation_descriptor(const std::uint16_t *, void *,
                                                AxisRotationDescriptor *);
void quaternion_from_rotation_angles(const float *, float *);
void compose_x_axis_rotation(const float *, const float *, float *);
void compose_y_axis_rotation(const float *, const float *, const float *, float *);
void compose_z_axis_rotation(const float *, const float *, float *);
extern float rotation_angle_span;
} // namespace nfs::game::world::actor_resources::animation_runtime
