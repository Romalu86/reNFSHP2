#include "../lifecycle/gui_ime.hpp"
namespace nfs::gui::widgets::ime {
using game::runtime::invoke;
static Widget *widget(Edit *edit) {
    return &edit->base.base.base;
}

void __fastcall parent_leave(Edit *edit, void *, Dword argument) {
    target_id = widget(edit)->id;
    if (context) {
        open_context(edit);
        select_layout(edit, GetKeyboardLayout(0));
    }
    family::edit_text_parent_leave(widget(edit), nullptr, argument);
}

Dword __fastcall character(Edit *edit, void *, Dword character, Dword modifiers) {
    return family::edit_text_character(widget(edit), nullptr, character, modifiers);
}

void __fastcall edit_operation(Edit *edit, void *, Dword position) {
    family::edit_text_item_operation_51(widget(edit), nullptr, position);
    if (position == Dword(-1) && context_open(edit) && context) {
        ImmNotifyIME(context, 21, 4, 0);
        Dword event, first, second;
        while (family::take_widget_event(widget(edit), &event, &first, &second, 5, 5, 1)) {
        }
        edit->pending_character = 0;
    }
}

void __fastcall parent_enter(Edit *edit, void *, Widget *next) {
    if (next && !invoke<Dword>(next, 2, edit_class_key)) {
        target_id = 0;
        if (context)
            close_context(edit);
    }
    family::edit_text_parent_enter(widget(edit), nullptr, reinterpret_cast<Dword>(next));
}

void __fastcall update_composition(Edit *edit, void *, std::uint16_t character, Dword flags) {
    if (!edit->composing || !character)
        return;
    if (edit->composition_caret != family::edit_caret(widget(edit)))
        edit->pending_character = 0;
    if (!edit->pending_character)
        character = first_composition_character(edit);
    if (!flags) {
        invoke(edit, 57);
    } else {
        if (!edit->pending_character)
            family::insert_edit_character(widget(edit), character, 1);
        else
            family::overwrite_edit_character(widget(edit), character, 1);
        edit->pending_character = (flags & GCS_RESULTSTR) ? 0 : character;
    }
    edit->composition_caret = family::edit_caret(widget(edit));
    if (invoke<const String *>(edit, 8)->wide) {
        OutputDebugStringW(
            static_cast<const wchar_t *>(string_data(invoke<const String *>(edit, 8))));
        OutputDebugStringA("\n");
    }
}

void __fastcall begin_composition(Edit *edit, void *) {
    edit->composing = 1;
    edit->pending_character = 0;
    edit->composition_caret = family::edit_caret(widget(edit));
}

void __fastcall end_composition(Edit *edit, void *) {
    edit->composing = 0;
    edit->pending_character = 0;
}

Dword __fastcall navigate(Edit *edit, void *, Dword event, Dword first, Dword second) {
    if (int(event) < 206 || (int(event) > 208 && event != 211) ||
        (enabled_in_ancestors(widget(edit)) && active_widget_id == widget(edit)->id))
        return pending::navigate(widget(edit), nullptr, event, first, second);
    return 1;
}

Dword __fastcall filter_character(Edit *edit, void *, Dword character, Dword modifiers) {
    if (edit->composing)
        return 0;
    if (context) {
        if (IsDBCSLeadByte(BYTE(character)))
            return 0;
        edit->pending_character = 0;
    }
    return invoke<Dword>(edit, 35, character, modifiers);
}

Dword __fastcall event(Edit *edit, void *, Dword event, Dword first, Dword second) {
    switch (event) {
    case 5:
        return 1;
    case 202:
    case 203:
        select_layout(edit, reinterpret_cast<HKL>(second));
        break;
    case 206:
        invoke(edit, 56);
        break;
    case 207:
        invoke(edit, 55,
               unicode_composition ? std::uint16_t(first)
                                   : decode_character(edit, std::uint16_t(first)),
               second);
        break;
    case 208:
        invoke(edit, 57);
        break;
    case 210:
        notify_context(edit, first, second);
        break;
    case 211:
        invoke(edit, 58, first, second);
        break;
    case 213: {
        wchar_t decoded = 0;
        char narrow[2];
        narrow[0] = char(first);
        if (MultiByteToWideChar(codepage, 0, narrow, 2, &decoded, 1))
            invoke(edit, 58, decoded, second);
        break;
    }
    }
    return family::edit_text_event(widget(edit), nullptr, event, first, second);
}
// The final three words are a Point passed by value.
void __fastcall draw_caret(Edit *edit, void *, engine::Engine *render, Dword point_methods, Dword x,
                           Dword y) {
    int height = invoke<int>(render, 17);
    if (!(edit->base.text_style & 1)) {
        Rectangle bounds;
        family::text_content_bounds(widget(edit), &bounds);
        height = bounds.height;
        engine::destroy_rectangle(&bounds);
    }
    Point start{reinterpret_cast<const Destroy *>(point_methods), int(x), int(y)};
    if (height) {
        const Rectangle *viewport = engine::current_viewport_bounds(&render->base);
        start.x += viewport->left;
        start.y += viewport->top;
        if (edit->composing && edit->pending_character) {
            invoke(render, 21, family::inherited_palette(widget(edit)));
            Rectangle bounds{engine::rectangle_methods, start.y, start.x, 0, 0};
            Dword metrics[6];
            const Dword previous = engine::pending::character_at(
                family::edit_value(widget(edit)), family::edit_caret(widget(edit)) - 1);
            if (invoke<Dword>(render, 16, previous, metrics)) {
                bounds.left -= int(metrics[1]);
                bounds.width = int(metrics[1]);
            }
            bounds.height = height;
            Dword colour;
            const auto *selected_colour = invoke<const Dword *>(render, 23, &colour, Dword(11));
            invoke(render, 48, &bounds, selected_colour, Dword(1));
            engine::destroy_rectangle(&bounds);
        }
        if (invoke<Dword>(render, 59)) {
            engine::destroy_point(&start);
            return;
        }
        Point end{engine::point_methods, start.x, int(Dword(start.y) + Dword(height))};
        const Dword colour = widget(edit)->colour;
        invoke(render, 46, &start, &end, colour, colour);
        ++start.x;
        ++end.x;
        invoke(render, 46, &start, &end, colour, colour);
        engine::destroy_point(&end);
    }
    engine::destroy_point(&start);
}
} // namespace nfs::gui::widgets::ime
