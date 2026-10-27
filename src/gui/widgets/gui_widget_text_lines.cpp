#include "gui_widget_text.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include <cstring>
#include <cwchar>
namespace nfs::gui {

int string_find_character(const String *text, Dword character) {
    const Dword wide = text->wide;
    const void *data = string_data(text);
    if (!data)
        return -1;
    if (wide) {
        const auto *start = static_cast<const wchar_t *>(data);
        const auto *found = std::wcschr(start, wchar_t(character));
        return found ? int(found - start) : -1;
    }
    const auto *start = static_cast<const char *>(data);
    const auto *found = std::strchr(start, int(character));
    return found ? int(found - start) : -1;
}

void string_append_character(String *text, char character) {
    if (character) {
        const char tail[2] = {character, 0};
        string_append_ascii(text, tail);
    }
}
} // namespace nfs::gui
namespace nfs::gui::engine {

Rectangle *translate_rectangle(Rectangle *out, const Rectangle *bounds, const Point *offset) {
    const int top = int(Dword(bounds->top) + Dword(offset->y));
    const int left = int(Dword(bounds->left) + Dword(offset->x));
    const int width = bounds->width, height = bounds->height;
    *out = {rectangle_methods, top, left, width, height};
    return out;
}
} // namespace nfs::gui::engine
namespace nfs::gui::widgets::family {
using game::runtime::at;
using game::runtime::invoke;

Rectangle *text_content_bounds(Widget *widget, Rectangle *out) {
    client_bounds(widget, out);
    const Dword margin = at<Dword>(widget, 272);
    out->left = int(Dword(out->left) + margin);
    out->width = int(Dword(out->width) - margin * 2);
    return out;
}
namespace {
// The narrow and UTF-16 branches perform the same word scan.
template <class Character>
void wrap_text_words(const String *source, const Character *start, int offset, String *out,
                     Rectangle *line, const Rectangle *content, Object *render) {
    auto *destination = static_cast<Character *>(text_scratch);
    *destination = 0;
    const Character *cursor = start + offset;
    const Character *newline = cursor;
    while (*newline && *newline != Character(10))
        ++newline;
    if (!*newline)
        newline = nullptr;
    int accepted = 0;
    while (*cursor) {
        String word;
        string_construct(&word);
        Rectangle measured;
        engine::construct_rectangle(&measured);
        const Character *end = cursor;
        while (*end && *end != Character(32))
            ++end;
        string_substring(source, int(cursor - start), int(end - cursor) + 1, &word);
        if (*end)
            ++end;
        int line_break = -1;
        if (newline && newline <= end)
            line_break = string_find_character(&word, 10);
        if (line_break >= 0) {
            string_substring(&word, 0, line_break + 1, &word);
            invoke(render, 15, &word, &measured);
            if (!accepted || int(Dword(line->width) + Dword(measured.width)) <= content->width) {
                measured.left = int(Dword(measured.left) + Dword(line->width));
                union_rectangle(line, &measured);
                const void *data =
                    (word.wide != 0) == (sizeof(Character) == 2) ? string_data(&word) : nullptr;
                std::memcpy(destination, data, Dword(word.length) * sizeof(Character));
                destination[word.length] = 0;
            }
            engine::destroy_rectangle(&measured);
            string_destruct(&word);
            break;
        }
        if (!invoke<Dword>(render, 15, &word, &measured) ||
            (accepted && content->width < int(Dword(line->width) + Dword(measured.width)))) {
            engine::destroy_rectangle(&measured);
            string_destruct(&word);
            break;
        }
        measured.left = int(Dword(measured.left) + Dword(line->width));
        union_rectangle(line, &measured);
        line->left = 0;
        const void *data =
            (word.wide != 0) == (sizeof(Character) == 2) ? string_data(&word) : nullptr;
        std::memcpy(destination, data, Dword(word.length) * sizeof(Character));
        destination += word.length;
        *destination = 0;
        ++accepted;
        engine::destroy_rectangle(&measured);
        string_destruct(&word);
        cursor = end;
    }
    if constexpr (sizeof(Character) == 2)
        string_assign_wide(out, static_cast<const wchar_t *>(text_scratch));
    else
        string_assign(out, static_cast<const char *>(text_scratch));
}
} // namespace

Dword next_text_line(Widget *widget, String *out, Rectangle *line, int *state) {
    auto *render = active_engine();
    if (!render)
        return 0;
    const String *source = invoke<const String *>(widget, 8);
    Rectangle content;
    text_content_bounds(widget, &content);
    const int advance = int(invoke<Dword>(render, 17) + at<Dword>(widget, 276));
    if (!string_data(source)) {
        engine::destroy_rectangle(&content);
        return 0;
    }
    string_copy(out, &lifecycle::construction::unresolved::shared_string);
    line->left = engine::default_bounds.left;
    line->top = engine::default_bounds.top;
    line->width = engine::default_bounds.width;
    line->height = engine::default_bounds.height;
    const int length = source->length;
    const int next = int(Dword(state[0]) + Dword(state[1]));
    if (length <= next) {
        engine::destroy_rectangle(&content);
        return 0;
    }
    state[0] = next;
    state[1] = 0;
    const char *narrow = string_ascii(source);
    const auto *wide =
        !narrow && source->wide ? static_cast<const wchar_t *>(string_data(source)) : nullptr;
    const Dword style = at<Dword>(widget, 264);
    const char mask = at<char>(widget, 268);
    if ((style & 256) && mask) {
        for (int index = 0; index < length; ++index)
            string_append_character(out, mask);
        invoke(render, 15, out, line);
    } else if ((style & 1) && (narrow || wide)) {
        if ((style & 17) == 17) {
            const Dword required = (Dword(length) - Dword(state[0])) * (wide ? 2 : 1) + (wide ? 2 : 1);
            if (text_scratch_busy || !text_scratch || required > 35840) {
                engine::destroy_rectangle(&content);
                return 0;
            }
            text_scratch_busy = 1;
            if (wide)
                wrap_text_words(source, wide, state[0], out, line, &content, render);
            else
                wrap_text_words(source, narrow, state[0], out, line, &content, render);
            --text_scratch_busy;
        } else {
            if (wide)
                string_assign_wide(out, wide + state[0]);
            else
                string_assign(out, narrow + state[0]);
            const int newline = string_find_character(out, 10);
            if (newline >= 0)
                string_substring(out, 0, newline + 1, out);
            invoke(render, 15, out, line);
        }
    } else {
        string_copy(out, source);
        invoke(render, 15, out, line);
    }
    state[1] = out->length;
    int alignment = 0;
    if (style & 2)
        alignment = int(Dword(content.width) - Dword(line->width));
    else if (style & 4)
        alignment = int(Dword(content.width / 2) - Dword(line->width / 2));
    line->left = int(Dword(content.left) + Dword(alignment));
    line->top = state[2];
    line->height = advance;
    state[2] = int(Dword(state[2]) + Dword(advance));
    engine::destroy_rectangle(&content);
    return out->length != 0;
}

Dword first_text_line(Widget *widget, String *out, Rectangle *line, int *state) {
    if (!string_data(invoke<const String *>(widget, 8)))
        return 0;
    state[0] = state[1] = state[2] = 0;
    const Dword style = at<Dword>(widget, 264);
    if ((style & 128) && !(style & 1)) {
        int height = 0;
        if (auto *render = active_engine()) {
            const Dword font = invoke<Dword>(render, 14, inherited_font_style(widget));
            height = invoke<int>(render, 17);
            invoke(render, 14, font);
        }
        state[2] = int(Dword(widget->bounds.height) - Dword(height)) / 2;
    }
    if (next_text_line(widget, out, line, state))
        return 1;
    Rectangle content;
    text_content_bounds(widget, &content);
    string_copy(out, &lifecycle::construction::unresolved::shared_string);
    const Dword current_style = at<Dword>(widget, 264);
    line->left = int(Dword(content.left) + Dword(current_style & 2   ? content.width
                                               : current_style & 4 ? content.width / 2
                                                                   : 0));
    line->top = state[2];
    Dword height = 0;
    if (auto *render = active_engine()) {
        const Dword font = invoke<Dword>(render, 14, inherited_font_style(widget));
        height = invoke<Dword>(render, 17);
        invoke(render, 14, font);
    }
    line->height = int(height);
    line->width = 0;
    engine::destroy_rectangle(&content);
    return 1;
}
} // namespace nfs::gui::widgets::family
