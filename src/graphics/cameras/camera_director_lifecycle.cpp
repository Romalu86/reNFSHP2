#include "camera_director_runtime.hpp"
#include "nfs/world/lifecycle/world_dependency.hpp"
#include "nfs/network/session/network_runtime.hpp"
#include "nfs/gui/engine/gui_engine.hpp"
namespace nfs::application::pending {
void create_network_camera(void *);
}
namespace nfs::game::world::camera {
int jump_variant{};

Dword pop_mode(void *director) {
    end_camera_mode(director, at<Dword>(director, 312 + at<Dword>(director, 356) * 4));
    --at<Dword>(director, 356);
    if (at<Dword>(director, 356))
        begin_camera_mode(director, at<Dword>(director, 312 + at<Dword>(director, 356) * 4));
    notify_camera_mode(director);
    return at<Dword>(director, 316 + at<Dword>(director, 356) * 4);
}
// Callers discard the incidental EAX result.
void pop_matrix_mode(void *director) {
    if (invoke<void *>(director, 2) && at<Dword>(invoke<void *>(director, 2), 72) == 9)
        pop_mode(director);
}
} // namespace nfs::game::world::camera
namespace nfs::game::world::dependency::pending {

void initialize_camera_director(void *director) {
    if (network::session_active() && !find_camera_mode(director, 11))
        application::pending::create_network_camera(director);
    camera::create_high_camera(director);
    camera::create_bird_camera(director);
    camera::create_low_camera(director);
    camera::create_bumper_camera(director);
    camera::create_pause_camera(director);
    camera::create_matrix_camera(director);
    camera::create_jump_camera(director);
    camera::create_actor_camera(director);
    camera::read_master_configuration(director);
}

void update_camera_director(void *director) {
    if (void *provider = at<void *>(director, 300)) {
        invoke(provider, 0, offset(director, 288));
    } else {
        void *state = invoke<void *>(director, 2);
        const void *target = state ? offset(state, 16) : offset(subsystem(68), 784);
        for (unsigned axis = 0; axis < 3; ++axis)
            at<Dword>(director, 288 + axis * 4) = at<Dword>(target, axis * 4);
    }
    const Dword stop_mode = game::stop_requested;
    if (stop_mode) {
        if (!at<Byte>(director, 308)) {
            at<Byte>(director, 308) = 1;
            void *previous = find_camera_mode(
                director, at<Dword>(director, 312 + at<Dword>(director, 356) * 4));
            camera::push_mode(director, stop_mode == 3 ? 7 : 5);
            void *current = find_camera_mode(
                director, at<Dword>(director, 312 + at<Dword>(director, 356) * 4));
            // The saved copy is forward and includes all nineteen state words.
            auto *source = previous ? reinterpret_cast<const Dword *>(offset(previous, 4)) : nullptr;
            auto *destination = current ? reinterpret_cast<Dword *>(offset(current, 4)) : nullptr;
            for (unsigned word = 0; word < 19; ++word)
                destination[word] = source[word];
        }
    } else if (at<Byte>(director, 308)) {
        at<Byte>(director, 308) = 0;
        camera::pop_mode(director);
    }
    if (at<int>(director, 160) > 0 && --at<Dword>(director, 160) == 0) {
        at<Dword>(director, 160) = Dword(-1);
        camera::push_mode(director, 9);
    }
    camera::update_jump_camera(director);
    if (void *controller =
            find_camera_mode(director, at<Dword>(director, 312 + at<Dword>(director, 356) * 4)))
        invoke(controller, 2);
}

void unload_camera_director(void *director) {
    end_all_camera_modes(director);
    for (Dword identifier = 0; identifier < 12; ++identifier)
        if (void *controller = detach_camera_mode(director, identifier)) {
            camera::destroy_controller(controller);
            gui::application_release(controller);
        }
}
} // namespace nfs::game::world::dependency::pending
