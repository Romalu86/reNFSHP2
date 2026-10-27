#pragma once
#include "../../vehicles/rendering/vehicle_renderable_resources.hpp"
#include "nfs/core/lifecycle/object_state.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
namespace nfs::game::world::actor_resources {
// The actor loaders receive both triples as six saved stack words.
struct ActorVector {
    float x, y, z;
};
static_assert(sizeof(ActorVector) == 12);
using ActorAnimationCallback = void(__cdecl *)(void *, Dword);
using ActorFrameCallback = void(__cdecl *)(void *);
struct EventMapping {
    char *name;
    Dword identifier;
};
struct EventRegistry {
    std::uint16_t reserved, count;
    Dword unknown;
    EventMapping *mapping;
    char **names;
    int capacity, growth;
};
struct ActorEventStorage {
    Dword *targets[100];
    EventRegistry registry;
};
static_assert(offsetof(ActorEventStorage, registry) == 400);
extern ActorEventStorage actor_event_storage;
extern EventRegistry &animation_events;
extern void *event_actor;
extern Dword *(&actor_event_targets)[100];
void actor_displacement(void *, float *);
void actor_world_position(void *, float *);
void update_actor_pose(void *);
Byte advance_animation_clock(void *);
Byte update_actor_animation(void *);
void evaluate_actor_channel(void *, Dword, Dword *);
void update_actor_sound(int, Dword, const float *, const float *);
void submit_actor_sound(void *, int);
extern const Dword node_methods[2], animation_entry_methods[2], animation_table_methods[2],
    animation_reference_methods[1];
extern const Dword animation_bank_methods[8];
void *construct_node(void *);
void destroy_node(void *);
void load_animation_table(void *, const char *, Dword, Dword);
void destroy_animation_table(void *);
int __fastcall bank_animation_index(void *, void *, const char *);
Dword resolve_animation_symbol(const char *, Byte *);
void *create_actor(void *, const char *);
void *construct_actor(void *);
void destroy_actor(void *);
void bind_actor_resources(void *, const char *, const char *, const char *, Byte,
                          ActorAnimationCallback, ActorFrameCallback);
void *acquire_character(void *, const char *);
void *acquire_character_model(void *, const char *, const char *);
void *acquire_character_texture(void *, const char *, const char *, const char *);
void yaw_actor(void *, float);
float animation_duration(void *, Dword);
void reset_animation_clock(void *);
extern const Dword actor_methods[6], actor_reference_methods[2];
namespace pending {
void *create_runtime_animation(Dword);
void __fastcall draw_actor(void *, void *, Dword, Dword);
void __fastcall advance_actor(void *, void *);
} // namespace pending
} // namespace nfs::game::world::actor_resources
