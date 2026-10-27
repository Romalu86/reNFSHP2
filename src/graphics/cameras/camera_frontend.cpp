#include "camera_factory_support.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
#include <cstdlib>
namespace nfs::game::runtime::physics {
void *construct_vehicle(void *);
void destroy_vehicle(void *);
} // namespace nfs::game::runtime::physics
namespace nfs::game::world::camera {
alignas(4) Byte frontend_body[736]{};
Byte frontend_body_guard{};

void destroy_frontend_body() {
    runtime::physics::destroy_vehicle(frontend_body);
}
} // namespace nfs::game::world::camera
namespace nfs::game::world::pending {

void initialize_frontend_camera(void *director) {
    using namespace game::world;
    using namespace game::world::camera;
    void *controller = allocate_controller(director, 10);
    void *position = allocate_behavior(112, "behavior - position pause", construct_pause_position);
    if (!(frontend_body_guard & 1)) {
        frontend_body_guard |= 1;
        game::runtime::physics::construct_vehicle(frontend_body);
        std::atexit(destroy_frontend_body);
    }
    at<void *>(position, 24) = frontend_body;
    at<float>(position, 12) = 4.5f;
    at<float>(position, 16) = 0;
    at<float>(position, 20) = 7.88000011444091796875f;
    at<void *>(controller, 80) = position;
    void *look = allocate_behavior(8, "behavior - look at point", construct_point_look);
    at<const float *>(look, 4) = game::runtime::saved::zero_vector;
    at<void *>(controller, 84) = look;
    at<float>(controller, 36) = at<float>(controller, 32) = 50.f;
    at<Dword>(controller, 20) = at<Dword>(controller, 24) = at<Dword>(controller, 28) = 0;
    at<float>(controller, 56) = 1.f;
    at<float>(controller, 60) = 10000.f;
    append_camera_mode(director, controller);
    push_mode(director, 10);
}
} // namespace nfs::game::world::pending
