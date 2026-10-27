#include "nfs/input/controllers/controller_resources.hpp"
#include "nfs/core/application/application.hpp"
#include "nfs/input/directinput/directinput.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
namespace nfs::controllers {
using game::runtime::at;
void *primary{}, *secondary{}, *profiles{}, *bindings{}, *devices{};

void clear_devices(void *owner) {
    for (Dword index = 0; index < at<Dword>(owner, 4); ++index) {
        void *&device = at<void *>(owner, 8 + index * 4);
        if (device)
            gui::destroy(static_cast<gui::Object *>(device));
        device = nullptr;
    }
    at<Dword>(owner, 4) = 0;
    at<Byte>(owner, 0) = 0;
}

void clear_profiles(void *owner) {
    for (Dword index = 0; index < at<Dword>(owner, 192); ++index) {
        void *&profile = at<void *>(owner, index * 4);
        gui::application_release(profile);
        profile = nullptr;
    }
    at<Byte>(owner, 196) = 0;
    at<Dword>(owner, 192) = 0;
}
// EAX returns the existing or newly allocated controller list.
Dword *get_bindings() {
    if (!bindings) {
        auto *list = static_cast<Dword *>(
            lifecycle::allocate(40, "Controllers::ControllerList", lifecycle::ui_pool));
        if (list) {
            list[0] = 0;
            for (unsigned index = 0; index < 3; ++index) {
                list[index * 2 + 1] = Dword(-1);
                list[index * 2 + 2] = 0;
            }
            list[8] = list[9] = Dword(-1);
        }
        bindings = list;
    }
    return static_cast<Dword *>(bindings);
}
// The receiver is the manager's controller/feedback system.
void stop_device_effects(void *owner) {
    if (at<int>(owner, 4) >= 0)
        for (unsigned offset : {104u, 200u, 8u})
            if (const int effect = at<int>(owner, offset); effect >= 0)
                input::stop_effect(effect);
}
} // namespace nfs::controllers
namespace nfs::application::pending {

void release_controller_resources() {
    using namespace controllers;
    if (primary)
        gui::destroy(static_cast<gui::Object *>(primary));
    void *keys = secondary;
    primary = nullptr;
    if (keys)
        gui::destroy(static_cast<gui::Object *>(keys));
    void *definitions = devices;
    secondary = nullptr;
    if (definitions) {
        clear_devices(definitions);
        gui::application_release(definitions);
    }
    void *controller_list = bindings;
    devices = nullptr;
    if (controller_list)
        gui::application_release(controller_list);
    void *maps = profiles;
    bindings = nullptr;
    if (maps) {
        clear_profiles(maps);
        gui::application_release(maps);
    }
    profiles = nullptr;
}

void stop_force_feedback() {
    void *owner = game::runtime::system(144);
    if (game::runtime::at<Byte>(owner, 296))
        controllers::stop_device_effects(owner);
    else {
        controllers::pending::set_rumble(0, 0, 0);
        controllers::pending::set_rumble(1, 0, 0);
    }
}
} // namespace nfs::application::pending
