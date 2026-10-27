#include "gui_widgets.hpp"
#include "nfs/core/application/application.hpp"
namespace nfs::gui::widgets {

Dword dispatch_target(Dword id, Dword event, Dword first, Dword second) {
    if (id)
        if (void *widget = events::find_widget(context_state, id))
            return events::dispatch(widget, event, first, second);
    return 0;
}

Dword enabled_in_ancestors(Widget *widget) {
    Dword enabled = ~widget->flags & 1;
    for (Widget *parent = widget->parent; parent && enabled; parent = parent->parent)
        enabled = ~parent->flags & 1;
    return enabled;
}

Dword has_ancestor(Widget *widget, const void *ancestor) {
    if (ancestor)
        for (Widget *parent = widget->parent; parent; parent = parent->parent)
            if (parent == ancestor)
                return 1;
    return 0;
}
namespace {
bool in_modal_root(Widget *widget) {
    if (!context_state)
        return true;
    void *root = events::focused(context_state);
    return !root || root == widget || has_ancestor(widget, root);
}
Widget *active_widget() {
    return active_widget_id
               ? static_cast<Widget *>(events::find_widget(context_state, active_widget_id))
               : nullptr;
}
} // namespace

Dword may_activate(Widget *widget) {
    if (!enabled_in_ancestors(widget) || (widget->flags & 4096))
        return 0;
    return in_modal_root(widget);
}

Dword activate_first_child(Widget *widget) {
    const Dword flags = widget->flags;
    if ((flags & 131076) == 131076 && active_widget_id != widget->id && !(flags & 1)) {
        if (enabled_in_ancestors(widget) && !(flags & 4096) && in_modal_root(widget)) {
            Widget *previous = active_widget();
            if (previous != widget) {
                const Dword old_id = previous ? previous->id : 0;
                if (may_activate(widget)) {
                    const Dword id = widget->id;
                    active_widget_id = id;
                    dispatch_target(id, 14, old_id, 0);
                    dispatch_target(old_id, 15, id, 0);
                }
            }
            if (active_widget_id == widget->id)
                return widget->id;
        }
    }
    const int count = int(widget->children.count);
    for (int index = 0; index < count; ++index)
        if (auto *child =
                static_cast<Widget *>(events::container_value(&widget->tab_order, Dword(index))))
            if (!(child->flags & 1) && (child->flags & 4) && enabled_in_ancestors(child) &&
                !(child->flags & 4096) && in_modal_root(child)) {
                if (Dword activated = activate_first_child(child))
                    return activated;
                Widget *previous = active_widget();
                if (previous != child) {
                    const Dword old_id = previous ? previous->id : 0;
                    if (may_activate(child)) {
                        const Dword id = child->id;
                        active_widget_id = id;
                        dispatch_target(id, 14, old_id, 0);
                        dispatch_target(old_id, 15, id, 0);
                    }
                }
                if (active_widget_id == child->id)
                    return child->id;
            }
    return 0;
}
namespace pending {

Dword __fastcall update_active_style(Widget *widget, void *, Dword, Dword) {
    const Dword flags = widget->flags;
    if ((flags & 256) && active_widget_id != widget->id && enabled_in_ancestors(widget) &&
        !(flags & 4096) && in_modal_root(widget)) {
        Widget *previous = active_widget();
        if (previous != widget) {
            const Dword old_id = previous ? previous->id : 0;
            if (enabled_in_ancestors(widget) && !(widget->flags & 4096) &&
                in_modal_root(widget)) {
                const Dword id = widget->id;
                active_widget_id = id;
                dispatch_target(id, 14, old_id, 0);
                dispatch_target(old_id, 15, id, 0);
            }
        }
        return 1;
    }
    return application::pending::captured_widget_id == widget->id ? 1 : flags & 82048;
}

Dword __fastcall update_focus_style(Widget *widget, void *, Dword, Dword suppressed) {
    if (!std::uint8_t(suppressed) && enabled_in_ancestors(widget) && !(widget->flags & 4096) &&
        in_modal_root(widget)) {
        if (active_widget_id != widget->id &&
            (!active_widget_id || !descendant(widget, active_widget_id))) {
            if (!activate_first_child(widget)) {
                Widget *previous = active_widget();
                if (previous != widget) {
                    const Dword old_id = previous ? previous->id : 0;
                    if (enabled_in_ancestors(widget) && !(widget->flags & 4096) &&
                        in_modal_root(widget)) {
                        const Dword id = widget->id;
                        active_widget_id = id;
                        dispatch_target(id, 14, old_id, 0);
                        dispatch_target(old_id, 15, id, 0);
                    }
                }
            }
        }
        return 1;
    }
    return application::pending::captured_widget_id == widget->id ? 1 : widget->flags & 82048;
}
} // namespace pending

void capture_widget(Widget *widget) {
    Dword &captured = application::pending::captured_widget_id;
    const Dword previous = captured;
    if (!(widget->flags & 3) && captured != widget->id) {
        captured = widget->id;
        if (previous)
            dispatch_target(previous, 13, widget->id, 0);
    }
}

void release_capture(Widget *widget) {
    Dword &captured = application::pending::captured_widget_id;
    const Dword previous = captured;
    if (previous && (previous == widget->id || descendant(widget, previous))) {
        captured = 0;
        dispatch_target(previous, 13, 0, 0);
    }
}

void set_enabled(Widget *widget, Dword enabled) {
    if (!enabled && !(widget->flags & 1)) {
        release_capture(widget);
        const Dword previous = active_widget_id;
        if (previous && (previous == widget->id || descendant(widget, previous))) {
            active_widget_id = 0;
            dispatch_target(previous, 15, 0, 0);
        }
    }
    widget->flags = (widget->flags & ~Dword(enabled != 0)) | Dword(enabled == 0);
}
} // namespace nfs::gui::widgets
