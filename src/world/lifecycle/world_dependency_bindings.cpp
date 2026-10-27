#include "nfs/world/lifecycle/world_dependency.hpp"

// Forward world-lifecycle calls to the shared implementations.
 
namespace nfs::game::world::pending {
void initialize_animation_factory_entry(Dword bytes, Dword) {
    dependency::initialize_animation_factory(bytes);
}
void shutdown_animation_factory_entry() {
    dependency::shutdown_animation_factory();
}
void reset_animation_scratch_entry() {
    dependency::reset_animation_scratch();
}
void release_vehicle_character_bindings_entry(void *owner) {
    dependency::release_vehicle_character_bindings(owner);
}
void load_vehicle_shared_textures(void *owner) {
    dependency::load_shared_vehicle_textures(owner);
}
void destroy_car(void *car) {
    dependency::release_car(car);
}
void register_car_particles(void *car) {

    dependency::bind_car_client(at<void *>(car, 7988), car);
}
Dword bind_vehicle_slot_entry(void *owner, Dword catalog, Dword variant) {
    return dependency::bind_vehicle_slot(owner, catalog, variant);
}
void initialize_track_objects_entry() {
    dependency::initialize_track_objects();
}
void release_track_objects_entry() {
    dependency::release_track_objects();
}
void release_track_sky_entry() {
    dependency::release_track_sky();
}
void release_compartment_entry(void *owner) {
    dependency::release_compartment(owner);
}
void unload_compartment_directory_entry(void *owner) {
    dependency::unload_compartment_directory(owner);
}
void bind_track_texture_state_entry(void *owner) {
    dependency::bind_track_texture_state(owner);
}
Dword legacy_clock_entry() {
    return dependency::legacy_clock();
}
void initialize_boom_entry(void *owner) {
    dependency::initialize_boom(owner);
}
} // namespace nfs::game::world::pending
