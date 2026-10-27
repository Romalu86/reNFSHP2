#include "gui_widget_controls.hpp"
#include "nfs/core/application/application.hpp"
#include "nfs/gui/resources/gui_format.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
namespace nfs::gui::widgets::family {
using game::runtime::invoke;

void __fastcall button_set_text(Widget *widget, void *, Dword text_address) {
    const auto *source = reinterpret_cast<const String *>(text_address);
    const auto *previous = invoke<const String *>(widget, 8);
    if (string_equal(source, previous))
        return;
    string_copy(&widget->text, source);
    if (Widget *text = find_child(widget, reinterpret_cast<Button *>(widget)->text))
        invoke(text, 7, source);
}

Dword __fastcall button_visual_state(Widget *widget, void *) {
    auto *button = reinterpret_cast<Button *>(widget);
    Dword state = 0;
    if (((button->style & 6) && button->checked) ||
        (application::pending::captured_widget_id == widget->id && invoke<Dword>(widget, 9) &&
         !(widget->flags & 1)))
        state = 1;
    if (widget->parent && (button->style & 32)) {
        if (widget->parent->flags & 1)
            state |= 4;
        if (active_widget_id == widget->parent->id)
            state |= 2;
    }
    if (active_widget_id == widget->id)
        state |= 2;
    if (widget->flags & 1)
        state |= 4;
    return state;
}

void __fastcall set_button_checked(Widget *widget, void *, Dword checked) {
    auto *button = reinterpret_cast<Button *>(widget);
    if (!(button->style & 6) || button->checked == checked)
        return;
    button->checked = checked;
    Widget *parent = widget->parent;
    if ((button->style & 4) && checked && parent)
        for (int index = 0; index < int(parent->children.count); ++index)
            if (Widget *child = child_at(parent, index, 0))
                if (invoke<Dword>(child, 2, button_class_key)) {
                    auto *other = reinterpret_cast<Button *>(child);
                    if ((other->style & 4) && child != widget &&
                        other->radio_group == button->radio_group)
                        events::dispatch(child, 45, 0, 0);
                }
    events::dispatch(widget, 44, checked, 0);
}

void __fastcall activate_button(Widget *widget, void *) {
    auto *button = reinterpret_cast<Button *>(widget);
    const Dword style = button->style;
    if ((style & 4) && !(style & 2))
        invoke(widget, 47, Dword(1));
    else if (style & 6)
        invoke(widget, 47, Dword(button->checked == 0));
    events::dispatch(widget, 40, 0, 0);
}

void __fastcall button_set_checked_event(Widget *widget, void *, Dword checked) {
    invoke(widget, 47, checked);
}

Dword __fastcall button_button_down(Widget *widget, void *, Dword button, Dword modifiers) {
    if (button == 7 &&
        (!std::uint8_t(modifiers) || (reinterpret_cast<Button *>(widget)->style & 8)))
        return invoke<Dword>(widget, 32, Dword(28), modifiers);
    return pending::general_button(reinterpret_cast<GeneralWidget *>(widget), nullptr, button,
                                   modifiers);
}

void __fastcall button_parent_enter(Widget *widget, void *, Dword parent) {
    release_capture(widget);
    notify_parent_enter(widget, nullptr, parent);
}

Dword __fastcall button_pointer_press(Widget *widget, void *, Dword button, Dword modifiers) {
    if (button)
        return pending::update_focus_style(widget, nullptr, button, modifiers);
    auto *control = reinterpret_cast<Button *>(widget);
    if ((control->style & 8) && invoke<Dword>(widget, 9))
        capture_widget(widget);
    else if (!(control->style & 64)) {
        if (application::pending::captured_widget_id != widget->id && !std::uint8_t(modifiers)) {
            activate(widget);
            capture_widget(widget);
            return 1;
        }
        return pending::update_focus_style(widget, nullptr, button, modifiers);
    } else if (std::uint8_t(modifiers))
        return 1;
    events::dispatch(widget, 43, 0, modifiers);
    return 1;
}

void update_button_appearance(Widget *widget) {
    auto *button = reinterpret_cast<Button *>(widget);
    const Dword state = invoke<Dword>(widget, 48);
    if (state == button->last_state)
        return;
    events::dispatch(widget, 48, state, button->last_state);
    auto *render = active_engine();
    if (!render)
        return;
    Widget *disabled_down = find_child(widget, button->disabled_down);
    Widget *disabled = find_child(widget, button->disabled);
    Widget *down = find_child(widget, button->down);
    Widget *focus = find_child(widget, button->focus);
    Widget *up = find_child(widget, button->up);
    Widget *focus_down = find_child(widget, button->focus_down);
    const Dword pressed = state & 1;
    int selected = 3;
    if (pressed && (state & 4) && disabled_down)
        selected = -2;
    else if (pressed && (state & 2) && focus_down)
        selected = -1;
    else if (pressed && down)
        selected = 0;
    else if ((state & 2) && focus)
        selected = 1;
    else if ((state & 4) && disabled)
        selected = 2;
    if (disabled_down)
        application::pending::set_widget_visible(disabled_down, selected == -2);
    if (focus_down)
        application::pending::set_widget_visible(focus_down, selected == -1);
    if (down)
        application::pending::set_widget_visible(down, selected == 0);
    if (focus)
        application::pending::set_widget_visible(focus, selected == 1);
    if (disabled)
        application::pending::set_widget_visible(disabled, selected == 2);
    if (up)
        application::pending::set_widget_visible(up, selected == 3);
    if (Widget *text = find_child(widget, button->text)) {
        invoke(render, 21, inherited_palette(text));
        Dword colour;
        const Dword index = pressed ? 3 : (state & 2) ? 1 : (state & 4) ? 2 : 0;
        text->colour = *invoke<Dword *>(render, 23, &colour, index);
    }
    if (Widget *icon = find_child(widget, button->icon)) {
        invoke(render, 21, inherited_palette(icon));
        Dword colour;
        const Dword index = pressed ? 3 : (state & 2) ? 1 : (state & 4) ? 2 : 0;
        icon->colour = *invoke<Dword *>(render, 23, &colour, index);
    }
    button->last_state = state;
}

Dword __fastcall button_event(Widget *widget, void *, Dword event, Dword first, Dword second) {
    switch (event) {
    case 21:
        update_button_appearance(widget);
        break;
    case 43:
        invoke(widget, 49);
        return 1;
    case 44:
        invoke(widget, 50, first);
        return 1;
    case 45:
        invoke(widget, 51, first);
        return 1;
    case 46:
        return reinterpret_cast<Button *>(widget)->checked;
    case 47:
        return invoke<Dword>(widget, 48);
    }
    return pending::general_event(reinterpret_cast<GeneralWidget *>(widget), nullptr, event, first,
                                  second);
}
} // namespace nfs::gui::widgets::family
