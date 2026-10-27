#include "nfs/graphics/configuration/display_configuration.hpp"
#include "nfs/graphics/backend/eagl_backend.hpp"
#include "nfs/graphics/textures/texture_conversion.hpp"
#include "nfs/input/window/window_input.hpp"
#include <cstring>
namespace nfs::graphics::startup {
// Display launch flags alias the shared Options object.
std::uint8_t cursor_clipped = 0;

Dword set_gamma(const WORD *input) {
    WORD ramp[3][256];
    for (unsigned i = 0; i != 256; ++i) {
        ramp[0][i] = input[i];
        ramp[1][i] = input[256 + i];
        ramp[2][i] = input[512 + i];
    }
    if (backend_device) {
        backend::com(backend_device, 72, Dword(0), ramp);
        backend::com(backend_device, 76, backend::saved_gamma);
    }
    return 1;
}
 
int centre_window(std::uint8_t frontend) {
    HWND desktop = GetDesktopWindow();
    HDC dc = GetWindowDC(desktop);
    int desktop_width = GetDeviceCaps(dc, 8), desktop_height = GetDeviceCaps(dc, 10);
    GetDeviceCaps(dc, 12);
    ReleaseDC(desktop, dc);
    HWND window = window_input::window_state.handle;
    DWORD class_style = GetClassLongA(window, GCL_STYLE);
    SetClassLongA(window, GCL_STYLE, class_style | 512);
    RECT rectangle;
    GetClientRect(window, &rectangle);
    rectangle.right = width(&settings, frontend);
    rectangle.bottom = height(&settings, frontend);
    LONG style = GetWindowLongA(window, GWL_STYLE);
    style &= 4294639615u;
    SetWindowLongA(window, GWL_STYLE, style);
    LONG extended_style = GetWindowLongA(window, GWL_EXSTYLE);
    AdjustWindowRectEx(&rectangle, style, FALSE, extended_style);
    int window_width = rectangle.right - rectangle.left;
    int window_height = rectangle.bottom - rectangle.top;
    int x = (desktop_width - window_width) / 2, y = (desktop_height - window_height) / 2;
    SetWindowPos(window, nullptr, x < 1 ? 0 : x, y < 1 ? 0 : y, window_width, window_height, 0);
    return 0;
}

int configure_display(std::uint8_t frontend, float *output_width, float *output_height,
                      std::uint8_t *fullscreen_output) {
    std::uint8_t low_colour = !textures::supports_format(&textures::database, 827611204);
    if (prefer_low_colour())
        low_colour = 1;
    if (!validate_modes(&settings, low_colour))
        return 4;
    DisplayMode &mode = frontend ? settings.frontend : settings.race;
    backend::set_dimensions(context, static_cast<float>(width(&settings, frontend)),
                            static_cast<float>(height(&settings, frontend)));
    backend::get_dimensions(context, output_width, output_height);
    backend::set_color_depth(context, mode.colour_depth);
    backend::set_depth_bits(context, mode.depth_bits);
    state::set_context_backbuffer_count(context, 1);
    state::set_context_stencil_bits(context, mode.stencil);
    state::set_texture_lod_bias_bits_override(3211159142u);
    if (refresh_context)
        state::mark_context_dirty(context);
    bool windowed = !fullscreen(&settings);
    if (windowed && fullscreen_override)
        windowed = false;
    if (windowed_override)
        windowed = true;
    *fullscreen_output = !windowed;
    backend::set_display_mode(context, windowed);
    SetWindowTextA(render_window, display_caption);
    if (!backend::initialize_context(context)) {
        GetLastError();
        return 2;
    }
    SetWindowTextA(render_window, "Need For Speed Hot Pursuit 2");
    if (windowed)
        centre_window(frontend);
    if (!fullscreen(&settings)) {
        cursor_clipped = 0;
        ClipCursor(nullptr);
    } else {
        RECT bounds{0, 0, width(&settings, frontend), height(&settings, frontend)};
        cursor_clipped = ClipCursor(&bounds) != 0;
    }
    HWND window = window_input::window_state.handle;
    SetForegroundWindow(window);
    BringWindowToTop(window);
    if (settings.gamma < 0) {
        if (!gamma_saved)
            settings.gamma = 1;
        else {
            WORD maximum = 0;
            for (unsigned i = 0; i != 256; ++i)
                if (gamma[0][i] > maximum)
                    maximum = gamma[0][i];
            settings.gamma = maximum * 0.00001525902189314365386962890625f;
        }
    }
    WORD ramp[3][256];
    for (unsigned i = 0; i != 256; ++i) {
        double sample = static_cast<double>(static_cast<WORD>(i << 8)) * settings.gamma;
        if (!(sample < 65535.0))
            sample = 65535.0;
        WORD entry = static_cast<WORD>(static_cast<std::int64_t>(sample));
        ramp[0][i] = ramp[1][i] = ramp[2][i] = entry;
    }
    set_gamma(&ramp[0][0]);
    mode.stencil = state::context_stencil_bits(context);
    if (settings_owner)
        write_render_settings();
    return 0;
}
} // namespace nfs::graphics::startup
