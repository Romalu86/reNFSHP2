#include "nfs/gui/events/gui_events.hpp"
#include "nfs/network/session/network_runtime.hpp"
namespace nfs::gui::events {
using network::field;

void *focused(ContextState *context) {
    return peek_front(&context->extra->list);
}

Dword contains_focus(ContextState *context, void *widget) {
    return contains(&context->extra->list, widget);
}

void *focus_ancestor(void *widget) {
    while (widget) {
        if (contains_focus(context_state, widget))
            return widget;
        widget = field<void *>(widget, 56);
    }
    return widget;
}

Dword is_within_focus(void *widget) {
    if (context_state)
        if (void *current = focused(context_state))
            while (widget) {
                if (widget == current)
                    return 1;
                widget = field<void *>(widget, 56);
            }
    return 0;
}

void request_focus(void *widget, Dword value) {
    if (context_state && value)
        notify_context_focus(context_state, widget, value);
}

void notify_context_focus(ContextState *context, void *widget, Dword value) {
    if (!widget || !is_within_focus(widget))
        return;
    void *ancestor = focus_ancestor(widget);
    if (!ancestor)
        return;
    bool notify = ancestor != widget;
    const Dword count = context->fourth.count;
    for (Dword i = 0; i < count; ++i) {
        void *item = container_value(&context->fourth, i);
        if (item && item != widget && focus_ancestor(item) == ancestor) {
            post_widget(item, 35, field<Dword>(widget, 4), value);
            if (item == ancestor)
                notify = false;
        }
    }
    if (notify)
        post_widget(ancestor, 35, field<Dword>(widget, 4), value);
}
} // namespace nfs::gui::events
