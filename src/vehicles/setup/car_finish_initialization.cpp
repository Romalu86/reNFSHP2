#include "car_initialization_runtime.hpp"
#include <cmath>
namespace nfs::game::world::pending {

void finish_car_initialization(void *car) {
    reset_car_start_position(car);
    vehicle_setup::initialize_derived_parameters(car);
    vehicle_setup::initialize_tire_geometry(car);
    for (int gear = 0; gear < 8; ++gear) {
        if (gear < 2)
            at<float>(car, 852 + gear * 4) = -10000;
        else if (gear < int(at<Dword>(car, 780) - 1))
            at<float>(car, 852 + gear * 4) = float(vehicle_setup::gear_shift_speed(car, gear));
        else
            at<float>(car, 852 + gear * 4) = 12000;
    }
    vehicle_setup::initialize_drive_weights(car);
    const auto format = kernel::numeric::current_format();
    for (unsigned wheel = 0; wheel < 4; ++wheel) {
        const auto angle =
            kernel::numeric::Extended(at<float>(car, 1968 + wheel * 296), format) *
            6.283185482025146484375f;
        // Discard the sine results while retaining the floating-point operations.
        std::sin(double(angle));
    }
}
} // namespace nfs::game::world::pending
