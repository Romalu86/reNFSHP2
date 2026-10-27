#include "gui_widget_edit.hpp"
namespace nfs::gui::widgets::family {
using engine::pending::character_at;
using game::runtime::invoke;

int edit_caret(Widget *widget) {
    return clamp_gui_integer(edit(widget)->caret, 0, edit_value(widget)->length);
}

int edit_anchor(Widget *widget) {
    return clamp_gui_integer(edit(widget)->edit_style & 8 ? edit(widget)->anchor
                                                          : edit(widget)->caret,
                             0, edit_value(widget)->length);
}

Point *edit_position_to_screen(Widget *widget, Point *out, int position) {
    Rectangle content;
    text_content_bounds(widget, &content);
    const Point scroll{engine::point_methods, widget->scroll.x, widget->scroll.y};
    Point result{engine::point_methods, content.left, content.top};
    EditString line;
    Rectangle line_bounds{engine::rectangle_methods, 0, 0, 0, 0};
    Rectangle measured{engine::rectangle_methods, 0, 0, 0, 0};
    const String *source = edit_value(widget);
    auto *render = active_engine();
    if (render) {
        invoke(render, 14, inherited_font_style(widget));
        int state[3];
        if (!first_text_line(widget, &line, &line_bounds, state)) {
            if (edit(widget)->text_style & 4)
                result.x = content.left + content.width / 2;
            else if (edit(widget)->text_style & 2)
                result.x = content.left + content.width;
        } else {
            bool contains = state[0] <= position && position <= state[0] + state[1];
            if (!contains)
                while (next_text_line(widget, &line, &line_bounds, state))
                    if (state[0] <= position && position <= state[0] + state[1]) {
                        contains = true;
                        break;
                    }
            if (contains) {
                EditString prefix;
                measured.width = 0;
                string_substring(&line, 0, position - state[0], &prefix);
                invoke(render, 15, &prefix, &measured);
                result.x = measured.left + line_bounds.left + measured.width;
                result.y = line_bounds.top;
            }
            if (character_at(source, position - 1) == 10) {
                result.y += invoke<int>(render, 17);
                if (edit(widget)->text_style & 2)
                    result.x = content.left + content.width;
                else if (edit(widget)->text_style & 4)
                    result.x = content.left + content.width / 2;
                else
                    result.x = content.left;
            }
        }
    }
    result.x = int(Dword(result.x) + Dword(scroll.x));
    result.y = int(Dword(result.y) + Dword(scroll.y));
    to_screen_point(widget, out, &result);
    engine::destroy_rectangle(&measured);
    engine::destroy_rectangle(&line_bounds);
    engine::destroy_point(&result);
    engine::destroy_rectangle(&content);
    return out;
}
namespace {
void line_to_screen(Widget *widget, Rectangle *line) {
    Rectangle client;
    client_bounds(widget, &client);
    line->left += client.left + widget->scroll.x;
    line->top += client.top + widget->scroll.y;
    Rectangle screen;
    to_screen_rectangle(widget, &screen, line);
    line->left = screen.left;
    line->top = screen.top;
    line->width = screen.width;
    line->height = screen.height;
    engine::destroy_rectangle(&screen);
    engine::destroy_rectangle(&client);
}
int character_inside_line(Object *render, const String *line, const String *source,
                          const Rectangle *bounds, const int *state, int x) {
    Rectangle measured{engine::rectangle_methods, 0, 0, 0, 0};
    int accepted = state[1];
    int previous;
    for (;;) {
        previous = accepted - 1;
        if (!previous)
            break;
        EditString prefix;
        string_substring(line, 0, previous, &prefix);
        if (!invoke<Dword>(render, 15, &prefix, &measured) ||
            measured.left + measured.width < x - bounds->left)
            break;
        accepted = previous;
    }
    if (character_at(source, accepted + state[0]) != 10)
        previous = accepted;
    engine::destroy_rectangle(&measured);
    return previous + state[0];
}
} // namespace

int edit_screen_to_position(Widget *widget, const Point *point) {
    EditString line;
    Rectangle bounds{engine::rectangle_methods, 0, 0, 0, 0};
    const String *source = edit_value(widget);
    auto *render = active_engine();
    if (!render)
        return -1;
    invoke(render, 14, inherited_font_style(widget));
    int state[3];
    if (!first_text_line(widget, &line, &bounds, state))
        return -1;
    line_to_screen(widget, &bounds);
    if (point->y < bounds.top + bounds.height || !(edit(widget)->text_style & 1)) {
        if (point->x <= bounds.left)
            return 0;
        if (point->x < bounds.left + bounds.width)
            return character_inside_line(render, &line, source, &bounds, state, point->x);
        const int end = state[0] + state[1];
        if (end == source->length)
            return character_at(source, end - 1) == 10 ? end - 1 : end;
        return end - 1;
    }
    const int y = point->y;
    for (;;) {
        if (!next_text_line(widget, &line, &bounds, state))
            return source->length;
        line_to_screen(widget, &bounds);
        if (bounds.top <= y && y < bounds.top + bounds.height)
            break;
    }
    if (point->x <= bounds.left)
        return state[0];
    if (point->x < bounds.left + bounds.width)
        return character_inside_line(render, &line, source, &bounds, state, point->x);
    const int end = state[0] + state[1];
    return character_at(source, end) == 0 && character_at(source, end - 1) != 10 ? end : end - 1;
}

Dword edit_text_fits(Widget *widget, const String *candidate) {
    auto *render = active_engine();
    if (!render)
        return 0;
    const Dword previous_font = invoke<Dword>(render, 14, inherited_font_style(widget));
    EditString saved;
    string_copy(&saved, edit_value(widget));
    invoke(widget, 7, candidate);
    Rectangle content;
    text_content_bounds(widget, &content);
    EditString line;
    Rectangle extent{engine::rectangle_methods, 0, 0, 0, 0};
    int state[3];
    Dword fits = 1;
    if (first_text_line(widget, &line, &extent, state)) {
        Rectangle next{engine::rectangle_methods, 0, 0, 0, 0};
        while (next_text_line(widget, &line, &next, state))
            union_rectangle(&extent, &next);
        if (character_at(candidate, candidate->length - 1) == 10)
            extent.height += invoke<int>(render, 17);
        if (content.width < extent.width || content.height < extent.height)
            fits = 0;
        engine::destroy_rectangle(&next);
    }
    invoke(widget, 7, &saved);
    invoke(render, 14, previous_font);
    engine::destroy_rectangle(&extent);
    engine::destroy_rectangle(&content);
    return fits;
}
} // namespace nfs::gui::widgets::family
