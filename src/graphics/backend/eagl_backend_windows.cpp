#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "nfs/input/window/window_input.hpp"
#include "nfs/graphics/backend/eagl_backend.hpp"
namespace nfs::graphics::backend {

LRESULT CALLBACK render_window_proc(HWND window, UINT event, WPARAM wparam, LPARAM lparam) {
    LPARAM result = lparam;
    if (window_bound && window_input::dispatch(window, event, wparam, lparam, &result))
        return result;
    return DefWindowProcA(window, event, wparam, lparam);
}
WNDCLASSEXA window_definition = {
    48,      64,     render_window_proc, 0, 0, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr}; // Window class fields include aliased instance and name storage. Resource 32512 identifies the
// application's icon group.
static void initialize_window_definition() {
    window_definition.hInstance = GetModuleHandleA(nullptr);
    window_definition.hIcon = LoadIconA(GetModuleHandleA(nullptr), MAKEINTRESOURCEA(32512));
    window_definition.hCursor = LoadCursorA(nullptr, MAKEINTRESOURCEA(32512));
    window_definition.hbrBackground = nullptr;
    window_definition.lpszMenuName = nullptr;
    window_definition.lpszClassName = "EAGL PC";
    window_definition.hIconSm = nullptr;
}
// CRT entry  tail-calls the definition initializer.
static void initialize_window_class() {
    initialize_window_definition();
}

HWND caps_window(WNDCLASSEXA definition) {
    RECT bounds;
    SetRect(&bounds, 0, 0, 100, 100);
    HWND window =
        CreateWindowExA(0, "EAGL PC Caps Retrieval", "EAGL PC Caps Retrieval", 0, CW_USEDEFAULT,
                        CW_USEDEFAULT, bounds.right - bounds.left, bounds.bottom - bounds.top,
                        GetDesktopWindow(), nullptr, definition.hInstance, nullptr);
    if (window)
        UpdateWindow(window);
    return window;
}
// Authorized compatibility change: capability discovery must
// not create a temporary fullscreen device before the configured display mode.
std::uint8_t retrieve_adapter_caps(std::uint8_t alternate) {
    char error[256];
    Dword *caps = query_adapter_capabilities(backend_interface, 0, error);
    if (alternate)
        alternate_caps = caps;
    else
        hardware_caps = caps;
    return caps != nullptr;
}
static bool register_class(Dword *object) {
    Dword *c = owner(object);
    auto *bytes = reinterpret_cast<std::uint8_t *>(c);
    if (!bytes[184]) {
        bytes[184] = 1;
        if (window_created) {
            UnregisterClassA(window_class, window_instance);
            window_created = 0;
        }
        window_class = reinterpret_cast<const char *>(owner(object)[180 / 4]);
        if (!RegisterClassExA(&window_definition)) {
            UnregisterClassA(window_class, window_instance);
            if (!RegisterClassExA(&window_definition))
                return false;
        }
        window_created = 1;
    }
    return true;
}
static void change_window(Dword *object) {
    Dword *c = owner(object);
    Dword previous = c[156 / 4];
    if (previous != c[152 / 4]) {
        if (render_window) {
            if (previous != 3 && previous != 2)
                DestroyWindow(render_window);
            render_window = nullptr;
        }
        owner(object)[156 / 4] = owner(object)[152 / 4];
    }
}

std::uint8_t fullscreen_window(Dword *object) {
    if (!register_class(object))
        return 0;
    change_window(object);
    if (!render_window) {
        int height = GetSystemMetrics(1), width = GetSystemMetrics(0);
        render_window = CreateWindowExA(8, window_class, window_class, 2429091840u, 0, 0, width,
                                        height, nullptr, nullptr, window_instance, nullptr);
        if (!render_window)
            return 0;
        SetFocus(render_window);
        ShowWindow(render_window, 5);
        UpdateWindow(render_window);
        window_input::bind_window(render_window, render_window, 1);
        window_bound = 1;
    } else {
        int height = GetSystemMetrics(1), width = GetSystemMetrics(0);
        MoveWindow(render_window, 0, 0, width, height, 1);
    }
    presentation = {};
    if (fullscreen_formats(object, 0) && populate_presentation(object)) {
        int result = backend_device ? reset_device(object) : create_hal_device(object, 0);
        if (result >= 0 && backend_device)
            return 1;
    }
    return 0;
}

std::uint8_t windowed_window(Dword *object) {
    if (!register_class(object))
        return 0;
    change_window(object);
    Dword *c = owner(object);
    RECT bounds;
    if (!render_window) {
        SetRect(&bounds, 0, 0, c[8], c[9]);
        AdjustWindowRect(&bounds, 2429485056u, 0);
        render_window =
            CreateWindowExA(0, window_class, window_class, 2429485056u, CW_USEDEFAULT, CW_USEDEFAULT,
                            bounds.right - bounds.left, bounds.bottom - bounds.top,
                            GetDesktopWindow(), nullptr, window_instance, nullptr);
        if (!render_window)
            return 0;
        UpdateWindow(render_window);
        window_input::bind_window(render_window, render_window, 0);
        window_bound = 1;
    } else {
        SetRect(&bounds, 0, 0, c[8], c[9]);
        AdjustWindowRect(&bounds, 2429485056u, 0);
        SetWindowPos(render_window, nullptr, 0, 0, owner(object)[8], owner(object)[9], 64);
        UpdateWindow(render_window);
    }
    presentation = {};
    if (windowed_formats(object, 0) && populate_presentation(object)) {
        int result = backend_device ? reset_device(object) : create_hal_device(object, 0);
        if (result >= 0 && backend_device)
            return 1;
    }
    return 0;
}
} // namespace nfs::graphics::backend

NFS_KERNEL_INITIALIZER(startup_initialize_window_class, ".CRT$XCU_NFS_006903A8",
                       nfs::graphics::backend::initialize_window_class);
