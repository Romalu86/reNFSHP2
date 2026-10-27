#pragma once
#include "vehicle_renderable_resources.hpp"
#include "vehicle_attachment_transforms.hpp"
#include "../../graphics/animation/animation_resource_runtime.hpp"
#include "../../graphics/actors/model_render_runtime.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
#include "nfs/physics/frame/simulation_runtime.hpp"
#include "nfs/graphics/backend/eagl_backend.hpp"
#include "nfs/core/frame/game_step.hpp"
#include "nfs/graphics/skids/skid_lifecycle.hpp"
#include "nfs/network/packets/network_packet_handlers.hpp"
#include "../../math/precision/extended_float.hpp"
namespace nfs::game::world::camera {
void active_camera_position(void *, float *);
}
namespace nfs::game::world::pending {
void unload_vehicle_renderable(void *);
}
namespace nfs::game::runtime::physics {
void animate_trunk(void *, float);
void animate_hood(void *, float);
} // namespace nfs::game::runtime::physics
namespace nfs::game::world::actor_resources {
void compose_actor_transform(float *, const float *, const float *);
void set_channel_offset(void *, int, Dword);
} // namespace nfs::game::world::actor_resources
namespace nfs::game::world::vehicle_render {
void bind_renderable_car(void *, void *);
void prepare_renderable_frame(void *, void *, const float *, const float *);
Byte renderable_visible(void *, void *);
void renderable_half_extents(void *, float *);
void prepare_renderable_models(void *, void *);
void prepare_renderable_panels(void *);
void draw_renderable_lights(void *, void *);
void update_renderable_light_channels(void *);
void update_renderable_lod(void *, void *, const float *);
void smooth_renderable_damage(void *, Byte);
void release_preview_renderable(void *, void *);
void advance_damage_visibility(void *);
void evaluate_damage_pose(void *);
void draw_damage_bounds(void *, const float *, const float *);
void draw_damage_zone_bounds(void *, const float *, const float *);
void bind_driver_car(void *, void *);
void update_driver_animation(void *, Dword);
void draw_vehicle_driver(void *, void *, const float *, const float *);
void update_vehicle_light_positions(void *, const float *);
void draw_vehicle_lights(void *, void *, const float *);
void update_vehicle_shadow(void *);
void update_trunk_shadow(void *);
void update_hood_shadow(void *);
void update_shadow_attachments(void *);
void rotate_shadow_trunk(float *, float);
void rotate_shadow_hood(float *, float);
float *attachment_pose(void *, Dword);
void set_frontend_light(void *, int, const float *, Dword, const float *);
} // namespace nfs::game::world::vehicle_render
