#include "gui_widget_items.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
namespace nfs::gui::widgets::family {
using game::runtime::at;
using game::runtime::invoke;

static ListItems *__fastcall delete_list_items(ListItems *grid, void *, Dword flags) {
    destroy_list_items(grid);
    if (flags & 1)
        application_release(grid);
    return grid;
}

 void __fastcall clear_list_items(ListItems *grid, void *) {
    clear_list_selection(grid, nullptr);
    container_clear(&grid->grid.rows);
}

 Dword __fastcall clear_list_cell(ListItems *grid, void *, Dword row, Dword column) {
    return grid_set_cell(&grid->grid, nullptr, row, column, dispose_object);
}

 int __fastcall append_list_row(ListItems *grid, void *) {
    grid_add_row(&grid->grid);
    return int(grid->grid.rows.count) - 1;
}

 Dword __fastcall insert_list_row(ListItems *grid, void *, Dword row) {
    return grid_insert_row(&grid->grid, row);
}

 Dword __fastcall erase_list_row(ListItems *grid, void *, Dword column) {
    return grid_remove_row(&grid->grid, column);
}

 void __fastcall reorder_list_rows(ListItems *grid, void *, int first, int second) {
    if (!grid->grid.rows.count)
        return;
    const int last = int(grid->grid.rows.count) - 1, minimum = last < 0 ? last : 0;
    if (first < minimum)
        first = minimum;
    else if (last < first)
        first = last;
    if (second < minimum)
        second = minimum;
    else if (last < second)
        second = last;
    grid_reorder_rows(&grid->grid, first, second);
}

 Dword __fastcall copy_list_cell(ListItems *grid, void *, const ListItem *source, int row,
                                      int column) {
    while (int(grid->grid.columns.count) <= column)
        grid_add_column(&grid->grid, nullptr, dispose_object);
    while (int(grid->grid.rows.count) <= row)
        invoke<int>(grid, 3);
    auto *item = static_cast<ListItem *>(allocate_flags(sizeof(ListItem), 0, "GListItem"));
    if (!item)
        return 1;
    string_construct(&item->text);
    string_construct(&item->tooltip);
    item->methods = list_item_methods;
    try {
        string_copy(&item->text, &source->text);
        item->data = source->data;
        item->flags = source->flags;
        string_copy(&item->tooltip, &source->tooltip);
        item->image = source->image;
    } catch (...) {
        string_destruct(&item->tooltip);
        string_destruct(&item->text);
        item->methods = reinterpret_cast<const Method *>(gui::base_methods);
        release_flags(item, 0, "GListItem");
        throw;
    }
    const Dword result = grid_set_cell(&grid->grid, item, Dword(row), Dword(column), dispose_object);
    if (result)
        invoke(item, 0, Dword(1));
    return result;
}

 void __fastcall append_list_cell(ListItems *grid, void *, const ListItem *item) {
    invoke<Dword>(grid, 7, item, grid->grid.rows.count, Dword(0));
}

 void *__fastcall list_cell(ListItems *grid, void *, Dword row, Dword column) {
    return grid_cell(&grid->grid, row, column);
}

 Dword __fastcall select_single_row(ListItems *grid, void *, Dword row, Dword selected) {
    auto *before = invoke<ListItem *>(grid, 9, row, Dword(0));
    const Dword previous = before ? before->flags & 1 : 0;
    auto *item = invoke<ListItem *>(grid, 9, row, Dword(0));
    if (!item || (item->flags & 2))
        return 0;
    const Dword flags = item->flags;
    clear_list_selection(grid, nullptr);
    item->flags = selected ? flags | 1 : flags & ~Dword(1);
    return selected != previous;
}

 Dword __fastcall select_row(ListItems *grid, void *, Dword row, Dword selected) {
    auto *item = invoke<ListItem *>(grid, 9, row, Dword(0));
    if (item && !(item->flags & 2) && (item->flags & 1) != selected) {
        item->flags = selected ? item->flags | 1 : item->flags & ~Dword(1);
        return 1;
    }
    return 0;
}

