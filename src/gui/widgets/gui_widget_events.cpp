#include "gui_widgets.hpp"
#include "nfs/core/application/application.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include "nfs/core/memory/memory_primitives.hpp"
namespace nfs::gui::widgets {
using game::runtime::invoke;

void clear_active(Widget *widget) {
    const Dword previous = active_widget_id;
    if (previous && (previous == widget->id || descendant(widget, previous))) {
        active_widget_id = 0;
        dispatch_target(previous, 15, 0, 0);
    }
}

Widget *activate(Widget *widget) {
    auto *previous =
        active_widget_id
            ? static_cast<Widget *>(events::find_widget(context_state, active_widget_id))
            : nullptr;
    if (previous != widget) {
        const Dword previous_id = previous ? previous->id : 0;
        if (may_activate(widget)) {
            const Dword id = widget->id;
            active_widget_id = id;
            dispatch_target(id, 14, previous_id, 0);
            dispatch_target(previous_id, 15, id, 0);
        }
    }
    return previous;
}

static Dword push_modal(ContextState *context, Widget *widget) {
    return events::push_front(&context->extra->list, widget, nullptr);
}

static void remove_modal(ContextState *context, Widget *widget) {
    events::remove_value(&context->extra->list, widget);
}

Dword bind_string_reference(String *text, Dword reference) {
    if (reference && resolve_string(reference)) {
        if (text->data) {
            kernel::memory_free(text->data);
            text->data = nullptr;
        }
        text->length = 0;
        text->wide = 0;
        text->reference = reference;
        text->length = resolve_length(reference);
        text->wide = resolve_wide(reference);
        return 1;
    }
    return 0;
}
namespace pending {

Dword __fastcall enter_modal(Widget *widget, void *) {
    if (widget->options & 1)
        return 0;
    for (Widget *ancestor = widget; ancestor; ancestor = ancestor->parent)
        if (!(ancestor->options & 2))
            return 0;
    widget->modal_result = 0;
    widget->options |= 1;
    if (context_state)
        for (Dword event : {11u, 10u, 12u, 9u, 7u, 6u, 8u, 3u, 4u, 2u, 5u})
            events::remove_event(context_state, event);
    Dword &captured = application::pending::captured_widget_id;
    if (captured)
        if (auto *owner = static_cast<Widget *>(events::find_widget(context_state, captured)))
            release_capture(owner);
    captured = 0;
    if (active_widget_id)
        if (auto *owner =
                static_cast<Widget *>(events::find_widget(context_state, active_widget_id)))
            clear_active(owner);
    activate(widget);
    return push_modal(context_state, widget) == 0;
}

void __fastcall leave_modal(Widget *widget, void *) {
    widget->options &= ~Dword(1);
    remove_modal(context_state, widget);
}

Dword __fastcall navigate(Widget *widget, void *, Dword event, Dword first, Dword second) {
    switch (event) {
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
        if (widget->flags & 1)
            return 1;
        break;
    case 9:
    case 10:
    case 11:
    case 12:
        if (widget->flags & 3)
            return 1;
        break;
    case 21:
        return 0; // Entry bypasses both filtering and forwarding.
    case 13:
    case 14:
    case 15:
    case 16:
    case 17:
    case 19:
    case 23:
    case 24:
        break;
    case 18:
        if (widget->options & 2)
            return 1;
        break;
    case 20:
    case 22:
        if (!(widget->options & 2))
            return 1;
        break;
    case 25:
        if ((widget->flags & 1) == Dword(!first))
            return 1;
        break;
    case 26:
        if (((widget->flags >> 1) & 1) == Dword(!first))
            return 1;
        break;
    default:
        break;
    }
    if (widget->value_d8 && !(widget->flags & 262144))
        if (Widget *child = find_child(widget, widget->value_d8)) {
            struct ForwardedEvent {
                Widget *sender;
                Dword event, first, second;
            };
            ForwardedEvent forwarded{widget, event, first, second};
            events::dispatch(child, 36, reinterpret_cast<Dword>(&forwarded), 0);
        }
    return 0;
}

Dword __fastcall handle_event(Widget *widget, void *, Dword event, Dword first, Dword second) {
    Dword result = 0;
    const auto bubble = [&] {
        if (widget->parent)
            events::dispatch(widget->parent, event, first, second);
    };
    const auto forward_pointer = [&] {
        const Dword id = next_pointer_target(widget);
        if (id)
            if (void *target = events::find_widget(context_state, id))
                events::dispatch(target, event, first, second);
    };
    switch (event) {
    case 1:
        if (first) {
            result = invoke<Dword>(widget, 16, first);
            if (!result && !(widget->flags & 16512) && widget->parent)
                result = events::dispatch(widget->parent, event, first, second);
        }
        break;
    case 3:
        result = invoke<Dword>(widget, 36, first, second);
        if (!result)
            bubble();
        break;
    case 4:
        result = invoke<Dword>(widget, 37, first, second & 65535);
        if (!result)
            bubble();
        break;
    case 5:
        result = invoke<Dword>(widget, 35, first, second);
        if (!result)
            bubble();
        break;
    case 6:
        result = invoke<Dword>(widget, 32, first, second);
        if (!result)
            bubble();
        break;
    case 7:
        result = invoke<Dword>(widget, 33, first, second);
        break;
    case 8:
        result = invoke<Dword>(widget, 34, first, second);
        if (!result)
            bubble();
        break;
    case 9:
        result = invoke<Dword>(widget, 38, first, second);
        if (!result)
            forward_pointer();
        break;
    case 10: {
        Point point;
        engine::construct_point(&point);
        point.x = int(std::int16_t(first));
        point.y = int(std::int16_t(first >> 16));
        result = invoke<Dword>(widget, 39, &point, second);
        engine::destroy_point(&point);
        if (!result)
            forward_pointer();
        break;
    }
    case 11:
        result = invoke<Dword>(widget, 40, first, second);
        if (!result)
            forward_pointer();
        break;
    case 12:
        result = invoke<Dword>(widget, 41, first, second);
        if (!result)
            forward_pointer();
        break;
    case 13:
    case 14:
    case 15:
    case 16:
    case 17: {
        void *source = first ? events::find_widget(context_state, first) : nullptr;
        const unsigned slot = event == 13    ? 26
                              : event == 14  ? 28
                              : event == 15  ? 27
                              : event == 16 ? 29
                                              : 30;
        invoke(widget, slot, source);
        break;
    }
    case 18:
        if (!(widget->options & 2))
            invoke(widget, 21);
        break;
    case 19:
        invoke(widget, 22);
        break;
    case 20:
        if (widget->options & 2)
            invoke(widget, 25);
        break;
    case 21:
        invoke(widget, 24, first);
        break;
    case 22:
        invoke(widget, 23, first);
        break;
    case 23:
        if (Widget *child = find_child(widget, first)) {
            if (contains_modal(child))
                events::post(context_state, widget->id, event, first, second, 0);
            else {
                container_remove(&widget->draw_order, child);
                container_remove(&widget->tab_order, child);
                container_remove(&widget->children, child);
            }
        }
        break;
    case 24: {
        void *source = second ? events::find_widget(context_state, second) : nullptr;
        if (invoke<Dword>(widget, 31, first, source)) {
            const int count = int(widget->children.count);
            for (int index = 0; index < count; ++index)
                if (Widget *child = child_at(widget, index, 0))
                    events::dispatch(child, event, 13, second);
        }
        break;
    }
    case 25:
        result = ~widget->flags & 1;
        if (!first && result) {
            release_capture(widget);
            clear_active(widget);
        }
        widget->flags = (widget->flags & ~Dword(first != 0)) | Dword(first == 0);
        break;
    case 26:
        result = (~widget->flags >> 1) & 1;
        if (!first && result)
            release_capture(widget);
        widget->flags = (widget->flags & ~(Dword(first != 0) * 2)) | (Dword(first == 0) * 2);
        break;
    case 27:
        widget->colour = first;
        result = 1;
        break;
    case 28:
        widget->colour = ((widget->colour ^ first) & 16777215) ^ widget->colour;
        result = 1;
        break;
    case 29:
        result = widget->colour;
        break;
    case 30:
        result = widget->flags;
        widget->flags = (result & ~first) | second;
        break;
    case 31: {
        if (!first && !second)
            invoke(widget, 7, &lifecycle::construction::unresolved::shared_string);
        else {
            String text;
            string_construct(&text);
            try {
                if (first) {
                    string_assign(&text, reinterpret_cast<const char *>(first));
                    invoke(widget, 7, &text);
                } else if (bind_string_reference(&text, second))
                    invoke(widget, 7, &text);
            } catch (...) {
                string_destruct(&text);
                throw;
            }
            string_destruct(&text);
        }
        break;
    }
    case 32:
        widget->scroll.x = int(first);
        widget->scroll.y = int(second);
        break;
    case 33:
        invoke(widget, 6, first);
        break;
    case 34:
        activate(widget);
        return active_widget_id == widget->id;
    case 35:
        if (first)
            if (void *source = events::find_widget(context_state, first))
                invoke(widget, 15, source, second);
        break;
    }
    return result;
}
} // namespace pending
} // namespace nfs::gui::widgets
