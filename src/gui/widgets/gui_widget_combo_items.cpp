#include "gui_widget_combo.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
namespace nfs::gui::widgets::family {
using game::runtime::at;
using game::runtime::invoke;

void erase_combo_row(Widget *widget, int row) {
    if (auto *list = combo_list(widget))
        invoke(&list_box(list)->items, 5, row);
}

void clear_combo_rows(Widget *widget) {
    if (auto *list = combo_list(widget))
        invoke(&list_box(list)->items, 1);
    invoke(widget, 7, &lifecycle::construction::unresolved::shared_string);
    at<int>(widget, 340) = -1;
}

int append_combo_item(Widget *widget, const String *text, Dword value, Dword disabled) {
    ListItem item;
    construct_text_list_item(&item, text);
    int row;
    try {
        item.data = value;
        item.flags = disabled ? item.flags | 2 : item.flags & ~Dword(2);
        if (auto *list = combo_list(widget)) {
            auto *items = &list_box(list)->items;
            row = int(items->grid.rows.count);
            invoke<Dword>(items, 7, &item, row, Dword(0));
        } else
            row = -1;
    } catch (...) {
        string_destruct(&item.tooltip);
        string_destruct(&item.text);
        throw;
    }
    string_destruct(&item.tooltip);
    string_destruct(&item.text);
    return row;
}

int find_combo_value(Widget *widget, Dword value) {
    if (auto *list = combo_list(widget))
        for (int row = 0; row < int(list_box(list)->items.grid.rows.count); ++row)
            if (auto *item = invoke<ListItem *>(&list_box(list)->items, 9, row, Dword(0));
                item && item->data == value)
                return row;
    return -1;
}

Dword enable_combo_row(Widget *widget, int row, Dword enabled) {
    if (auto *list = combo_list(widget))
        return set_list_row_disabled(&list_box(list)->items, Dword(row), enabled == 0);
    return 0;
}

int combo_row_count(Widget *widget) {
    if (auto *list = combo_list(widget))
        return int(list_box(list)->items.grid.rows.count);
    return 0;
}

Dword select_combo_text(Widget *widget, const String *text) {
    if (auto *list = combo_list(widget))
        for (int row = 0; row < int(list_box(list)->items.grid.rows.count); ++row)
            if (auto *item = invoke<ListItem *>(&list_box(list)->items, 9, row, Dword(0));
                item && string_equal(&item->text, text))
                return select_combo_row(widget, row);
    return select_combo_row(widget, -1);
}

ListItem *combo_item(Widget *widget, int row) {
    if (auto *list = combo_list(widget))
        return invoke<ListItem *>(&list_box(list)->items, 9, row, Dword(0));
    return nullptr;
}
} // namespace nfs::gui::widgets::family
