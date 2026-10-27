#include "gui_widget_settings.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
namespace nfs::gui::widgets {
using game::runtime::invoke;
void *create_tool_tip(void *);
void *create_scroll_bar(void *);
void __fastcall request_layout(Widget *, void *);

Dword initialize_tree(Widget *widget) {
    events::propagate(widget, 18, 0, 0, 0);
    return 0;
}
namespace pending {

void __fastcall request_general_layout(GeneralWidget *widget, void *) {
    if (widget->style & 1) {
        auto *tooltip = static_cast<Widget *>(create_tool_tip(widget));
        tooltip->layout_flags |= 64;
        tooltip->flags |= 8;
        initialize_tree(tooltip);
    }
    if (widget->style & 4) {
        void *scroll = create_scroll_bar(widget);
        game::runtime::at<Dword>(scroll, 264) = 1;
    }
    if (widget->style & 8) {
        void *scroll = create_scroll_bar(widget);
        game::runtime::at<Dword>(scroll, 264) = 3;
    }
    request_layout(&widget->base, nullptr);
}

Dword __fastcall general_key(GeneralWidget *widget, void *, Dword key, Dword modifiers) {
    if (!(modifiers & 3932160)) {
        switch (key) {
        case 15:
            if (widget->state & 1) {
                if (modifiers & 196608)
                    previous_tab(&widget->base);
                else
                    next_tab(&widget->base);
                return 1;
            }
            break;
        case 200:
            if (widget->state & 8) {
                previous_tab(&widget->base);
                return 1;
            }
            break;
        case 203:
            if (widget->state & 2) {
                previous_tab(&widget->base);
                return 1;
            }
            break;
        case 205:
            if (widget->state & 4) {
                next_tab(&widget->base);
                return 1;
            }
            break;
        case 208:
            if (widget->state & 16) {
                next_tab(&widget->base);
                return 1;
            }
            break;
        }
    }
    return widget->base.flags & 16512;
}

Dword __fastcall general_button(GeneralWidget *widget, void *, Dword button, Dword) {
    if (button == 2) {
        previous_tab(&widget->base);
        return 1;
    }
    if (button == 1) {
        next_tab(&widget->base);
        return 1;
    }
    return widget->base.flags & 16512;
}

const char *__cdecl scroll_bar_class_name() {
    return "GScrollBar";
}

Dword __fastcall general_event(GeneralWidget *widget, void *, Dword event, Dword first, Dword second) {
    switch (event) {
    case 37:
        if (Widget *child = find_child(&widget->base, first))
            if (invoke<Dword>(child, 2, scroll_bar_class_name))
                invoke(widget, 44, child);
        break;
    case 38:
    case 39:
        if (invoke<Dword>(widget, event == 38 ? 46 : 45, first, second))
            return 1;
        if (!(widget->base.flags & 16512) && widget->base.parent)
            return events::dispatch(widget->base.parent, event, first, second);
        return 0;
    case 40:
        invoke(widget, 42);
        break;
    case 41:
        invoke(widget, 43);
        break;
    }
    return handle_event(&widget->base, nullptr, event, first, second);
}
} // namespace pending
} // namespace nfs::gui::widgets
