#pragma once
#include "gui_widget_items.hpp"
namespace nfs::gui::widgets::family {
ListItem *construct_text_list_item(ListItem *, const String *);
ListItem *copy_list_item(ListItem *, const ListItem *);
void sort_list_items(ListItems *, Dword,
                     int(__cdecl *)(const void *, const void *, Dword));
} // namespace nfs::gui::widgets::family
