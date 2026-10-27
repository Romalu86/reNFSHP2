#include "car_asset_parameters.hpp"
#include "../../physics/vehicles/car_construction_runtime.hpp"
#include "nfs/network/session/network_runtime.hpp"
namespace nfs::game::world::vehicle_setup {

void register_car_role(void *car) {
    void *manager = subsystem(68);
    ++at<Dword>(manager, 4);
    ++at<Dword>(manager, 8);
    at<Dword>(car, 1776) = Dword(-1);
    at<Dword>(car, 1788) = 1;
    if (network::session_active() &&
        (!at<Dword>(subsystem(112), 68) ||
         at<std::int16_t>(subsystem(112), 378) != at<std::int16_t>(car, 1772))) {
        at<Dword>(car, 1788) = 9;
        return;
    }
    const Dword index = at<Dword>(car, 1772);
    void *record = index < 24 ? bytes(subsystem(72)) + 248 + index * 84 : nullptr;
    manager = subsystem(68);
    if (at<Dword>(record, 0)) {
        const Dword local = at<Dword>(manager, 16);
        at<Dword>(car, 1776) = at<Dword>(car, 1780) = at<Dword>(car, 1784) = local;
        at<Dword>(car, 1788) = 5;
        at<Dword>(manager, 16) = local + 1;
    } else {
        at<Dword>(car, 1788) = 3;
        ++at<Dword>(manager, 12);
    }
}
// RET8 includes the unused resource-name stack word.
Dword copy_network_car_parameters(void *network, void *car, const char *) {
    const int local = at<std::int16_t>(network, 378);
    if (local < 0)
        return 0;
    void *local_car = at<void *>(subsystem(68), 20 + Dword(local) * 4);
    if (!local_car || !car || at<Dword>(local_car, 1772) != at<Dword>(car, 1772) ||
        !at<Dword>(network, 2092))
        return 0;
    for (unsigned word = 0; word < 393; ++word)
        at<Dword>(car, 4 + word * 4) = at<Dword>(network, 520 + word * 4);
    return 1;
}
} // namespace nfs::game::world::vehicle_setup
namespace nfs::game::world::pending {

void load_car_assets(void *car, const char *name, Dword kind, Dword automatic) {
    vehicle_setup::register_car_role(car);
    at<void *>(car, 1768) = runtime::physics::reserve_vehicle();
    if ((!at<Dword>(subsystem(112), 68) ||
         !vehicle_setup::copy_network_car_parameters(subsystem(112), car, name)) &&
        !vehicle_setup::load_physical_parameters(car, name, kind, automatic)) {
        const char *gearbox = Byte(automatic) ? "Auto" : "Manual";
        (void)gearbox;
        runtime::particles::lifecycle::pending::diagnostic_message(
            "Vehicle %s (%d,%s) has a fatal physic problem!");
    }
    // All four initializers write the same fields and leave wheel-structure padding untouched.
     
    for (Dword wheel = 0; wheel < 4; ++wheel) {
        const Dword tire = wheel < 2 ? 912 : 1188;
        const Dword brake = wheel < 2 ? 1464 : 1480;
        runtime::physics::construction::initialize_wheel_state(
            bytes(car) + 1948 + wheel * 296, bytes(car) + tire, bytes(car) + brake, wheel, car);
    }
}
} // namespace nfs::game::world::pending
