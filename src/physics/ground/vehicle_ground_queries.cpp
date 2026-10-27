#include "nfs/physics/contacts/vehicle_contacts.hpp"
#include "../../math/precision/extended_float.hpp"
#include "../motion/world_motion_x87.hpp"
namespace nfs::game::runtime::track {
// Keep the public compatibility aggregate wholly in MSVC.
kernel::numeric::Extended retained_vehicle_direction(void *vehicle) {
    unsigned char value[10];
    x87::vehicle_direction_native80(vehicle, value);
    return kernel::numeric::Extended::from_native80(value, kernel::numeric::current_format());
}
long double vehicle_direction(void *vehicle) {
    return double(retained_vehicle_direction(vehicle));
}
} // namespace nfs::game::runtime::track
