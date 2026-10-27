#pragma once
#include "gui_widget_settings.hpp"
namespace nfs::gui::widgets {
void __fastcall set_text(Widget *widget, void *, const String *text);
const String *__fastcall text(Widget *widget, void *);
void __fastcall reserved_four_arguments(Widget *, void *, Dword, Dword, Dword, Dword);
void __fastcall reserved_two_arguments(Widget *, void *, Dword, Dword);
Dword __fastcall apply_drawing_state(Widget *widget, void *, engine::Engine *engine);
void __fastcall prepare_draw(Widget *widget, void *, engine::Engine *engine);
void __fastcall push_draw_bounds(Widget *widget, void *, engine::Engine *engine);
void __fastcall draw_content(Widget *, void *, engine::Engine *);
void __fastcall pop_draw_bounds(Widget *widget, void *, engine::Engine *engine);
void __fastcall reserved_no_arguments(Widget *, void *);
void __fastcall reserved_render_argument(Widget *, void *, Dword);
void __fastcall reserved_style_argument(Widget *, void *, Dword);
void __fastcall clear_layout_request(Widget *widget, void *);
void __fastcall reserved_layout_argument(Widget *, void *, Dword);
void __fastcall notify_parent_enter(Widget *widget, void *, Dword);
void __fastcall notify_parent_leave(Widget *widget, void *, Dword);
void __fastcall reserved_parent_argument(Widget *, void *, Dword);
void __fastcall reserved_child_argument(Widget *, void *, Dword);
Dword __fastcall set_modal_state(Widget *widget, void *, Dword value, Dword);
Dword __fastcall reserved_result(Widget *, void *, Dword, Dword);
Dword __fastcall style_result_34(Widget *widget, void *, Dword, Dword);
Dword __fastcall style_result_35(Widget *widget, void *, Dword, Dword);
Dword __fastcall style_result_37(Widget *widget, void *, Dword, Dword);
Dword __fastcall style_result_38(Widget *widget, void *, Dword, Dword);
Dword __fastcall capture_style(Widget *widget, void *, Dword, Dword);
void __fastcall accept_general(Widget *, void *);
void __fastcall cancel_general(Widget *, void *);
void __fastcall notify_scroll(Widget *, void *, Widget *);
Dword __fastcall process_general_command(Widget *, void *, Dword, Dword);
Dword __fastcall process_general_notification(Widget *, void *, Dword, Dword);
const char *__cdecl base_class_name();
const char *__cdecl general_class_name();
namespace pending {
void __fastcall layout(Widget *widget, void *, Dword flags);
Dword __fastcall hit_test(Widget *widget, void *);
Dword __fastcall navigate(Widget *widget, void *, Dword event, Dword first, Dword second);
Dword __fastcall enter_modal(Widget *widget, void *);
void __fastcall leave_modal(Widget *widget, void *);
Dword __fastcall update_active_style(Widget *widget, void *, Dword, Dword);
Dword __fastcall update_focus_style(Widget *widget, void *, Dword, Dword suppressed);
Dword __fastcall general_event(GeneralWidget *widget, void *, Dword event, Dword first,
                              Dword second);
void __fastcall request_general_layout(GeneralWidget *widget, void *);
Dword __fastcall general_key(GeneralWidget *widget, void *, Dword key, Dword modifiers);
Dword __fastcall general_button(GeneralWidget *widget, void *, Dword button, Dword);
} // namespace pending
} // namespace nfs::gui::widgets

