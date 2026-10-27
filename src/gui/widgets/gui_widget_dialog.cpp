#include "gui_widget_family.hpp"
#include "gui_widget_shared_methods.hpp"
#include "nfs/core/application/application.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include "../lifecycle/gui_timer_x87.hpp"
namespace nfs::application::pending {
void set_widget_visible(void *, Dword);
}
namespace nfs::gui::widgets::family {
using game::runtime::at;
using game::runtime::invoke;
void move_to_first_draw_position(Widget *);

Dword named_child_id(Widget *widget, const char *name) {
    if (!name)
        return 0;
    const int count = int(widget->children.count);
    for (int index = 0; index < count; ++index)
        if (index >= 0 && index <= int(widget->children.count))
            if (auto *child =
                    static_cast<Widget *>(events::container_value(&widget->children, Dword(index))))
                if (string_data(&child->name) && !string_compare_ascii(&child->name, name))
                    return child->id;
    return 0;
}

Rectangle *to_screen_rectangle(Widget *widget, Rectangle *out, const Rectangle *source) {
    int top = source->top, left = source->left;
    const int height = source->height;
    do {
        left = int(Dword(left) + Dword(widget->bounds.left));
        top = int(Dword(top) + Dword(widget->bounds.top));
        const Dword flags = widget->flags;
        if (flags & 8)
            break;
        widget = widget->parent;
        if (!(flags & 64)) {
            if (!widget)
                break;
            left = int(Dword(left) + Dword(widget->scroll.x));
            top = int(Dword(top) + Dword(widget->scroll.y));
        }
    } while (widget);
    *out = Rectangle{engine::rectangle_methods, top, left, source->width, height};
    return out;
}

Rectangle *absolute_widget_bounds(Widget *widget, Rectangle *out) {
    Rectangle local{engine::rectangle_methods, 0, 0, widget->bounds.width, widget->bounds.height};
    return to_screen_rectangle(widget, out, &local);
}

void finish_modal(Widget *widget, Dword result, Dword owner) {
    events::dispatch(widget, 24, result, owner ? owner : widget->id);
}

Dword __fastcall dialog_pointer_press(Widget *widget, void *, Dword button, Dword modifiers) {
    if ((at<Dword>(widget, 272) & 8) && !std::uint8_t(modifiers)) {
        Rectangle bounds;
        absolute_widget_bounds(widget, &bounds);
        const Dword contains = contains_point(&bounds, pending::pointer_position());
        engine::destroy_rectangle(&bounds);
        if (!contains) {
            events::dispatch(widget, 41, 0, 0);
            return 1;
        }
    }
    return pending::update_focus_style(widget, nullptr, button, modifiers);
}

Dword __fastcall dialog_parent_leave(Widget *widget, void *, Dword) {
    return activate_first_child(widget);
}

Dword __fastcall show_dialog(Widget *widget, void *) {
    set_enabled(widget, 1);
    application::pending::set_widget_visible(widget, 1);
    move_to_first_draw_position(widget);
    if (!(at<Dword>(widget, 272) & 4))
        invoke(widget, 6, Dword(48));
    return reinterpret_cast<Dword>(activate(widget));
}

void __fastcall dialog_tick(Widget *widget, void *, Dword ticks) {
    timer_x87::dialog_tick(widget, ticks);
}

void __fastcall hide_dialog(Widget *widget, void *) {
    set_enabled(widget, 0);
    application::pending::set_widget_visible(widget, 0);
}

Dword __fastcall dialog_enter_modal(Widget *widget, void *) {
    at<Dword>(widget, 276) = 0;
    if (Widget *previous = activate(widget))
        at<Dword>(widget, 276) = previous->id;
    if (!pending::enter_modal(widget, nullptr))
        return 0;
    events::dispatch(widget, 79, 0, 0);
    at<float>(widget, 284) = 0;
    return 1;
}

void __fastcall dialog_leave_modal(Widget *widget, void *) {
    events::dispatch(widget, 80, 0, 0);
    pending::leave_modal(widget, nullptr);
    if (auto *previous =
            static_cast<Widget *>(events::find_widget(context_state, at<Dword>(widget, 276))))
        activate(previous);
    at<Dword>(widget, 276) = 0;
}

void __fastcall dialog_accept(Widget *widget, void *) {
    finish_modal(widget, 2, widget->id);
}

void __fastcall dialog_cancel(Widget *widget, void *) {
    finish_modal(widget, 3, widget->id);
}

void __fastcall dialog_configure_geometry(Widget *widget, void *, Dword sender_address, Dword event,
                                          Dword first, Dword) {
    if (event == 43) {
        const Dword sender = reinterpret_cast<Widget *>(sender_address)->id;
        if (sender == at<Dword>(widget, 264))
            events::post_widget(widget, 40, first, 0);
        else if (sender == at<Dword>(widget, 268))
            events::post_widget(widget, 41, first, 0);
    }
}

void __fastcall dialog_initialize(Widget *widget, void *) {
    at<Dword>(widget, 264) = named_child_id(widget, "<OK>");
    at<Dword>(widget, 268) = named_child_id(widget, "<CANCEL>");
    pending::request_general_layout(reinterpret_cast<GeneralWidget *>(widget), nullptr);
}

Dword __fastcall dialog_key_down(Widget *widget, void *, Dword key, Dword modifiers) {
    if (!std::uint8_t(modifiers)) {
        if (key == 1 && (at<Dword>(widget, 272) & 2)) {
            events::post_widget(widget, 41, 0, 0);
            return 1;
        }
        if (key == 28 && (at<Dword>(widget, 272) & 1)) {
            events::post_widget(widget, 40, 0, 0);
            return 1;
        }
    }
    return pending::general_key(reinterpret_cast<GeneralWidget *>(widget), nullptr, key, modifiers);
}

Dword __fastcall dialog_event(Widget *widget, void *, Dword event, Dword first, Dword second) {
    if (event == 79)
        invoke(widget, 47);
    else if (event == 80)
        invoke(widget, 48);
    return pending::general_event(reinterpret_cast<GeneralWidget *>(widget), nullptr, event, first,
                                  second);
}

Dword __fastcall dialog_button_down(Widget *widget, void *, Dword button, Dword modifiers) {
    if (!std::uint8_t(modifiers)) {
        if (button == 7 && (at<Dword>(widget, 272) & 1)) {
            events::post_widget(widget, 40, 0, 0);
            return 1;
        }
        if (button == 8 && (at<Dword>(widget, 272) & 2)) {
            events::post_widget(widget, 41, 0, 0);
            return 1;
        }
    }
    return pending::general_button(reinterpret_cast<GeneralWidget *>(widget), nullptr, button,
                                   modifiers);
}

void __fastcall msg_box_initialize(Widget *widget, void *) {
    at<Dword>(widget, 288) = named_child_id(widget, "<TITLE>");
    at<Dword>(widget, 292) = named_child_id(widget, "<TEXT>");
    at<Dword>(widget, 264) = named_child_id(widget, "<OK>");
    at<Dword>(widget, 268) = named_child_id(widget, "<CANCEL>");
    pending::request_general_layout(reinterpret_cast<GeneralWidget *>(widget), nullptr);
}
} // namespace nfs::gui::widgets::family
