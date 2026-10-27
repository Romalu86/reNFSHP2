#pragma once
#include "gui_widget_interaction.hpp"
#include "gui_widget_text.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
namespace nfs::gui {
void string_append_wide(String *, const wchar_t *);
bool string_equal_prefix_folded(const String *, const String *, int);
} // namespace nfs::gui
namespace nfs::gui::widgets::family {
struct ListBox {
    GeneralWidget base;
    ListItems items;
    int row_height, anchor, cursor;
    Dword style, fitted_rows;
    String search;
    Dword search_time, columns, checks, unchecks, headers;
};
static_assert(sizeof(ListBox) == 368 && offsetof(ListBox, style) == 320);
inline ListBox *list_box(Widget *widget) {
    return reinterpret_cast<ListBox *>(widget);
}
int list_row_height(Widget *, int);
int first_visible_list_row(Widget *);
int visible_list_rows(Widget *);
int last_visible_list_row(Widget *, int);
void ensure_list_box_row_visible(Widget *, int);
int list_pointer_row(Widget *);
int list_pointer_check_column(Widget *);
int find_list_text(Widget *, const String *, Dword);
Dword list_row_disabled(ListItems *, Dword);
Dword list_row_selected(ListItems *, Dword);
Dword inherited_tooltip_owner(Widget *);
void configure_scroll_bar_range(Widget *, int, int, float);
} // namespace nfs::gui::widgets::family
