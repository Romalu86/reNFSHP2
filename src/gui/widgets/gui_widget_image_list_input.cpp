#include "gui_widget_image_list.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include "../../math/precision/extended_float.hpp"
#include <cmath>
namespace nfs::gui::widgets::family {
using game::runtime::invoke;

void __fastcall image_list_pointer_idle(Widget *widget, void *, Dword first, Dword modifiers) {
    if (modifiers & 4194304) {
        Point local;
        from_screen_point(widget, &local, pending::pointer_position());
        int index = 0;
        const int width = image_list(widget)->cell_size.x;
        if (width) {
            Rectangle client;
            client_bounds(widget, &client);
            const int x = clamp_gui_integer(local.x, client.left, client.left + client.width);
            const int first_cell = first_image_cell(widget);
            index = (x - (width * first_cell + widget->scroll.x)) / width + first_cell;
            engine::destroy_rectangle(&client);
        }
        engine::destroy_point(&local);
        if (select_image_cell(widget, index, 1))
            events::post_widget(widget, 68, 0, 0);
    }
    style_result_38(widget, nullptr, first, modifiers);
}

Dword __fastcall image_list_key_down(Widget *widget, void *, Dword key, Dword modifiers) {
    auto *list = image_list(widget);
    int index;
    switch (key) {
    case 199:
        index = 0;
        if (!(list->style & 8)) {
            const int count = int(list->items.count);
            int checked = 0;
            for (; checked < count; ++checked, --index)
                if (image_item_enabled(image_cell_id(widget, index)))
                    break;
            if (checked >= count)
                index = 1;
        }
        break;
    case 203:
        index = previous_enabled_image_cell(widget, list->selected);
        break;
    case 205:
        index = next_enabled_image_cell(widget, list->selected);
        break;
    case 201:
        index = previous_enabled_image_cell(widget, list->selected - visible_image_cells(widget));
        break;
    case 207:
        index = next_enabled_image_cell(widget, int(list->items.count) - 2);
        break;
    case 209:
        index = next_enabled_image_cell(widget, list->selected + visible_image_cells(widget));
        break;
    default:
        return pending::general_key(&list->base, nullptr, key, modifiers);
    }
    if (select_image_cell(widget, index, 1))
        events::post_widget(widget, 68, 0, 0);
    return 1;
}

void __fastcall image_list_animate(Widget *widget, void *, Dword timer) {
    auto *list = image_list(widget);
    if (timer != list->timer || !list->cell_size.x)
        return;
    Point delta;
    image_scroll_delta(widget, &delta, list->selected);
    const int distance = delta.x;
    engine::destroy_point(&delta);
    if (!distance)
        return;
    using kernel::numeric::Extended;
    const auto format = kernel::numeric::current_format();
    const float step_value =
        float(Extended::integer(list->cell_size.x, format) * Extended(list->speed, format) *
              Extended(0.0333333350718021392822265625f, format));
    const int step = int(std::lrintf(step_value));
    auto signed_magnitude = [](int value) {
        const Dword sign = value < 0 ? Dword(-1) : 0;
        return int((Dword(value) ^ sign) - sign);
    };
    const int amount = signed_magnitude(step) < signed_magnitude(distance)
                           ? (distance < 1 ? int(0u - Dword(step)) : step)
                           : distance;
    const int next = int(Dword(widget->scroll.x) + Dword(amount));
    if (next == widget->scroll.x)
        return;
    widget->scroll.x = next;
    refresh_image_frames(widget);
    image_scroll_delta(widget, &delta, list->selected);
    if (!delta.x)
        events::post_widget(widget, 56, 0, 0);
    engine::destroy_point(&delta);
}
// Tail jump to .
void __fastcall image_list_initialize(Widget *widget, void *) {
    pending::request_general_layout(&image_list(widget)->base, nullptr);
}
} // namespace nfs::gui::widgets::family
