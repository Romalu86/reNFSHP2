#include "server_runtime.hpp"
#include "nfs/world/lifecycle/world_lifecycle.hpp"
namespace nfs::application::pending {

void set_network_camera_overlay(void *camera, Byte enabled) {
    if (enabled) {
        if (at<Dword>(camera, 356))
            game::world::end_camera_mode(camera,
                                         at<Dword>(camera, 312 + at<Dword>(camera, 356) * 4));
        if (at<int>(camera, 356) < 10)
            ++at<Dword>(camera, 356);
        at<Dword>(camera, 312 + at<Dword>(camera, 356) * 4) = 11;
        game::world::begin_camera_mode(camera,
                                       at<Dword>(camera, 312 + at<Dword>(camera, 356) * 4));
        game::world::notify_camera_mode(camera);
        return;
    }
    if (game::world::find_camera_mode(camera, 11) &&
        at<Dword>(camera, 312 + at<Dword>(camera, 356) * 4) == 11) {
        game::world::end_camera_mode(camera, at<Dword>(camera, 312 + at<Dword>(camera, 356) * 4));
        --at<Dword>(camera, 356);
        if (at<Dword>(camera, 356))
            game::world::begin_camera_mode(camera,
                                           at<Dword>(camera, 312 + at<Dword>(camera, 356) * 4));
        game::world::notify_camera_mode(camera);
    }
}
} // namespace nfs::application::pending
