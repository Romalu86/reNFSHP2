#include "nfs/world/lifecycle/world_lifecycle.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
#include "nfs/core/lifecycle/system_runtime.hpp"
namespace nfs::lifecycle::table_entries {
void __fastcall enter_vehicle_frontend(System *owner, void *) {
    game::world::enter_vehicle_frontend(owner);
}
void __fastcall leave_vehicle_frontend(System *owner, void *) {
    game::world::leave_vehicle_frontend(owner);
}
void __fastcall load_vehicle_level(System *owner, void *) {
    game::world::load_vehicle_level(owner);
}
void __fastcall unload_vehicle_level(System *owner, void *) {
    game::world::unload_vehicle_level(owner);
}
void __fastcall reset_vehicle_level(System *owner, void *) {
    game::world::reset_vehicle_level(owner);
}
void __fastcall initialize_vehicle_catalog(System *owner, void *) {
    game::world::initialize_vehicle_catalog(owner);
}
void __fastcall initialize_track_catalog(System *owner, void *) {
    game::world::initialize_track_catalog(owner);
}
void __fastcall load_track_manager_level(System *owner, void *) {
    game::world::load_track_manager_level(owner);
}
void __fastcall unload_track_manager_level(System *owner, void *) {
    game::world::unload_track_manager_level(owner);
}
void __fastcall load_level_resources(System *owner, void *) {
    game::world::load_level_resources(owner);
}
void __fastcall unload_level_resources(System *owner, void *) {
    game::world::unload_level_resources(owner);
}
void __fastcall load_track_level(System *owner, void *) {
    game::world::load_track_level(owner);
}
void __fastcall unload_track_level(System *owner, void *) {
    game::world::unload_track_level(owner);
}
void __fastcall reset_track_level(System *owner, void *) {
    game::world::reset_track_level(owner);
}
void __fastcall load_car_level(System *owner, void *) {
    game::world::load_car_level(owner);
}
void __fastcall unload_car_level(System *owner, void *) {
    game::world::unload_car_level(owner);
}
void __fastcall reset_car_level(System *owner, void *) {
    game::world::reset_car_level(owner);
}
void __fastcall enter_car_frontend(System *owner, void *) {
    game::world::enter_car_frontend(owner);
}
void __fastcall enter_camera_frontend(System *owner, void *) {
    game::world::enter_camera_frontend(owner);
}
void __fastcall load_camera_level(System *owner, void *) {
    game::world::load_camera_level(owner);
}
void __fastcall unload_camera_level(System *owner, void *) {
    game::world::unload_camera_level(owner);
}
void __fastcall reset_camera_level(System *owner, void *) {
    game::world::reset_camera_level(owner);
}
void __fastcall initialize_characters(System *owner, void *) {
    game::world::initialize_characters(owner);
}
void __fastcall shutdown_characters(System *owner, void *) {
    game::world::shutdown_characters(owner);
}
void __fastcall load_character_level(System *owner, void *) {
    game::world::load_character_level(owner);
}
void __fastcall unload_character_level(System *owner, void *) {
    game::world::unload_character_level(owner);
}
void __fastcall reset_character_level(System *owner, void *) {
    game::world::reset_character_level(owner);
}
void __fastcall enter_character_frontend(System *owner, void *) {
    game::world::enter_character_frontend(owner);
}
void __fastcall leave_character_frontend(System *owner, void *) {
    game::world::leave_character_frontend(owner);
}
void __fastcall load_start_line(System *owner, void *) {
    game::world::load_start_line(owner);
}
} // namespace nfs::lifecycle::table_entries
namespace nfs::game::runtime::pending {
void prepare_start_line_entry(Dword *owner) {
    world::prepare_start_line(owner);
}
void prepare_start_line_object_entry(Dword *owner) {
    world::prepare_start_line_object(owner);
}
} // namespace nfs::game::runtime::pending
namespace nfs::game::runtime::track::pending {
void advance_replay(Dword *owner, int player) {
    world::advance_replay_track(owner, player);
}
void unload_light_glows_entry(Dword *owner) {
    world::unload_light_glows(owner);
}
void bind_light_glows_entry(Dword *owner, void *definitions, Dword textures) {
    world::bind_light_glows(owner, static_cast<Dword *>(definitions),
                            reinterpret_cast<Dword *>(textures));
}
} // namespace nfs::game::runtime::track::pending
