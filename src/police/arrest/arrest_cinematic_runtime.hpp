#pragma once
#include "../../graphics/actors/character_runtime.hpp"
#include "../pursuit/chase_runtime.hpp"
#include "../../graphics/cameras/camera_director_runtime.hpp"
#include "nfs/resources/archives/archive_cache.hpp"
#include "nfs/world/lifecycle/world_dependency.hpp"
namespace nfs::game::world::actor_resources {
void actor_axis_rotation(float *, float, float, float, float);
void load_arrest_actor(void *, const char *, Dword, ActorVector, ActorVector, const char *);
void load_arrest_actors(void *);
void allocate_actor_sequences(void *, int);
void begin_arrest_cinematic(void *, int, Dword, void *);
void *find_actor_by_name(void *, const char *);
void *find_system_actor(void *, const char *);
void unload_renderable_driver(void *);
void unload_vehicle_drivers(void *);
} // namespace nfs::game::world::actor_resources
namespace nfs::game::world::camera {
void begin_arrest_camera(void *);
void end_arrest_camera(void *);
} // namespace nfs::game::world::camera
namespace nfs::application::pending {
void set_letterbox_visible(kernel::Dword, void *);
}
namespace nfs::game::runtime::ai::lifecycle::pending {
Byte can_finish_chase(void *);
Dword car_speech_mask(int);
void end_arrest_cinematic(void *, void *);
void __cdecl arrest_cinematic_complete(void *);
int choose_arrest_marker(void *);
} // namespace nfs::game::runtime::ai::lifecycle::pending
