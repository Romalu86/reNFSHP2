#pragma once
#include "../animation/animation_resource_runtime.hpp"
#include "nfs/resources/ini/ini.hpp"
#include "nfs/resources/files/file_cache.hpp"
#include "nfs/audio/vehicles/vehicle_audio.hpp"
#include "nfs/graphics/resources/eagl_graphics.hpp"
#include "nfs/core/frame/game_step.hpp"
#include "nfs/physics/contacts/vehicle_contacts.hpp"
#include "../../math/precision/extended_float.hpp"
#include "../../physics/bodies/shape_body_runtime.hpp"
namespace nfs::game::world::actor_resources {
void load_actor_sound_catalog();
void set_actor_identifier(void *, Dword);
void bind_default_actor_events();
int find_actor_sound(int);
void load_actor_definition(void *, const void *);
void configure_actor_behavior(void *, const char *, const char *, const char *, Dword, const void *);
void freeze_actor_channel_offsets(void *, Dword);
void set_channel_offset(void *, int, Dword);
void bind_path_cursor(void *, void *);
void locate_path_cursor(void *, const float *);
Dword advance_path_cursor(void *, float *, float, float *, float *);
float *actor_avoidance_direction(float *, void *);
Dword place_actor_on_ground(void *);
void add_actor_position(const float *, const float *, float *);
void update_actor_proximity_loop(void *);
void update_actor_proximity_transition(void *);
void update_actor_proximity_pose(void *);
void update_actor_proximity_motion(void *);
void update_actor_walking_path(void *);
void update_actor_continuous_path(void *);
} // namespace nfs::game::world::actor_resources
