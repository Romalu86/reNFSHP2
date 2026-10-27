#include "nfs/core/application/application.hpp"
#include "nfs/input/directinput/directinput.hpp"
#include "nfs/graphics/configuration/display_configuration.hpp"
#include "nfs/graphics/math/eagl_math.hpp"
#include "nfs/graphics/display/viewport_transform.hpp"
namespace nfs::application::pending {

int switch_display(Dword frontend) {
    float width, height;
    Byte fullscreen;
    const int result =
        graphics::startup::configure_display(Byte(frontend), &width, &height, &fullscreen);
    input::devices_shutdown();
    input::devices_initialize();
    input::keyboard_shutdown();
    input::keyboard_initialize();
    if (result)
        return result;
    for (auto *viewport :
         {graphics::main_viewport, graphics::background_viewport, graphics::ui_viewport})
        graphics::viewports::rectangle(viewport, 0, 0, width, height, 0, 1);
    float matrix[16], translation[16];
    graphics::math::diagonal(matrix, 2.0f / width, -2.0f / height, 1, 1);
    graphics::math::translation(translation, -1, 1, 0);
    graphics::math::append(matrix, translation);
    graphics::viewports::set_matrix(graphics::background_viewport, matrix);
    if (!fullscreen) {
        graphics::startup::centre_window(Byte(frontend));
        if (graphics::gamma_saved) {
            HWND desktop = GetDesktopWindow();
            HDC dc = GetWindowDC(desktop);
            SetDeviceGammaRamp(dc, graphics::gamma);
            ReleaseDC(desktop, dc);
        }
    }
    return 0;
}
} // namespace nfs::application::pending
