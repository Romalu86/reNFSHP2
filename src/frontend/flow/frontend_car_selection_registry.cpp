#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "../screens/frontend_menu_roots.hpp"
#include <cstdlib>
namespace nfs::application::frontend {
using namespace gui::widgets;
Dword __cdecl car_next_pressed(void *, Dword, Dword, Dword);
Dword __cdecl toggle_car_description(void *, Dword, Dword, Dword);
static CallbackRecord car_callbacks[2];

static void unregister_car_callbacks() {
    for (unsigned i = 2; i; --i)
        unregister_callback(car_callbacks + i - 1);
}

static void register_car_callbacks() {
    register_callback(car_callbacks, family::button_class_key, "BEVENT_PRESSED", 43,
                      "CarSelectScreen::OnNextPress", car_next_pressed, 0);
    register_callback(car_callbacks + 1, family::button_class_key, "BEVENT_PRESSED", 43,
                      "CarSelectScreen::ToggleShowcase", toggle_car_description, 0);
    std::atexit(unregister_car_callbacks);
}
namespace {
NFS_KERNEL_INITIALIZER(startup_register_car_callbacks, ".CRT$XCU_NFS_00690050",
                       register_car_callbacks);
} // namespace
} // namespace nfs::application::frontend
