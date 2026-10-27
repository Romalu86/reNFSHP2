#include "gui_widget_edit.hpp"
namespace nfs::gui::widgets::family {
using engine::pending::character_at;
using game::runtime::invoke;
// Slot47, five stack arguments.
void __fastcall draw_edit_line(Widget *widget, void *, engine::Engine *render, const String *line,
                               const Rectangle *bounds, const Dword *colour, Dword line_start) {
    if (!(edit(widget)->edit_style & 16) && active_widget_id == widget->id &&
        (edit(widget)->edit_style & 8) && edit_caret(widget) != edit_anchor(widget)) {
        int start = edit_anchor(widget), end = edit_caret(widget);
        if (end < start)
            start = end;
        const int anchor = edit_anchor(widget), caret = edit_caret(widget);
        end = anchor < caret ? caret : anchor;
        const int first = int(line_start);
        int last = first + line->length;
        if (first < start || first < end) {
            if (character_at(line, line->length - 1) == 10)
                --last;
            if (first <= end && start <= last) {
                Rectangle selection{engine::rectangle_methods, 0, 0, 0, 0};
                if (start < first) {
                    selection = *bounds;
                    selection.methods = engine::rectangle_methods;
                    if (last >= end) {
                        Point finish;
                        edit_position_to_screen(widget, &finish, end);
                        selection.width = finish.x - bounds->left;
                        engine::destroy_point(&finish);
                    }
                } else {
                    Point begin{engine::point_methods, bounds->left, bounds->top}, finish;
                    if (start != first)
                        edit_position_to_screen(widget, &begin, start);
                    edit_position_to_screen(widget, &finish, last < end ? last : end);
                    selection.left = begin.x;
                    selection.top = begin.y;
                    selection.width = finish.x - begin.x;
                    selection.height = bounds->height;
                    engine::destroy_point(&finish);
                    engine::destroy_point(&begin);
                }
                if (selection.width < 4)
                    selection.width = 3;
                Dword selection_colour;
                invoke(render, 23, &selection_colour, Dword(11));
                invoke(render, 48, &selection, &selection_colour, Dword(1));
                engine::destroy_rectangle(&selection);
            }
        }
    }
    Dword state_colour;
    if (edit(widget)->edit_style & 64) {
        invoke(render, 21, inherited_palette(widget));
        invoke(render, 23, &state_colour,
               Dword(widget->flags & 1                ? 2
                    : active_widget_id == widget->id ? 1
                                                     : 0));
        colour = &state_colour;
    }
    invoke(render, 42, line, Dword(bounds->left), Dword(bounds->top), colour, Dword(0));
}

void __fastcall edit_text_draw(Widget *widget, void *, Dword render_address) {
    auto *render = reinterpret_cast<engine::Engine *>(render_address);
    Rectangle viewport = *engine::current_viewport_bounds(&render->base);
    viewport.methods = engine::rectangle_methods;
    if (edit(widget)->text_style & 8) {
        Rectangle clip = *invoke<const Rectangle *>(render, 66);
        clip.methods = engine::rectangle_methods;
        engine::intersect_in_place(&clip, &viewport);
        invoke(render, 64, &clip);
        engine::destroy_rectangle(&clip);
    }
    text_draw(widget, nullptr, render_address);
    if (!(edit(widget)->edit_style & 16) && active_widget_id == widget->id) {
        Point caret;
        edit_position_to_screen(widget, &caret, edit_caret(widget));
        invoke(widget, 54, render, reinterpret_cast<Dword>(engine::point_methods),
               Dword(caret.x) - Dword(viewport.left), Dword(caret.y) - Dword(viewport.top));
        engine::destroy_point(&caret);
    }
    if (edit(widget)->text_style & 8)
        invoke(render, 65);
    engine::destroy_rectangle(&viewport);
}

void __fastcall edit_text_prepare_draw(Widget *widget, void *, Dword render_address) {
    prepare_draw(widget, nullptr, reinterpret_cast<engine::Engine *>(render_address));
    if (edit(widget)->edit_style & 32)
        return;
    if (!(edit(widget)->text_style & 8) || active_widget_id != widget->id) {
        widget->scroll.x = engine::default_point.x;
        widget->scroll.y = engine::default_point.y;
        return;
    }
    int x = widget->scroll.x, y = widget->scroll.y;
    Rectangle content, screen;
    text_content_bounds(widget, &content);
    to_screen_rectangle(widget, &screen, &content);
    int height = 0;
    if (auto *render = active_engine()) {
        const Dword font = invoke<Dword>(render, 14, inherited_font_style(widget));
        height = invoke<int>(render, 17);
        invoke(render, 14, font);
    }
    const int length = edit_value(widget)->length;
    edit(widget)->caret = clamp_gui_integer(edit(widget)->caret, 0, length);
    edit(widget)->anchor = clamp_gui_integer(edit(widget)->anchor, 0, length);
    if (x > 0) {
        x = 0;
        widget->scroll.x = x;
        widget->scroll.y = y;
    }
    if (y > 0) {
        y = 0;
        widget->scroll.x = x;
        widget->scroll.y = y;
    }
    Point caret;
    edit_position_to_screen(widget, &caret, edit(widget)->caret);
    if (screen.left + screen.width < caret.x)
        x += screen.left + screen.width - caret.x;
    if (caret.x < screen.left)
        x += screen.left - caret.x;
    if (!edit_layout_flag(widget, 64) && (edit(widget)->text_style & 1)) {
        if (screen.top + screen.height < caret.y + height)
            y += screen.top + screen.height - height - caret.y;
        if (caret.y < screen.top)
            y += screen.top - caret.y;
    }
    widget->scroll.x = x;
    widget->scroll.y = y;
    engine::destroy_point(&caret);
    engine::destroy_rectangle(&screen);
    engine::destroy_rectangle(&content);
}
} // namespace nfs::gui::widgets::family
