#include "gui_widget_edit.hpp"
namespace nfs::gui::widgets::family {
using game::runtime::at;
using game::runtime::invoke;

void set_edit_anchor(Widget *widget, int position) {
    const int length = invoke<const String *>(widget, 8)->length;
    at<int>(widget, 284) = clamp_gui_integer(position, 0, length);
}

void set_edit_caret(Widget *widget, int position, Dword move_anchor) {
    if (active_widget_id == widget->id && position != at<int>(widget, 280))
        if (auto *render = active_engine())
            invoke(render, 60);
    const int length = invoke<const String *>(widget, 8)->length;
    const int clamped = clamp_gui_integer(position, 0, length);
    at<int>(widget, 280) = clamped;
    if (!(at<Dword>(widget, 324) & 2048) || move_anchor) {
        const int updated_length = invoke<const String *>(widget, 8)->length;
        at<int>(widget, 284) = clamp_gui_integer(clamped, 0, updated_length);
    }
}
} // namespace nfs::gui::widgets::family
