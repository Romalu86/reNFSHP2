#include "gui_widgets.hpp"
namespace nfs::gui::widgets {

static Widget *next_sibling(Widget *widget, Dword wrap, Dword order) {
    Widget *parent = widget->parent;
    if (!parent)
        return nullptr;
    const int count = int(parent->children.count);
    for (int index = 0; index < count; ++index)
        if (child_at(parent, index, order) == widget) {
            ++index;
            if (wrap)
                index %= count;
            return child_at(parent, index, order);
        }
    return nullptr;
}

static Widget *previous_sibling(Widget *widget, Dword wrap, Dword order) {
    Widget *parent = widget->parent;
    if (!parent)
        return nullptr;
    const int count = int(parent->children.count);
    if (!count)
        return nullptr;
    for (int index = count - 1; index >= 0; --index)
        if (child_at(parent, index, order) == widget) {
            --index;
            if (wrap && index < 0)
                index = count - 1;
            return child_at(parent, index, order);
        }
    return nullptr;
}
namespace {
bool tab_candidate(Widget *widget) {
    if ((widget->flags & 1) || !(widget->flags & 4) || !enabled_in_ancestors(widget) ||
        (widget->flags & 4096))
        return false;
    if (!context_state)
        return true;
    void *root = events::focused(context_state);
    return !root || root == widget || has_ancestor(widget, root);
}
} // namespace

Dword activate_last_child(Widget *widget) {
    if ((widget->flags & 131076) == 131076 && active_widget_id != widget->id &&
        tab_candidate(widget)) {
        activate(widget);
        if (active_widget_id == widget->id)
            return widget->id;
    }
    const int count = int(widget->children.count);
    for (int index = 0; index < count; ++index)
        if (auto *child = static_cast<Widget *>(
                events::container_value(&widget->tab_order, Dword(count - index - 1))))
            if (tab_candidate(child)) {
                if (Dword id = activate_last_child(child))
                    return id;
                activate(child);
                if (active_widget_id == child->id)
                    return child->id;
            }
    return 0;
}
namespace pending {

void next_tab(Widget *widget) {
restart_at_widget:
    if (!(widget->flags & 128) && widget->parent) {
        if (activate_first_child(widget))
            return;
        for (Widget *candidate = next_sibling(widget, 0, 2); candidate;
             candidate = next_sibling(candidate, 0, 2))
            if (tab_candidate(candidate)) {
                if (activate_first_child(candidate))
                    return;
                activate(candidate);
                if (active_widget_id == candidate->id)
                    return;
            }
        Widget *ancestor = widget->parent;
        while (ancestor) {
            if ((ancestor->flags & 128) || !ancestor->parent)
                break;
            Widget *parent = ancestor->parent;
            for (Widget *candidate = next_sibling(ancestor, 0, 2); candidate;
                 candidate = next_sibling(candidate, 0, 2))
                if (tab_candidate(candidate)) {
                    if (activate_first_child(candidate))
                        return;
                    activate(candidate);
                    if (active_widget_id == candidate->id)
                        return;
                    widget = candidate;
                    goto restart_at_widget;
                }
            ancestor = parent;
        }
        if (ancestor)
            widget = ancestor;
    }
    activate_first_child(widget);
}

void previous_tab(Widget *widget) {
restart_at_widget:
    if (!(widget->flags & 128) && widget->parent) {
        if (activate_last_child(widget))
            return;
        for (Widget *candidate = previous_sibling(widget, 0, 2); candidate;
             candidate = previous_sibling(candidate, 0, 2))
            if (tab_candidate(candidate)) {
                if (activate_last_child(candidate))
                    return;
                activate(candidate);
                if (active_widget_id == candidate->id)
                    return;
            }
        Widget *ancestor = widget->parent;
        while (ancestor) {
            if ((ancestor->flags & 128) || !ancestor->parent)
                break;
            Widget *parent = ancestor->parent;
            for (Widget *candidate = previous_sibling(ancestor, 0, 2); candidate;
                 candidate = previous_sibling(candidate, 0, 2))
                if (tab_candidate(candidate)) {
                    if (activate_last_child(candidate))
                        return;
                    activate(candidate);
                    if (active_widget_id == candidate->id)
                        return;
                    widget = candidate;
                    goto restart_at_widget;
                }
            ancestor = parent;
        }
        if (ancestor)
            widget = ancestor;
    }
    activate_last_child(widget);
}
} // namespace pending
} // namespace nfs::gui::widgets
