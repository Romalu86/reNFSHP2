#include "gui_widget_items.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
namespace nfs::gui::widgets::family {

void *grid_cell(ItemGrid *grid, Dword row, Dword column) {
    auto *cells = row < grid->rows.count
                      ? static_cast<Container *>(events::container_value(&grid->rows, row))
                      : nullptr;
    return cells && column < cells->count ? events::container_value(cells, column) : nullptr;
}

Dword grid_set_cell(ItemGrid *grid, void *value, Dword row, Dword column, Dispose dispose) {
    if (row < grid->rows.count && column < grid->columns.count) {
        auto *cells = static_cast<Container *>(events::container_value(&grid->rows, row));
        if (cells)
            return container_set_at(cells, value, column, dispose);
    }
    return 3;
}

void grid_add_column(ItemGrid *grid, void *value, Dispose dispose) {
    if (!container_append(&grid->columns, value, dispose)) {
        const Dword count = grid->rows.count;
        for (Dword row = 0; row < count; ++row)
            if (row < grid->rows.count)
                if (auto *cells =
                        static_cast<Container *>(events::container_value(&grid->rows, row)))
                    container_append(cells, nullptr, nullptr);
    }
}

Dword grid_add_row(ItemGrid *grid) {
    auto *row =
        static_cast<Container *>(allocate_flags(sizeof(Container), grid->options, "GContainer"));
    if (row)
        container_construct(row);
    for (Dword column = 0; column < grid->columns.count; ++column)
        container_append(row, nullptr, nullptr);
    return container_append(&grid->rows, row, dispose_object);
}

Dword grid_insert_row(ItemGrid *grid, Dword index) {
    auto *row =
        static_cast<Container *>(allocate_flags(sizeof(Container), grid->options, "GContainer"));
    if (!row)
        return 1;
    container_construct(row);
    for (Dword column = 0; column < grid->columns.count; ++column)
        container_append(row, nullptr, nullptr);
    return container_insert(&grid->rows, row, index, dispose_object);
}

Dword grid_remove_row(ItemGrid *grid, Dword row) {
    return container_remove_at(&grid->rows, row);
}

void grid_reorder_rows(ItemGrid *grid, int first, int second) {
    container_move(&grid->rows, Dword(first), Dword(second));
}

void grid_set_selected_cell(ListItem *item, Dword selected) {
    item->flags = selected ? item->flags | 1 : item->flags & ~Dword(1);
}

static void destroy_list_item(ListItem *item) {
    string_destruct(&item->tooltip);
    string_destruct(&item->text);
    item->methods = reinterpret_cast<const Method *>(gui::base_methods);
}

static ListItem *__fastcall delete_list_item(ListItem *item, void *, Dword flags) {
    destroy_list_item(item);
    if (flags & 1)
        release(item);
    return item;
}
const Method list_item_methods[] = {reinterpret_cast<Method>(delete_list_item)};
} // namespace nfs::gui::widgets::family
