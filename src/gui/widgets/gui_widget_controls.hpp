#pragma once
#include "gui_widget_items.hpp"
#include "gui_widget_shared_methods.hpp"
#include "gui_widget_sequence.hpp"
namespace nfs::gui::widgets::family {
struct Button {
    GeneralWidget base;
    Dword up, down, focus, focus_down, disabled, text, icon, disabled_down;
    Dword style, checked, radio_group, last_state;
};
static_assert(sizeof(Button) == 312 && offsetof(Button, style) == 296);
Dword named_child_id(Widget *, const char *);
Widget *named_sibling(Widget *, const String *);
Widget *sibling_by_id(Widget *, Dword);
void set_widget_resource(Widget *, const String *);
void set_z_order(Widget *, Dword);
Dword destroy_widget_tree(Widget *);
int selected_list_row(ListItems *);
int next_list_row(ListItems *, int);
int previous_list_row(ListItems *, int);
void move_to_first_draw_position(Widget *);
void set_spin_button_value(Widget *, int);
} // namespace nfs::gui::widgets::family
namespace nfs::gui::widgets {
Dword initialize_tree(Widget *);
Rectangle *client_bounds(Widget *, Rectangle *);
void *create_text(void *);
void *create_image_box(void *);
} // namespace nfs::gui::widgets
namespace nfs::application::pending {
void *find_child(void *, const char *);
void set_widget_visible(void *, kernel::Dword);
kernel::Dword load_widget_children(void *, kernel::Dword);
} // namespace nfs::application::pending
