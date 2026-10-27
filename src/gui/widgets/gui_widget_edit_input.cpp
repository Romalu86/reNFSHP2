#include "gui_widget_edit.hpp"
#include "nfs/core/application/application.hpp"
#include <cstring>
namespace nfs::gui::widgets::family {
using game::runtime::invoke;

Dword take_widget_event(Widget *widget, Dword *event, Dword *first, Dword *second,
                       int minimum, int maximum, Dword remove) {
    return events::take_target(context_state, widget->id, event, first, second, minimum, maximum,
                               remove);
}

Dword overwrite_edit_character(Widget *widget, Dword character, Dword modifiers) {
    auto *state = edit(widget);
    if ((state->edit_style & 4) || !character ||
        (state->character_limit && state->character_limit <= edit_value(widget)->length))
        return 0;
    if (events::dispatch(widget, 62, character, Dword(edit_caret(widget))))
        return 0;
    backspace_edit_character(widget);
    return insert_edit_character(widget, character, modifiers);
}

Dword __fastcall edit_text_pointer_move(Widget *widget, void *, Dword first, Dword modifiers) {
    if (application::pending::captured_widget_id != widget->id)
        return pending::update_active_style(widget, nullptr, first, modifiers);
    const int position = edit_screen_to_position(widget, pending::pointer_position());
    if (position >= 0 && position != edit(widget)->caret) {
        const Dword style = edit(widget)->edit_style;
        move_edit_selection(widget, position, !(style & 8));
        events::post_widget(widget, 61, Dword(-1), 0);
    }
    return 1;
}

Dword __fastcall edit_text_pointer_press(Widget *widget, void *, Dword button, Dword modifiers) {
    if (button || active_widget_id != widget->id)
        return pending::update_focus_style(widget, nullptr, button, modifiers);
    const int position = edit_screen_to_position(widget, pending::pointer_position());
    if (application::pending::captured_widget_id == widget->id || position < 0)
        return 1;
    capture_widget(widget);
    if ((edit(widget)->edit_style & 8) && (modifiers & 33554432)) {
        const int caret = edit(widget)->caret, anchor = edit(widget)->anchor;
        if (position == caret) {
            select_edit_word(widget, position);
            return 1;
        }
        const int start = caret < anchor ? caret : anchor, end = caret < anchor ? anchor : caret;
        if (position <= end && start <= position && start != end &&
            end - start != edit_value(widget)->length) {
            const int length = edit_value(widget)->length;
            if (!length)
                return 1;
            const int current_length = edit_value(widget)->length;
            edit(widget)->anchor = current_length < 0 ? current_length : 0;
            move_edit_selection(widget, length, false);
            events::post_widget(widget, 61, Dword(-1), 0);
            return 1;
        }
    }
    move_edit_selection(widget, position, !(modifiers & 196608));
    if (!(modifiers & 255))
        events::post_widget(widget, 61, Dword(-1), 0);
    return 1;
}

void __fastcall begin_edit_session(Widget *widget, void *) {
    if (auto *render = active_engine())
        invoke(render, 60);
    string_copy(&edit(widget)->saved_text, edit_value(widget));
    if (!(edit(widget)->text_style & 1)) {
        if (edit(widget)->edit_style & 8) {
            const int length = edit_value(widget)->length;
            edit(widget)->anchor = length < 0 ? length : 0;
            move_edit_selection(widget, edit_value(widget)->length, false);
        } else {
            move_edit_selection(widget, edit_value(widget)->length, true);
        }
    } else {
        Point point{engine::point_methods, 0, 0};
        move_edit_selection(widget, edit_screen_to_position(widget, &point), true);
        engine::destroy_point(&point);
    }
}

Dword __fastcall edit_text_character(Widget *widget, void *, Dword character, Dword modifiers) {
    if (!(modifiers & 3932160) && insert_edit_character(widget, character, modifiers & 4194304)) {
        events::post_widget(widget, 61, Dword(edit(widget)->caret), 0);
        return 1;
    }
    return style_result_35(widget, nullptr, character, modifiers);
}

Dword __fastcall edit_text_button_down(Widget *widget, void *, Dword button, Dword modifiers) {
    auto *state = edit(widget);
    if (button == 7 || button == 8) {
        if (!(modifiers & 255))
            events::post_widget(widget, button == 7 ? 40 : 41, 0, 0);
    } else if (button == 4) {
        backspace_edit_character(widget);
        events::post_widget(widget, 61, Dword(state->caret), 0);
        return 1;
    } else if (button == 3 || button == 5 || button == 6) {
        int character = 0;
        if (state->alphabet) {
            const int length = int(std::strlen(state->alphabet));
            if (length) {
                if (button == 3)
                    state->alphabet_position =
                        clamp_gui_integer(state->alphabet_position, 0, length - 1);
                else if (button == 5)
                    state->alphabet_position = (state->alphabet_position + 1) % length;
                else if (--state->alphabet_position < 0)
                    state->alphabet_position = length - 1;
                character = static_cast<signed char>(state->alphabet[state->alphabet_position]);
            }
        }
        if (button != 3) {
            if ((state->edit_style & 4) || !character ||
                (state->character_limit && state->character_limit <= edit_value(widget)->length) ||
                events::dispatch(widget, 62, Dword(character), Dword(edit_caret(widget))))
                return pending::general_button(&state->base, nullptr, button, modifiers);
            backspace_edit_character(widget);
        }
        if (insert_edit_character(widget, Dword(character), modifiers & 4194304)) {
            events::post_widget(widget, 61, Dword(state->caret), 0);
            return 1;
        }
    }
    return pending::general_button(&state->base, nullptr, button, modifiers);
}
} // namespace nfs::gui::widgets::family
