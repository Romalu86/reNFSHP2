#include "gui_widget_controls.hpp"
#include "../lifecycle/gui_input.hpp"
#include "nfs/core/application/application.hpp"
#include "nfs/core/timing/timer.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
namespace nfs::gui::engine {
int seconds_to_ticks(float);
}
namespace nfs::gui::input {

int pointer_idle_ticks() {
    return int(kernel::timer_count() - pointer_moved_at);
}

int input_idle_ticks() {
    return registry_time ? int(kernel::timer_count() - registry_time) : 0;
}
} // namespace nfs::gui::input
namespace nfs::gui::widgets::family {
using game::runtime::at;
using game::runtime::invoke;
Dword tooltips_enabled = 1;

void set_tooltip_owner(Widget *widget, Dword id) {
    widget->value_e0 = id;
}

Dword inherited_tooltip_owner(Widget *widget) {
    while (widget) {
        if (widget->value_e0)
            return widget->value_e0;
        widget = widget->parent;
    }
    return 0;
}

Dword pointer_in_visible_bounds(Widget *widget) {
    return widget->flags & 2 ? 0 : contains_point(&widget->padding, pending::pointer_position());
}

Widget *find_tooltip_target(Widget *root) {
    if (!root || (root->flags & 2) || !pointer_in_visible_bounds(root))
        return nullptr;
    const int count = int(root->children.count);
    for (int index = 0; index < count; ++index)
        if (Widget *child = child_at(root, index, 1))
            if (Widget *target = find_tooltip_target(child))
                return target;
    return (root->flags & 128) ||
                   ((root->tooltip.length || (root->flags & 16384)) && invoke<Dword>(root, 9))
               ? root
               : nullptr;
}

Widget *current_tooltip_target() {
    auto *captured = static_cast<Widget *>(
        events::find_widget(context_state, application::pending::captured_widget_id));
    return captured ? captured : find_tooltip_target(static_cast<Widget *>(focused_widget()));
}

void update_tooltip_position(Widget *widget) {
    auto *render = active_engine();
    if (!render || tooltip_owner != widget->id)
        return;
    Point position = *pending::pointer_position();
    position.methods = engine::point_methods;
    const Dword drawing_state = invoke<Dword>(render, 52);
    Point cursor;
    invoke<Point *>(render, 51, &cursor, drawing_state);
    position.y = int(Dword(position.y) + Dword(cursor.y));
    widget->bounds.left = position.x;
    widget->bounds.top = position.y;
    auto *target =
        static_cast<Widget *>(events::find_widget(context_state, at<Dword>(widget, 280)));
    if (target && target->tooltip.length)
        if (Widget *text = find_child(widget, at<Dword>(widget, 284)))
            invoke(text, 7, &target->tooltip);
    engine::destroy_point(&cursor);
    engine::destroy_point(&position);
}

void __fastcall tool_tip_initialize(Widget *widget, void *) {
    if (widget->parent)
        set_tooltip_owner(widget->parent, widget->id);
    application::pending::set_widget_visible(widget, 0);
    at<Dword>(widget, 256) &= ~Dword(1);
    at<Dword>(widget, 284) = named_child_id(widget, "<TEXT>");
    if (!at<Dword>(widget, 284))
        if (auto *render = active_engine()) {
            invoke(render, 21, inherited_palette(widget));
            auto *text = static_cast<Widget *>(create_text(widget));
            at<Dword>(text, 272) = 5;
            Dword colour;
            text->colour = ~*invoke<Dword *>(render, 23, &colour, Dword(4)) | Dword(4278190080u);
            at<Dword>(text, 264) = 96;
            text->layout_flags = 64;
            initialize_tree(text);
            at<Dword>(widget, 284) = text->id;
        }
    dispatch_target(at<Dword>(widget, 284), 31, 0, 0);
    pending::request_general_layout(reinterpret_cast<GeneralWidget *>(widget), nullptr);
}

Dword __fastcall tool_tip_event(Widget *widget, void *, Dword event, Dword first, Dword second) {
    if (event == 76)
        at<Dword>(widget, 264) = 0;
    else if (event == 75)
        at<Dword>(widget, 264) = 3;
    else if (event == 74 && tooltip_owner == widget->id && at<Dword>(widget, 280) == first)
        update_tooltip_position(widget);
    return pending::general_event(reinterpret_cast<GeneralWidget *>(widget), nullptr, event, first,
                                  second);
}

void __fastcall tool_tip_tick(Widget *widget, void *, Dword ticks) {
    Widget *target = tooltips_enabled ? current_tooltip_target() : nullptr;
    if (!target || inherited_tooltip_owner(target) != widget->id ||
        !string_data(&target->tooltip)) {
        at<Dword>(widget, 264) = 0;
        return;
    }
    const Dword target_id = target->id;
    tooltip_owner = widget->id;
    if (target_id != at<Dword>(widget, 280))
        at<Dword>(widget, 264) = 0;
    switch (at<Dword>(widget, 264)) {
    case 0:
        at<Dword>(widget, 280) = target_id;
        at<Dword>(widget, 264) = 1;
        at<Dword>(widget, 268) = 0;
        return;
    case 1:
        at<Dword>(widget, 268) = 0;
        if (input::pointer_idle_ticks() < engine::seconds_to_ticks(at<float>(widget, 272)))
            return;
        if (input::input_idle_ticks() < engine::seconds_to_ticks(at<float>(widget, 272)))
            return;
        at<Dword>(widget, 264) = 2;
        update_tooltip_position(widget);
        [[fallthrough]];
    case 2: {
        const Dword elapsed = at<Dword>(widget, 268) + ticks;
        at<Dword>(widget, 268) = elapsed;
        if (input::modifiers() & 29360128) {
            at<Dword>(widget, 264) = 3;
            return;
        }
        const float duration = at<float>(widget, 276);
        if (duration != 0 && int(elapsed) > engine::seconds_to_ticks(duration))
            at<Dword>(widget, 264) = 4;
        return;
    }
    default:
        return;
    }
}
} // namespace nfs::gui::widgets::family
