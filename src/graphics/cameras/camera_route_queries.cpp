#include "camera_director_runtime.hpp"
#include "camera_route_math.hpp"
namespace nfs::game::world::camera {
using kernel::numeric::Extended;
Dword focused_car_index{};

void *focused_car() {
    return at<void *>(subsystem(68), 20 + focused_car_index * 4);
}

Dword top_mode(void *director) {
    return at<Dword>(director, 312 + at<Dword>(director, 356) * 4);
}

Extended route_lateral_position(void *entity) {
    alignas(4) unsigned char retained[10];
    route_math::lateral_native80(entity, retained);
    return Extended::from_native80(retained, kernel::numeric::current_format());
}

Extended route_heading_alignment(void *entity) {
    alignas(4) unsigned char retained[10];
    route_math::heading_native80(entity, retained);
    return Extended::from_native80(retained, kernel::numeric::current_format());
}
} // namespace nfs::game::world::camera
