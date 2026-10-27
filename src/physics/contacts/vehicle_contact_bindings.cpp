#include "nfs/physics/contacts/vehicle_contacts.hpp"
// Forward vehicle-contact entry points to the shared implementation.
 
namespace nfs::game::runtime::pending {
void reset_car_entry(Dword *car) {
    physics::reset_car(car);
}
void clear_car_skids_entry(Dword *car) {
    physics::clear_car_skids(car);
}
void update_car_reset_entry(Dword *car) {
    physics::update_car_reset(car);
}
void update_car_effects_entry(Dword *car) {
    physics::update_car_effects(car);
}
} // namespace nfs::game::runtime::pending
namespace nfs::game::runtime::physics::pending {
void begin_track_contacts_entry(Dword *owner) {
    begin_track_contacts(owner);
}
void update_track_car_entry(void *owner) {
    update_track_car(owner);
}
void reset_track_vehicle_entry(void *owner) {
    reset_track_vehicle(owner);
}
void update_contacts_entry(void *body, Byte remote) {
    update_contacts(body, remote);
}
Dword collide_wheels_entry(void *car) {
    return collide_wheels(car);
}
Byte sample_ground_entry(const float *position, void *contact) {
    return sample_ground(position, contact);
}
Byte locate_ground_entry(void *contact, float *position, float *height) {
    return locate_ground(contact, position, height);
}
Dword interpolate_vehicle_entry(Dword *, float seconds, void *car) {
    return interpolate_vehicle(seconds, car);
}
} // namespace nfs::game::runtime::physics::pending
namespace nfs::game::runtime::track::pending {
long double vehicle_direction_entry(void *vehicle) {
    return vehicle_direction(vehicle);
}
void path_direction_entry(void *path, int index, float *output) {
    path_direction(path, index, output);
}
} // namespace nfs::game::runtime::track::pending
