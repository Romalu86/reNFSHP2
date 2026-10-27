#include "gui_widget_edit.hpp"
namespace nfs::gui::widgets::family {
using game::runtime::invoke;
namespace {
int edit_font_height(Widget *widget) {
    if (auto *render = active_engine()) {
        const Dword previous = invoke<Dword>(render, 14, inherited_font_style(widget));
        const int height = invoke<int>(render, 17);
        invoke(render, 14, previous);
        return height;
    }
    return 0;
}
} // namespace

Dword __fastcall edit_text_key_down(Widget *widget, void *, Dword key, Dword modifiers) {
    auto *state = edit(widget);
    const bool shift = (modifiers & 196608) != 0;
    auto notify = [&](int position) {
        events::post_widget(widget, 61, Dword(position), 0);
        return Dword(1);
    };
    switch (key) {
    case 1:
        events::post_widget(widget, 41, 0, 0);
        break;
    case 14:
        backspace_edit_character(widget);
        return notify(state->caret);
    case 28:
        if ((state->edit_style & 128) && (state->text_style & 1)) {
            if (!(modifiers & 3932160) && insert_edit_character(widget, 10, modifiers & 4194304))
                notify(state->caret);
            return 1;
        }
        if (!(modifiers & 255))
            events::post_widget(widget, 40, 0, 0);
        break;
    case 30:
        if (!(modifiers & 255) && (modifiers & 786432) && (state->edit_style & 8)) {
            const int length = edit_value(widget)->length;
            if (!length)
                return 1;
            set_edit_anchor(widget, 0);
            set_edit_caret(widget, length, 0);
            return notify(-1);
        }
        break;
    case 45:
    case 46:
        if (!(modifiers & 255) && (modifiers & 786432) && (state->edit_style & 8) &&
            (key != 45 || !(state->edit_style & 4)) &&
            edit_caret(widget) != edit_anchor(widget)) {
            String selected;
            selected_edit_text(widget, &selected);
            Dword copied;
            try {
                copied = copy_clipboard(&selected);
            } catch (...) {
                string_destruct(&selected);
                throw;
            }
            string_destruct(&selected);
            if (key == 46)
                return 1;
            if (copied) {
                erase_edit_selection(widget);
                return 1;
            }
        }
        break;
    case 47:
        if ((modifiers & 786432) && paste_edit_text(widget))
            return 1;
        break;
    case 199:
    case 207:
        if (modifiers & 786432) {
            move_edit_selection(widget, key == 199 ? 0 : edit_value(widget)->length, !shift);
        } else {
            Point point;
            edit_position_to_screen(widget, &point, edit_caret(widget));
            point.x += key == 199 ? -10000 : 10000;
            move_edit_selection(widget, edit_screen_to_position(widget, &point), !shift);
            engine::destroy_point(&point);
        }
        return notify(-1);
    case 203:
    case 205:
        if (!(modifiers & 3145728)) {
            set_edit_caret(widget, state->caret + (key == 203 ? -1 : 1), !shift);
            return notify(-1);
        }
        break;
    case 200:
    case 208:
        if (!(modifiers & 3145728) && (state->text_style & 1)) {
            Point point;
            edit_position_to_screen(widget, &point, state->caret);
            const int height = edit_font_height(widget);
            point.y += key == 200 ? -height : height;
            const int position = edit_screen_to_position(widget, &point);
            engine::destroy_point(&point);
            if (position < 0 || (key == 200 ? state->caret <= position : position <= state->caret))
                return 1;
            move_edit_selection(widget, position, !shift);
            return notify(-1);
        }
        break;
    case 201:
    case 209:
        if (!(modifiers & 3145728)) {
            const int height = edit_font_height(widget);
            if ((state->text_style & 1) && height) {
                Point point;
                edit_position_to_screen(widget, &point, state->caret);
                const int page = (widget->bounds.height / height) * height;
                point.y += key == 201 ? -page : page;
                const int position = edit_screen_to_position(widget, &point);
                engine::destroy_point(&point);
                if (position < 0 ||
                    (key == 201 ? state->caret <= position : position <= state->caret))
                    return 1;
                move_edit_selection(widget, position, !shift);
                return notify(-1);
            }
        }
        break;
    case 211:
        if (!(state->edit_style & 4)) {
            if ((state->edit_style & 8) && edit_caret(widget) != edit_anchor(widget)) {
                erase_edit_selection(widget);
            } else {
                EditString changed;
                string_copy(&changed, edit_value(widget));
                erase_string_character(&changed, edit_caret(widget));
                invoke(widget, 7, &changed);
            }
        }
        return notify(state->caret);
    }
    return pending::general_key(&state->base, nullptr, key, modifiers);
}
} // namespace nfs::gui::widgets::family
