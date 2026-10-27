#include "camera_controller_runtime.hpp"
#include "nfs/core/lifecycle/object_state.hpp"
#include "nfs/gui/engine/gui_engine.hpp"
#include "nfs/core/errors/assertions.hpp"
namespace nfs::game::world::camera {
Dword selected_modes[2]{};

void *construct_controller(void *controller) {
    lifecycle::construction::objects::environment_view(offset(controller, 4));
    at<const Dword *>(controller, 0) = controller_methods;
    for (unsigned field = 80; field <= 100; field += 4)
        at<Dword>(controller, field) = 0;
    at<Dword>(controller, 4) = 0;
    return controller;
}

void clear_controller_owner(void *controller) {
    at<Dword>(controller, 0) = 0;
}

void destroy_controller(void *controller) {
    at<const Dword *>(controller, 0) = controller_methods;
    try {
        for (unsigned field = 80; field <= 100; field += 4)
            if (void *behavior = at<void *>(controller, field))
                invoke<void *>(behavior, 0, 1u);
    } catch (...) {
        clear_controller_owner(offset(controller, 4));
        throw;
    }
    clear_controller_owner(offset(controller, 4));
}
// Complete RET leaf.
void __fastcall begin_controller(void *, void *) {
}
// Complete RET leaf.
void __fastcall end_controller(void *, void *) {
}

void __fastcall advance_controller(void *controller, void *) {
    for (unsigned field = 80; field <= 100; field += 4)
        if (void *behavior = at<void *>(controller, field))
            invoke(behavior, 1, offset(controller, 4));
}
// Complete RET4 leaf.
void __fastcall end_position_behavior(void *, void *, void *) {
}

void *__fastcall delete_position_base(void *behavior, void *, Dword flags) {
    at<const Dword *>(behavior, 0) = position_base_methods;
    if (flags & 1)
        gui::application_release(behavior);
    return behavior;
}

void destroy_network_position(void *behavior) {
    at<const Dword *>(behavior, 0) = network_position_methods;
    at<const Dword *>(behavior, 0) = position_base_methods;
}

void *__fastcall delete_network_position(void *behavior, void *, Dword flags) {
    destroy_network_position(behavior);
    if (flags & 1)
        gui::application_release(behavior);
    return behavior;
}

void __fastcall notify_network_position(void *behavior, void *, void *state) {
    invoke(behavior, 1, state);
}

void *construct_network_position(void *behavior) {
    at<const Dword *>(behavior, 0) = position_base_methods;
    at<const Dword *>(behavior, 0) = network_position_methods;
    at<Dword>(behavior, 8) = 973279855;
    at<Dword>(behavior, 4) = 0;
    at<Dword>(behavior, 20) = 0;
    at<Dword>(behavior, 12) = 0;
    at<float>(behavior, 16) = 1.f;
    return behavior;
}

const Dword controller_methods[] = {reinterpret_cast<Dword>(begin_controller),
                                   reinterpret_cast<Dword>(end_controller),
                                   reinterpret_cast<Dword>(advance_controller)};
const Dword position_base_methods[] = {reinterpret_cast<Dword>(delete_position_base),
                                      reinterpret_cast<Dword>(assertions::pure_virtual_call),
                                      reinterpret_cast<Dword>(end_position_behavior)};
const Dword network_position_methods[] = {
    reinterpret_cast<Dword>(delete_network_position),
    reinterpret_cast<Dword>(advance_network_position),
    reinterpret_cast<Dword>(notify_network_position)};
} // namespace nfs::game::world::camera
namespace nfs::application::pending {

void create_network_camera(void *director) {
    using namespace game::world;
    void *memory = lifecycle::allocate(108, "camera controller", lifecycle::level_pool);
    void *controller = memory ? camera::construct_controller(memory) : nullptr;
    at<Dword>(controller, 76) = 11;
    at<Dword>(controller, 104) = 11;
    at<void *>(controller, 4) = director;
    memory = lifecycle::allocate(24, "behavior - position Setup", lifecycle::level_pool);
    void *behavior = memory ? camera::construct_network_position(memory) : nullptr;
    at<Dword>(behavior, 12) = 1083179008;
    at<Dword>(behavior, 20) = 1083179008;
    at<float>(behavior, 16) = 1.f;
    at<void *>(controller, 80) = behavior;
    append_camera_mode(director, controller);
}
} // namespace nfs::application::pending
