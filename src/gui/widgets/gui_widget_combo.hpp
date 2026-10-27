#pragma once
#include "gui_widget_edit.hpp"
#include "gui_widget_list.hpp"
namespace nfs::gui::widgets::family {
Widget *combo_list(Widget *);
void erase_combo_row(Widget *, int);
void clear_combo_rows(Widget *);
int append_combo_item(Widget *, const String *, Dword, Dword);
int find_combo_value(Widget *, Dword);
Dword enable_combo_row(Widget *, int, Dword);
Dword select_combo_row(Widget *, int);
int combo_row_count(Widget *);
Dword select_combo_text(Widget *, const String *);
ListItem *combo_item(Widget *, int);
ListItem *construct_text_list_item(ListItem *, const String *);
Dword set_list_row_disabled(ListItems *, Dword, Dword);
} // namespace nfs::gui::widgets::family