 Dword __fastcall select_row_range(ListItems *grid, void *, int first, int second) {
    Dword changed = 0;
    const auto deselect = [&](int index) {
        auto *item = invoke<ListItem *>(grid, 9, index, Dword(0));
        if (item && !(item->flags & 2) && (item->flags & 1)) {
            item->flags &= ~Dword(1);
            return Dword(1);
        }
        return Dword(0);
    };
    if (first >= 0 && first < int(grid->grid.rows.count)) {
        if (second <= first) {
            for (int index = 0; index < second; ++index)
                changed += deselect(index);
            for (int index = first + 1; index < int(grid->grid.rows.count); ++index)
                changed += deselect(index);
            do {
                changed += select_row(grid, nullptr, Dword(first), 1);
                --first;
            } while (second <= first);
        } else {
            for (int index = 0; index < first; ++index)
                changed += deselect(index);
            for (int index = second + 1; index <= int(grid->grid.rows.count); ++index)
                changed += deselect(index);
            do {
                changed += select_row(grid, nullptr, Dword(first), 1);
                ++first;
            } while (first <= second);
        }
        return changed;
    }
    auto *before = invoke<ListItem *>(grid, 9, second, Dword(0));
    const Dword previous = before ? before->flags & 1 : 0;
    auto *item = invoke<ListItem *>(grid, 9, second, Dword(0));
    if (!item || (item->flags & 2))
        return 0;
    clear_list_selection(grid, nullptr);
    grid_set_selected_cell(item, 1);
    return previous != 1;
}
#define METHOD(function) reinterpret_cast<Method>(function)
const Method list_items_methods[14] = {
    METHOD(delete_list_items), METHOD(clear_list_items),    METHOD(clear_list_cell),
    METHOD(append_list_row),   METHOD(insert_list_row),     METHOD(erase_list_row),
    METHOD(reorder_list_rows), METHOD(copy_list_cell),      METHOD(append_list_cell),
    METHOD(list_cell),         METHOD(select_single_row),   METHOD(select_row_range),
    METHOD(select_row),        METHOD(clear_list_selection)};

Widget *__fastcall delete_toggle_items(ListItems *items, void *, Dword flags) {
    return toggle_button_delete(reinterpret_cast<Widget *>(game::runtime::bytes(items) - 312),
                                nullptr, flags);
}

Widget *__fastcall delete_list_box_items(ListItems *items, void *, Dword flags) {
    return list_box_delete(reinterpret_cast<Widget *>(game::runtime::bytes(items) - 264), nullptr,
                           flags);
}

void __fastcall clear_list_box_items(ListItems *items, void *) {
    clear_list_items(items, nullptr);
    at<int>(items, -144) = engine::default_point.x;
    at<int>(items, -140) = engine::default_point.y;
    at<int>(items, 52) = at<int>(items, 48) = -1;
}
void ensure_list_box_row_visible(Widget *, int);
void __fastcall erase_list_box_row(ListItems *, void *, Dword);

Dword __fastcall select_list_box_item(ListItems *items, void *, Dword row, Dword selected) {
    if (!select_single_row(items, nullptr, row, selected))
        return 0;
    auto *widget = reinterpret_cast<Widget *>(game::runtime::bytes(items) - 264);
    if (int(row) >= 0 && int(row) < int(at<Dword>(widget, 280)))
        at<Dword>(widget, 312) = row;
    if (int(row) >= 0 && int(row) < int(at<Dword>(widget, 280))) {
        at<Dword>(widget, 316) = row;
        ensure_list_box_row_visible(widget, int(row));
    }
    return 1;
}

Dword __fastcall select_list_box_range(ListItems *items, void *, Dword first, Dword last) {
    if (!select_row_range(items, nullptr, int(first), int(last)))
        return 0;
    auto *widget = reinterpret_cast<Widget *>(game::runtime::bytes(items) - 264);
    if (int(first) >= 0 && int(first) < int(at<Dword>(widget, 280)))
        at<Dword>(widget, 312) = first;
    if (int(last) >= 0 && int(last) < int(at<Dword>(widget, 280))) {
        at<Dword>(widget, 316) = last;
        ensure_list_box_row_visible(widget, int(last));
    }
    return 1;
}

Dword __fastcall set_list_box_selection(ListItems *items, void *, Dword row, Dword selected) {
    if (!select_row(items, nullptr, row, selected))
        return 0;
    if (int(row) >= 0 && int(row) < int(items->grid.rows.count)) {
        at<Dword>(items, 52) = row;
        ensure_list_box_row_visible(reinterpret_cast<Widget *>(game::runtime::bytes(items) - 264),
                                    int(row));
    }
    return 1;
}

void __fastcall clear_list_box_selection(ListItems *items, void *) {
    clear_list_selection(items, nullptr);
    at<int>(items, 52) = at<int>(items, 48) = -1;
}
const Method toggle_items_methods[14] = {
    METHOD(delete_toggle_items), METHOD(clear_list_items),    METHOD(clear_list_cell),
    METHOD(append_list_row),     METHOD(insert_list_row),     METHOD(erase_list_row),
    METHOD(reorder_list_rows),   METHOD(copy_list_cell),      METHOD(append_list_cell),
    METHOD(list_cell),           METHOD(select_single_row),   METHOD(select_row_range),
    METHOD(select_row),          METHOD(clear_list_selection)};
const Method list_box_items_methods[14] = {
    METHOD(delete_list_box_items),  METHOD(clear_list_box_items),
    METHOD(clear_list_cell),        METHOD(append_list_row),
    METHOD(insert_list_row),        METHOD(erase_list_box_row),
    METHOD(reorder_list_rows),      METHOD(copy_list_cell),
    METHOD(append_list_cell),       METHOD(list_cell),
    METHOD(select_list_box_item),   METHOD(select_list_box_range),
    METHOD(set_list_box_selection), METHOD(clear_list_box_selection)};
#undef METHOD
} // namespace nfs::gui::widgets::family
