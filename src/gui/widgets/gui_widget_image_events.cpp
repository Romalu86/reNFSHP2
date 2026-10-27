#include "gui_widget_sequence.hpp"
#include "gui_widget_shared_methods.hpp"
#include "nfs/core/application/application.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include "../../math/precision/extended_float.hpp"
#include <cmath>
namespace nfs::gui::widgets::family {
using game::runtime::at;
using game::runtime::bytes;
using game::runtime::invoke;
using kernel::numeric::Extended;

Dword visible_in_ancestors(Widget *widget) {
    Dword visible = (~widget->flags >> 1) & 1;
    for (Widget *parent = widget->parent; parent && visible; parent = parent->parent)
        visible = (~parent->flags >> 1) & 1;
    return visible;
}

Dword inherited_palette(Widget *widget) {
    Dword palette = widget->palette;
    for (Widget *parent = widget->parent; !palette && parent; parent = parent->parent)
        palette = parent->palette;
    return palette;
}
// Equal z-order is intentional in this operation.
void move_to_first_draw_position(Widget *widget) {
    Widget *parent = widget->parent;
    if (!parent)
        return;
    auto *first = int(parent->children.count) < 0
                      ? nullptr
                      : static_cast<Widget *>(events::container_value(&parent->draw_order, 0));
    if (first && first != widget) {
        widget->z_order = first->z_order;
        container_remove(&parent->draw_order, widget);
        container_sorted_insert(&parent->draw_order, compare_order,
                                reinterpret_cast<void *>(widget->z_order), widget, nullptr);
    }
}
static float fixed_to_float(Dword value) {
    const auto format = kernel::numeric::current_format();
    return float(Extended::integer(int(value), format) * Extended(0.0000152587890625f, format));
}

Dword __fastcall static_image_event(Widget *widget, void *, Dword event, Dword first, Dword second) {
    if (event == 51) {
        at<float>(widget, 312) = fixed_to_float(first);
        return 1;
    }
    if (event == 52) {
        auto *resource = invoke<ImageResource *>(widget, 47);
        if (first)
            copy_image_resource(resource, reinterpret_cast<const ImageResource *>(first));
        else
            clear_image_resource(resource);
        return 1;
    }
    return pending::general_event(reinterpret_cast<GeneralWidget *>(widget), nullptr, event, first,
                                  second);
}

static void set_animation_rate(Widget *widget, float rate) {
    if (rate < 0)
        return;
    if (const Dword timer = at<Dword>(widget, 396))
        update_widget_timer(timer, rate != 0 ? 1.0f / rate : 0.0f);
    at<float>(widget, 324) = rate;
}

Dword __fastcall anim_image_event(Widget *widget, void *, Dword event, Dword first, Dword second) {
    switch (event) {
    case 50:
        if (first)
            copy_image_sequence(invoke<ImageSequence *>(widget, 48),
                                reinterpret_cast<const ImageSequence *>(first));
        return 1;
    case 51:
        at<float>(widget, 312) = fixed_to_float(first);
        return 1;
    case 52:
        if (first) {
            clear_image_sequence(invoke<ImageSequence *>(widget, 48));
            copy_image_resource(invoke<ImageResource *>(widget, 47),
                                reinterpret_cast<const ImageResource *>(first));
        }
        return 1;
    case 53:
        set_animation_rate(widget, fixed_to_float(first));
        return 1;
    case 54:
        return select_sequence_frame(reinterpret_cast<ImageSequence *>(bytes(widget) + 328),
                                     int(first));
    case 55:
        return at<Dword>(widget, 340);
    default:
        return pending::general_event(reinterpret_cast<GeneralWidget *>(widget), nullptr, event,
                                      first, second);
    }
}

void __fastcall anim_image_animate(Widget *widget, void *, Dword timer_id) {
    const float rate = at<float>(widget, 324);
    if (timer_id != at<Dword>(widget, 396) || !visible_in_ancestors(widget) ||
        (rate == 0 && !std::isnan(rate)) || (at<Dword>(widget, 384) & 1))
        return;
    const int count = int(invoke<ImageSequence *>(widget, 48)->frames);
    int frame = invoke<ImageSequence *>(widget, 48)->frame;
    // The saved virtual call is argumentless; its caller retains the frame argument.
    const Dword rate_frame = at<Dword>(widget, 336);
    const long double frame_rate =
        sequence_frame_rate(invoke<ImageSequence *>(widget, 48), rate_frame);
    float period_rate = float(frame_rate);
    if (frame_rate == 0 && !std::isnan(frame_rate))
        period_rate = at<float>(widget, 324);
    frame = int(Dword(frame) + ((at<Dword>(widget, 384) & 2) ? Dword(-1) : Dword(1)));
    if (frame < 0)
        frame = (at<Dword>(widget, 392) & 1) ? 0 : int(Dword(count) - 1);
    else if (frame >= count)
        frame = (at<Dword>(widget, 392) & 1) ? int(Dword(count) - 1) : 0;
    select_sequence_frame(reinterpret_cast<ImageSequence *>(bytes(widget) + 328), frame);
    if (const Dword timer = at<Dword>(widget, 396))
        update_widget_timer(timer, period_rate != 0 || std::isnan(period_rate) ? 1.0f / period_rate
                                                                               : 0.0f);
}

Dword __fastcall gripper_drawing_state(Widget *widget, void *, Dword render_address) {
    invoke(reinterpret_cast<engine::Engine *>(render_address), 50,
           Dword(application::pending::captured_widget_id == widget->id ? 16 : 15));
    return 1;
}

Dword __fastcall gripper_pointer_move(Widget *widget, void *, Dword first, Dword second) {
    if (application::pending::captured_widget_id == widget->id && widget->parent) {
        const auto *offset = reinterpret_cast<const Point *>(first);
        widget->parent->bounds.left = int(Dword(widget->parent->bounds.left) + Dword(offset->x));
        widget->parent->bounds.top = int(Dword(widget->parent->bounds.top) + Dword(offset->y));
        return 1;
    }
    return pending::update_active_style(widget, nullptr, first, second);
}

Dword __fastcall gripper_pointer_press(Widget *widget, void *, Dword button, Dword second) {
    if (!button) {
        if ((at<Dword>(widget, 400) & 1) && widget->parent)
            move_to_first_draw_position(widget->parent);
        capture_widget(widget);
        return 1;
    }
    return pending::update_focus_style(widget, nullptr, button, second);
}

Dword __fastcall gripper_pointer_release(Widget *widget, void *, Dword button, Dword second) {
    if (!button && application::pending::captured_widget_id == widget->id) {
        release_capture(widget);
        return 1;
    }
    return capture_style(widget, nullptr, button, second);
}
} // namespace nfs::gui::widgets::family
