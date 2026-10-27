#include "gui_widget_list.hpp"
#include "nfs/core/application/application.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
namespace nfs::gui::widgets::family {
using game::runtime::invoke;

int list_pointer_row(Widget *widget) {
    auto *list = list_box(widget);
    if (!invoke<Dword>(widget, 9) || (widget->flags & 1) || !list->items.grid.rows.count)
        return -1;
    Point local;
    from_screen_point(widget, &local, pending::pointer_position());
    const int y = local.y;
    engine::destroy_point(&local);
    if (!(list->style & 32)) {
        if (list->row_height) {
            const int first = first_visible_list_row(widget);
            return y / list->row_height + first;
        }
    } else {
        const int count = int(list->items.grid.rows.count);
        const int distance = y - widget->scroll.y;
        int top = 0;
        for (int row = 0; row < count; ++row) {
            const int height = list_row_height(widget, row);
            if (top <= distance && distance < top + height)
                return row;
            top += height;
        }
    }
    return -1;
}

int list_pointer_check_column(Widget *widget) {
    if (!invoke<Dword>(widget, 9) || (widget->flags & 1))
        return -1;
    for (int column = 0; column < int(list_box(widget)->items.grid.columns.count); ++column)
        if (auto *check = invoke<Widget *>(widget, 48, column)) {
            Rectangle local, screen;
            client_bounds(check, &local);
            to_screen_rectangle(check, &screen, &local);
            engine::destroy_rectangle(&local);
            const int x = pending::pointer_position()->x;
            const bool inside = screen.left <= x && x < screen.left + screen.width;
            engine::destroy_rectangle(&screen);
            if (inside)
                return column;
        }
    return -1;
}

Dword __fastcall list_box_button_down(Widget *widget, void *, Dword button, Dword modifiers) {
    if (button == 7 || button == 3 || button == 4)
        return invoke<Dword>(widget, 32,
                            Dword(button == 7   ? 28
                                 : button == 3 ? 208
                                               : 200),
                            modifiers);
    return pending::general_button(reinterpret_cast<GeneralWidget *>(widget), nullptr, button,
                                   modifiers);
}

Dword __fastcall list_box_key_down(Widget *widget, void *, Dword key, Dword modifiers) {
    auto *list = list_box(widget);
    auto *items = &list->items;
    const auto changed = [&](Dword result) {
        if (result)
            events::post_widget(widget, 68, 0, 0);
        return Dword(1);
    };
    const auto select_cursor = [&]() {
        if ((list->style & 1) && (modifiers & 196608))
            return invoke<Dword>(items, 11, list->anchor, list->cursor);
        return invoke<Dword>(items, 10, list->cursor, Dword(1));
    };
    const int count = int(items->grid.rows.count);
    switch (key) {
    case 1:
        release_capture(widget);
        break;
    case 28:
        if (!std::uint8_t(modifiers) && selected_list_row(items) >= 0)
            events::post_widget(widget, 40, 0, 0);
        return 1;
    case 30:
        if ((list->style & 1) && !std::uint8_t(modifiers) && (modifiers & 786432))
            return changed(invoke<Dword>(items, 11, Dword(0), items->grid.rows.count - 1));
        break;
    case 57:
        if ((list->style & 3) == 2 && !std::uint8_t(modifiers)) {
            const int row = selected_list_row(items);
            if (row >= 0)
                for (int column = 0; column < int(items->grid.columns.count); ++column)
                    if (invoke<Widget *>(widget, 48, column))
                        if (auto *item = invoke<ListItem *>(items, 9, row, column);
                            item && !(item->flags & 2)) {
                            const Dword flags = item->flags, checked = (~flags >> 3) & 1;
                            events::post_widget(
                                widget, 69, (Dword(column) & 65535) << 16 | (Dword(row) & 65535),
                                checked);
                            item->flags = checked ? flags | 8 : flags & ~Dword(8);
                        }
            return 1;
        }
        break;
    case 203:
        pending::previous_tab(widget);
        return 1;
    case 205:
        pending::next_tab(widget);
        return 1;
    case 199:
    case 200:
    case 201:
    case 207:
    case 208:
    case 209: {
        if ((key == 200 || key == 208) && selected_list_row(items) < 0) {
            const int row =
                key == 200 ? previous_list_row(items, count) : next_list_row(items, -1);
            return changed(invoke<Dword>(items, 10, row, Dword(1)));
        }
        const int before = list->cursor;
        const bool up = key == 199 || key == 200 || key == 201;
        if (up ? before < 1 : count - 1 <= before)
            return 1;
        int row;
        if (key == 199)
            row = next_list_row(items, -1);
        else if (key == 200)
            row = previous_list_row(items, before);
        else if (key == 207)
            row = previous_list_row(items, count);
        else if (key == 208)
            row = next_list_row(items, before);
        else {
            const int page = visible_list_rows(widget);
            const int candidate = up ? before - page : before + page;
            row = up ? previous_list_row(items, candidate) : next_list_row(items, candidate);
            if (row < 0)
                row = up ? next_list_row(items, candidate) : previous_list_row(items, candidate);
        }
        const bool directional = key == 199 || key == 201   ? row < before
                                 : key == 207 || key == 209 ? before < row
                                                              : true;
        if (directional && row >= 0 && row < int(items->grid.rows.count)) {
            list->cursor = row;
            ensure_list_box_row_visible(widget, row);
        }
        return changed(select_cursor());
    }
    }
    return pending::general_key(reinterpret_cast<GeneralWidget *>(widget), nullptr, key, modifiers);
}

void __fastcall list_box_pointer_idle(Widget *widget, void *, Dword first, Dword second) {
    if (list_box(widget)->style & 8) {
        const String *tooltip = &lifecycle::construction::unresolved::shared_string;
        if (invoke<Dword>(widget, 9))
            if (auto *item = invoke<ListItem *>(&list_box(widget)->items, 9,
                                                list_pointer_row(widget), Dword(0)))
                tooltip = &item->tooltip;
        string_copy(&widget->tooltip, tooltip);
    }
    style_result_38(widget, nullptr, first, second);
}

void __fastcall list_box_pointer_move(Widget *widget, void *, Dword first, Dword second) {
    auto *list = list_box(widget);
    if (list->style & 16) {
        const int row = list_pointer_row(widget);
        if (row >= 0 && !list_row_selected(&list->items, Dword(row)))
            if (invoke<Dword>(&list->items, 10, row, Dword(1)))
                events::post_widget(widget, 68, 0, 0);
    }
    if (list->style & 8)
        post_target(inherited_tooltip_owner(widget), 76, 0, 0);
    pending::update_active_style(widget, nullptr, first, second);
}

Dword __fastcall list_box_pointer_press(Widget *widget, void *, Dword button, Dword modifiers) {
    if (button != 0 && button != 1)
        return pending::update_focus_style(widget, nullptr, button, modifiers);
    auto *list = list_box(widget);
    const int height = widget->bounds.height;
    const int row = list_pointer_row(widget);
    bool check_handled = false;
    if ((list->style & 2) && row >= 0 && application::pending::captured_widget_id != widget->id &&
        !std::uint8_t(modifiers)) {
        const int column = list_pointer_check_column(widget);
        if (column >= 0 && !list_row_disabled(&list->items, Dword(row))) {
            if (auto *item = invoke<ListItem *>(&list->items, 9, row, column);
                item && !(item->flags & 2)) {
                const Dword flags = item->flags, checked = (~flags >> 3) & 1;
                events::post_widget(widget, 69, Dword(column) << 16 | (Dword(row) & 65535),
                                    checked);
                item->flags = checked ? flags | 8 : flags & ~Dword(8);
            }
            check_handled = true;
        }
    }
    if (!check_handled) {
        if (row >= 0) {
            capture_widget(widget);
            if (!button && list_row_selected(&list->items, Dword(row)) && (modifiers & 33554432) &&
                !(list->style & 16)) {
                events::post_widget(widget, 40, 0, 0);
                return 1;
            }
            if (!std::uint8_t(modifiers)) {
                if (button == 1) {
                    if (!list_row_selected(&list->items, Dword(row)))
                        invoke<Dword>(&list->items, 10, row, Dword(1));
                    else if (row < int(list->items.grid.rows.count)) {
                        list->cursor = row;
                        ensure_list_box_row_visible(widget, row);
                    }
                } else if ((list->style & 1) && (modifiers & 786432))
                    invoke<Dword>(&list->items, 12, row,
                                 Dword(!list_row_selected(&list->items, Dword(row))));
                else if ((list->style & 1) && (modifiers & 196608))
                    invoke<Dword>(&list->items, 11, list->anchor, row);
                else
                    invoke<Dword>(&list->items, 10, row, Dword(1));
            }
        } else if (application::pending::captured_widget_id == widget->id &&
                   std::uint8_t(modifiers)) {
            Point local;
            from_screen_point(widget, &local, pending::pointer_position());
            if (height < local.y && list->cursor < int(list->items.grid.rows.count) - 1)
                events::dispatch(widget, 39, 1, 0);
            else if (local.y < 0 && list->cursor > 0)
                events::dispatch(widget, 39, 0, 0);
            engine::destroy_point(&local);
        }
    }
    activate(widget);
    return 1;
}

Dword __fastcall list_box_pointer_release(Widget *widget, void *, Dword button, Dword modifiers) {
    if (button != 0 && button != 1)
        return capture_style(widget, nullptr, button, modifiers);
    if (application::pending::captured_widget_id != widget->id)
        return 1;
    if (selected_list_row(&list_box(widget)->items) >= 0)
        events::post_widget(widget, (list_box(widget)->style & 16) && button == 0 ? 40 : 68,
                            0, 0);
    release_capture(widget);
    return 1;
}
} // namespace nfs::gui::widgets::family
