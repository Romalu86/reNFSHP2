#include "nfs/physics/contacts/contact_dependencies.hpp"
#include <cstring>

namespace nfs::game::runtime::physics::contacts_pending {
void clear_car_controls(void *car) {
    contact::reset_controls(car);
}

Byte search_ground_cell(void *cell, void *compartment, const float *position, void *ground,
                        Byte surface) {
    return contact::search_ground(cell, compartment, position, ground, surface);
}

int apply_body_contact(void *body, const float *normal, const float *point, float travel,
                       Dword first_friction, Dword second_friction, Dword maximum_height_bits,
                       Dword flags) {
    float maximum_height;
    std::memcpy(&maximum_height, &maximum_height_bits, sizeof(maximum_height));
    return contact::apply_impulse(body, normal, point, travel, first_friction, second_friction,
                                  maximum_height, flags);
}
} // namespace nfs::game::runtime::physics::contacts_pending

namespace nfs::game::runtime::pending {
void position_emitter_sound_entry(Dword definition, Dword serial, const float *position,
                                     const float *velocity) {
    race_audio::position_emitter_sound(definition, serial, position, velocity);
}
} // namespace nfs::game::runtime::pending
