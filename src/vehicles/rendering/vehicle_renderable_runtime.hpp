#pragma once
#include "nfs/world/lifecycle/world_dependency.hpp"
namespace nfs::game::world::vehicle_render {
void *construct_actor_channels(void *);
void destroy_actor_channels(void *);
void *construct_damage_owner(void *);
void destroy_damage_owner(void *);
void *construct_light_owner(void *);
void destroy_light_owner(void *);
void *construct_driver_owner(void *);
void destroy_driver_owner(void *);
void destroy_animation_channel(void *);
void *construct_attachment_owner(void *);
void destroy_attachment_owner(void *);
void *construct_attachment_pose(void *);
void destroy_attachment_pose(void *);
void *construct_body_renderer(void *);
void destroy_body_renderer(void *);
void *construct_body_interface(void *);
void destroy_body_interface(void *);
void *construct_shadow_owner(void *);
void destroy_shadow_owner(void *);
extern const Dword body_interface_methods[6], body_renderer_methods[6];
namespace pending {
void __fastcall load_body_models(void *, void *, Dword *, Dword, Dword, Dword, Dword,
                                 Dword);
void __fastcall prepare_body_shader(void *, void *, const Dword *, const Dword *);
} // namespace pending
} // namespace nfs::game::world::vehicle_render
