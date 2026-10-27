#pragma once
#include "gui_widget_interaction.hpp"
#include "gui_widget_text.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
namespace nfs::gui::widgets::family {
struct MenuItem {
    const Method *methods;
    String text;
    Dword tag, image, flags, command, submenu, value;
    Rectangle bounds;
    String tooltip;
};
struct Menu {
    GeneralWidget base;
    int line_spacing, left_margin, right_margin;
    Dword style;
    int minimum_width, overlap;
    float delay;
    Container items;
    int selected;
    Dword value;
    ImageResource arrow;
    Dword state;
    int delay_ticks;
};
static_assert(sizeof(MenuItem) == 88 && offsetof(MenuItem, bounds) == 48);
static_assert(sizeof(Menu) == 364 && offsetof(Menu, items) == 292);
inline Menu *menu(Widget *widget) {
    return reinterpret_cast<Menu *>(widget);
}
inline MenuItem *menu_item(Menu *owner, Dword index) {
    return static_cast<MenuItem *>(events::container_value(&owner->items, index));
}
inline Widget *menu_widget(Menu *owner) {
    return &owner->base.base;
}
inline Widget *registered_widget(Dword id) {
    return static_cast<Widget *>(events::find_widget(context_state, id));
}
inline bool is_menu(Widget *widget) {
    return widget &&
           game::runtime::invoke<Dword>(widget, 2, reinterpret_cast<const void *>(menu_class_key));
}
MenuItem *menu_command(Menu *, Dword);
void measure_menu(Menu *);
void close_menu(Menu *);
void show_menu(Menu *);
Dword open_submenu(Menu *, Dword);
void update_menu_pointer(Menu *, Dword);
Dword menu_row_at_pointer(Menu *);
Dword inherited_tooltip_owner(Widget *);
void remove_child_indices(Widget *, Widget *);
Dword reload_widget_children(Widget *, const char *);
} // namespace nfs::gui::widgets::family
