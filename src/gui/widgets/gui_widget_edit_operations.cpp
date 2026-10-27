#include "gui_widget_edit.hpp"
#include "nfs/gui/resources/gui_format.hpp"
#include <ctype.h>
#include <wctype.h>
namespace nfs::gui::widgets::family {
using engine::pending::character_at;
using game::runtime::invoke;

String *selected_edit_text(Widget *widget, String *out) {
    EditString selected;
    if (edit(widget)->edit_style & 8) {
        const int anchor = edit_anchor(widget), caret = edit_caret(widget);
        const int start = anchor < caret ? anchor : caret;
        const int selected_caret = edit_caret(widget), selected_anchor = edit_anchor(widget);
        const Dword difference = Dword(selected_caret) - Dword(selected_anchor);
        const Dword sign = int(difference) < 0 ? Dword(-1) : 0;
        const int length = int((difference ^ sign) - sign);
        string_substring(edit_value(widget), start, length, &selected);
    }
    string_construct_copy(out, &selected);
    return out;
}

void erase_edit_selection(Widget *widget) {
    if (!(edit(widget)->edit_style & 8) || edit_caret(widget) == edit_anchor(widget))
        return;
    int start = edit_anchor(widget), end = edit_caret(widget);
    if (end < start)
        start = end;
    const int anchor = edit_anchor(widget), caret = edit_caret(widget);
    end = anchor < caret ? caret : anchor;
    const String *source = edit_value(widget);
    EditString prefix, suffix;
    string_substring(source, 0, start, &prefix);
    string_substring(source, end, source->length, &suffix);
    String joined;
    concatenate(&joined, &prefix, &suffix);
    try {
        invoke(widget, 7, &joined);
        move_edit_selection(widget, start, true);
    } catch (...) {
        string_destruct(&joined);
        throw;
    }
    string_destruct(&joined);
}

Dword insert_edit_character(Widget *widget, Dword character, Dword wide) {
    const Dword style = edit(widget)->edit_style;
    if ((style & 4) || !character)
        return 0;
    if (style & 1)
        character = Dword(toupper(int(character)));
    if (style & 2)
        character = Dword(tolower(int(character)));
    if (events::dispatch(widget, 62, character, Dword(edit_caret(widget))))
        return 0;
    erase_edit_selection(widget);
    if (edit(widget)->character_limit &&
        edit_value(widget)->length >= edit(widget)->character_limit)
        return 0;
    EditString changed;
    string_copy(&changed, edit_value(widget));
    if (wide)
        insert_string_wide_character(&changed, edit_caret(widget), wchar_t(character));
    else
        insert_string_character(&changed, edit_caret(widget), char(character));
    if ((edit(widget)->edit_style & 256) && !edit_layout_flag(widget, 64) &&
        !edit_text_fits(widget, &changed))
        return 0;
    invoke(widget, 7, &changed);
    move_edit_selection(widget, edit(widget)->caret + 1, true);
    return 1;
}

void backspace_edit_character(Widget *widget) {
    if (edit(widget)->edit_style & 4)
        return;
    if ((edit(widget)->edit_style & 8) && edit_caret(widget) != edit_anchor(widget)) {
        erase_edit_selection(widget);
        return;
    }
    if (!edit_caret(widget))
        return;
    move_edit_selection(widget, edit(widget)->caret - 1, true);
    EditString changed;
    string_copy(&changed, edit_value(widget));
    erase_string_character(&changed, edit_caret(widget));
    invoke(widget, 7, &changed);
}

Dword paste_edit_text(Widget *widget) {
    auto *render = active_engine();
    if ((edit(widget)->edit_style & 4) || !render)
        return 0;
    EditString clipboard;
    if (!read_clipboard(&clipboard))
        return 0;
    const Dword font = invoke<Dword>(render, 14, inherited_font_style(widget));
    erase_edit_selection(widget);
    for (int position = 0; position < clipboard.length; ++position)
        if (!insert_edit_character(widget, character_at(&clipboard, position), clipboard.wide))
            break;
    invoke(render, 14, font);
    return 1;
}
// Character operations use the CRT locale.
Dword select_edit_word(Widget *widget, int position) {
    const String *source = edit_value(widget);
    if (!string_data(source))
        return 0;
    auto classified = [&](int index, int mask) {
        const Dword character = character_at(source, index);
        return _isctype(static_cast<signed char>(character), mask) ||
               iswctype(wint_t(character), wctype_t(mask));
    };
    auto nonzero_class = [&](int index, int mask) {
        return classified(index, mask) && character_at(source, index);
    };
    const int initial = position == source->length ? position - 1 : position;
    int start = initial, end = initial;
    if (classified(initial, 8)) {
        do {
            ++end;
        } while (nonzero_class(end, 8));
        while (nonzero_class(start, 8))
            --start;
        if (!classified(start, 16)) {
            int scan = start;
            while (nonzero_class(--scan, 263))
                --start;
        }
    } else if (classified(initial, 16)) {
        do {
            ++end;
        } while (nonzero_class(end, 8));
    } else {
        int scan = start;
        while (nonzero_class(--scan, 263))
            --start;
        while (nonzero_class(end, 263))
            ++end;
        while (nonzero_class(end, 8))
            ++end;
    }
    if (start < 1)
        start = 0;
    if (source->length <= end)
        end = source->length;
    if (start == end)
        return 0;
    set_edit_anchor(widget, start);
    move_edit_selection(widget, end, false);
    events::post_widget(widget, 61, Dword(-1), 0);
    return 1;
}
} // namespace nfs::gui::widgets::family
