#pragma once
#include "gui_widget_resources.hpp"
namespace nfs::gui::widgets::family {
struct ListItem {
    const Method *methods;
    String text;
    Dword data, flags;
    String tooltip;
    Dword image;
};
static_assert(sizeof(ListItem) == 56);
extern const Method list_item_methods[1];
void __fastcall clear_list_selection(ListItems *, void *);
Dword grid_set_cell(ItemGrid *, void *, Dword, Dword, Dispose);
void *grid_cell(ItemGrid *, Dword, Dword);
void grid_add_column(ItemGrid *, void *, Dispose);
Dword grid_add_row(ItemGrid *);
Dword grid_remove_row(ItemGrid *, Dword);
Dword grid_insert_row(ItemGrid *, Dword);
void grid_reorder_rows(ItemGrid *, int, int);
void grid_set_selected_cell(ListItem *, Dword);
} // namespace nfs::gui::widgets::family