// Inherited native method entries referenced by fixed method tables.
namespace nfs::application::frontend {
struct FlashText;
struct RolloverButton;
void __fastcall activate_rollover(RolloverButton *widget, void *);
void __fastcall draw_flash_text(FlashText *widget, void *, gui::engine::Engine *render);
void __fastcall draw_simulation(::nfs::gui::widgets::Widget *, void *, void *engine);
void __fastcall flash_text_timer(FlashText *widget, void *, ::nfs::gui::Dword timer);
void __fastcall initialize_simulation(::nfs::gui::widgets::Widget *screen, void *);
void __fastcall release_simulation(::nfs::gui::widgets::Widget *screen, void *);
::nfs::gui::Dword __fastcall rollover_pointer_move(RolloverButton *widget, void *, ::nfs::gui::Dword first, ::nfs::gui::Dword second);
::nfs::gui::Dword __fastcall rollover_state(RolloverButton *widget, void *);
::nfs::gui::Dword __fastcall simulation_event(::nfs::gui::widgets::Widget *screen, void *, ::nfs::gui::Dword event, ::nfs::gui::Dword first, ::nfs::gui::Dword second);
::nfs::gui::Dword __fastcall simulation_key_down(::nfs::gui::widgets::Widget *screen, void *, ::nfs::gui::Dword key, ::nfs::gui::Dword modifiers);
::nfs::gui::Dword __fastcall simulation_key_up(::nfs::gui::widgets::Widget *screen, void *, ::nfs::gui::Dword key, ::nfs::gui::Dword modifiers);
void __fastcall tick_simulation(::nfs::gui::widgets::Widget *screen, void *, ::nfs::gui::Dword);
void __fastcall update_rollover(RolloverButton *widget, void *, ::nfs::gui::Dword);
} // namespace nfs::application::frontend
namespace nfs::gui::widgets {
::nfs::gui::Dword __fastcall style_result_32(::nfs::gui::widgets::Widget *widget, void *, ::nfs::gui::Dword, ::nfs::gui::Dword);
::nfs::gui::Dword __fastcall style_result_36(::nfs::gui::widgets::Widget *widget, void *, ::nfs::gui::Dword, ::nfs::gui::Dword);
} // namespace nfs::gui::widgets
namespace nfs::gui::widgets::family {
struct ListItems;
struct ListItem;
void __fastcall append_list_cell(ListItems *grid, void *, const ListItem *item);
int __fastcall append_list_row(ListItems *grid, void *);
void __fastcall clear_list_box_items(ListItems *items, void *);
void __fastcall clear_list_box_selection(ListItems *items, void *);
::nfs::gui::Dword __fastcall clear_list_cell(ListItems *grid, void *, ::nfs::gui::Dword row, ::nfs::gui::Dword column);
void __fastcall clear_list_items(ListItems *grid, void *);
::nfs::gui::Dword __fastcall copy_list_cell(ListItems *grid, void *, const ListItem *source, int row,
                                      int column);
void __fastcall erase_list_box_row(ListItems *items, void *, ::nfs::gui::Dword row);
::nfs::gui::Dword __fastcall erase_list_row(ListItems *grid, void *, ::nfs::gui::Dword column);
::nfs::gui::Dword __fastcall insert_list_row(ListItems *grid, void *, ::nfs::gui::Dword row);
void *__fastcall list_cell(ListItems *grid, void *, ::nfs::gui::Dword row, ::nfs::gui::Dword column);
::nfs::gui::Dword __fastcall load_image_widget(::nfs::gui::widgets::Widget *widget, void *, void *configuration);
void __fastcall reorder_list_rows(ListItems *grid, void *, int first, int second);
::nfs::gui::Dword __fastcall save_image_widget(::nfs::gui::widgets::Widget *widget, void *, void *configuration);
::nfs::gui::Dword __fastcall select_list_box_item(ListItems *items, void *, ::nfs::gui::Dword row, ::nfs::gui::Dword selected);
::nfs::gui::Dword __fastcall select_list_box_range(ListItems *items, void *, ::nfs::gui::Dword first, ::nfs::gui::Dword last);
::nfs::gui::Dword __fastcall select_row(ListItems *grid, void *, ::nfs::gui::Dword row, ::nfs::gui::Dword selected);
::nfs::gui::Dword __fastcall select_row_range(ListItems *grid, void *, int first, int second);
::nfs::gui::Dword __fastcall select_single_row(ListItems *grid, void *, ::nfs::gui::Dword row, ::nfs::gui::Dword selected);
::nfs::gui::Dword __fastcall set_list_box_selection(ListItems *items, void *, ::nfs::gui::Dword row, ::nfs::gui::Dword selected);
} // namespace nfs::gui::widgets::family
