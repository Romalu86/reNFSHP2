#include "nfs/input/window/window_input.hpp"
#include "nfs/graphics/display/raster_window.hpp"

namespace nfs::window_input {
namespace {
int constrained(int value, int minimum, int maximum) {
    int raised = value <= minimum ? minimum : value;
    if (raised >= maximum)
        return maximum;
    return value <= minimum ? minimum : value;
}
} // namespace
// No tool/test calls the cursor-moving API path.
void move_system_cursor(WindowState *state, int x, int y) {
    POINT point{x, y};
    if (ClientToScreen(state->handle, &point)) {
        SetCursorPos(point.x, point.y);
        SetCursor(nullptr);
        return;
    }
    SetCursorPos(x, y);
    SetCursor(nullptr);
}

void update_cursor(int x, int y, Dword buttons) {
    int bounded_x = constrained(x, cursor_min_x, cursor_max_x);
    int bounded_y = constrained(y, cursor_min_y, cursor_max_y);
    kernel::lock_enter(&message_lock);
    cursor_buttons = buttons;
    if (bounded_x != x || bounded_y != y)
        move_system_cursor(&window_state, bounded_x, bounded_y);
    if (bounded_x != cursor_x || bounded_y != cursor_y) {
        cursor_x = bounded_x;
        cursor_y = bounded_y;
    }
    kernel::lock_leave(&message_lock);
}

void bound_cursor(int min_x, int min_y, int max_x, int max_y) {
    cursor_min_x = min_x;
    cursor_min_y = min_y;
    cursor_max_x = max_x;
    cursor_max_y = max_y;
    update_cursor(cursor_x, cursor_y, cursor_buttons);
}

void position_cursor(int x, int y) {
    graphics::raster::State saved;
    if (kernel::is_thread(nullptr)) {
        graphics::raster::save(&saved);
        graphics::raster::disable();
    } else
        kernel::thread_gate(1);
    cursor_x = constrained(x, cursor_min_x, cursor_max_x);
    cursor_y = constrained(y, cursor_min_y, cursor_max_y);
    move_system_cursor(&window_state, cursor_x, cursor_y);
    if (!kernel::is_thread(nullptr)) {
        kernel::thread_gate(0);
        return;
    }
    graphics::raster::restore(&saved);
}
} // namespace nfs::window_input
