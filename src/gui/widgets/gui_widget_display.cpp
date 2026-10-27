#include "gui_widget_settings.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
namespace nfs::gui::engine {
const Rectangle *current_viewport_bounds(const Base *);
}
namespace nfs::gui::widgets {
using game::runtime::invoke;
void register_deferred_widget(ContextState *, Widget *);

Dword inherited_font_style(Widget *widget) {
    Dword style = widget->font_style;
    for (Widget *parent = widget->parent; !style && parent; parent = parent->parent)
        style = parent->font_style;
    return style;
}
namespace pending {
// The allocator's parent argument is ignored by the constructor.
void *construct_interface(void *storage, void *) {
    auto *widget = static_cast<Widget *>(construct_base(storage, nullptr));
    widget->flags = (widget->flags & ~Dword(3)) | 128;
    widget->methods = interface_methods;
    return widget;
}

Dword __fastcall enter_interface(Widget *widget, void *) {
    if (!enter_modal(widget, nullptr))
        return 0;
    activate_first_child(widget);
    return 1;
}

Dword __fastcall load_interface(Widget *widget, void *, void *configuration) {
    return load_configuration(widget, nullptr, configuration);
}

Dword __fastcall save_interface(Widget *widget, void *, void *configuration) {
    return save_configuration(widget, nullptr, configuration);
}

void __fastcall request_interface_layout(Widget *widget, void *) {
    widget->options |= 2;
    if (widget->flags & 2097152)
        register_deferred_widget(context_state, widget);
    widget->timer = 0;
    events::post(context_state, widget->id, 19, 0, 0, 0);
}
// RET8 includes the ignored modifier argument.
Dword __fastcall interface_key(Widget *widget, void *, Dword key, Dword) {
    switch (key) {
    case 15:
    case 205:
    case 208:
        if (!active_widget_id || !events::find_widget(context_state, active_widget_id)) {
            activate_first_child(widget);
            return 1;
        }
        break;
    case 200:
    case 203:
        if (!active_widget_id || !events::find_widget(context_state, active_widget_id)) {
            activate_last_child(widget);
            return 1;
        }
    }
    return widget->flags & 16512;
}
// RET8 includes the ignored modifier argument.
Dword __fastcall interface_button(Widget *widget, void *, Dword button, Dword) {
    if ((button == 1 || button == 2) &&
        (!active_widget_id || !events::find_widget(context_state, active_widget_id))) {
        if (button == 2)
            activate_last_child(widget);
        else
            activate_first_child(widget);
        return 1;
    }
    return widget->flags & 16512;
}

void *construct_simple_text(void *storage, void *parent) {
    auto *widget = static_cast<Widget *>(construct_base(storage, parent));
    widget->methods = simple_text_methods;
    widget->flags = (widget->flags & ~Dword(6)) | 1;
    return widget;
}

void __fastcall layout_simple_text(Widget *widget, void *, Dword flags) {
    if (auto *engine = active_engine())
        if (flags & 64) {
            Rectangle bounds;
            engine::construct_rectangle(&bounds);
            Dword previous = invoke<Dword>(engine, 14, inherited_font_style(widget));
            const String *text = invoke<const String *>(widget, 8);
            if (invoke<Dword>(engine, 15, text, &bounds)) {
                widget->bounds.width = bounds.width;
                widget->bounds.height = invoke<int>(engine, 17);
            }
            invoke(engine, 14, previous);
            engine::destroy_rectangle(&bounds);
        }
    layout(widget, nullptr, flags);
}

void __fastcall draw_simple_text(Widget *widget, void *, engine::Engine *engine) {
    if (!engine::pending::character_at(invoke<const String *>(widget, 8), 0))
        return;
    const Rectangle *bounds = engine::current_viewport_bounds(&engine->base);
    invoke(engine, 14, inherited_font_style(widget));
    invoke(engine, 42, invoke<const String *>(widget, 8), bounds->left, bounds->top,
           &widget->colour, Dword(0));
}
} // namespace pending
} // namespace nfs::gui::widgets
