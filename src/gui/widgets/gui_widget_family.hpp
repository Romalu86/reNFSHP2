#pragma once
#include "gui_widgets.hpp"
namespace nfs::gui::widgets::family {
struct TreeNode;
extern const Method text_methods[48];
void text_destroy(Widget *);
extern const Method edit_text_methods[55];
void edit_text_destroy(Widget *);
extern const Method tool_tip_methods[47];
void tool_tip_destroy(Widget *);
extern const Method static_image_methods[48];
void static_image_destroy(Widget *);
extern const Method anim_image_methods[49];
void anim_image_destroy(Widget *);
extern const Method image_box_methods[47];
void image_box_destroy(Widget *);
extern const Method image_bar_methods[47];
void image_bar_destroy(Widget *);
extern const Method image_list_methods[51];
void image_list_destroy(Widget *);
extern const Method button_methods[52];
void button_destroy(Widget *);
extern const Method toggle_button_methods[52];
void toggle_button_destroy(Widget *);
extern const Method tab_button_methods[53];
void tab_button_destroy(Widget *);
extern const Method spin_button_methods[47];
void spin_button_destroy(Widget *);
extern const Method list_box_methods[58];
void list_box_destroy(Widget *);
extern const Method combo_box_methods[57];
void combo_box_destroy(Widget *);
extern const Method tree_methods[51];
void tree_destroy(Widget *);
extern const Method frame_methods[47];
void frame_destroy(Widget *);
extern const Method scroll_bar_methods[47];
void scroll_bar_destroy(Widget *);
extern const Method scroll_view_methods[47];
void scroll_view_destroy(Widget *);
extern const Method text_view_methods[47];
void text_view_destroy(Widget *);
extern const Method gripper_methods[49];
void gripper_destroy(Widget *);
extern const Method slider_bar_methods[48];
void slider_bar_destroy(Widget *);
extern const Method menu_methods[52];
void menu_destroy(Widget *);
extern const Method dialog_methods[49];
void dialog_destroy(Widget *);
extern const Method msg_box_methods[49];
void msg_box_destroy(Widget *);
extern const Method marquee_methods[47];
void marquee_destroy(Widget *);
const char *__cdecl text_class_key();
const char *__cdecl edit_text_class_key();
const char *__cdecl tool_tip_class_key();
const char *__cdecl static_image_class_key();
const char *__cdecl image_class_key();
const char *__cdecl anim_image_class_key();
const char *__cdecl image_box_class_key();
const char *__cdecl image_bar_class_key();
const char *__cdecl image_list_class_key();
const char *__cdecl button_class_key();
const char *__cdecl toggle_button_class_key();
const char *__cdecl tab_button_class_key();
const char *__cdecl spin_button_class_key();
const char *__cdecl list_box_class_key();
const char *__cdecl combo_box_class_key();
const char *__cdecl tree_class_key();
const char *__cdecl frame_class_key();
const char *__cdecl scroll_view_class_key();
const char *__cdecl text_view_class_key();
const char *__cdecl gripper_class_key();
const char *__cdecl slider_bar_class_key();
const char *__cdecl menu_class_key();
const char *__cdecl dialog_class_key();
const char *__cdecl msg_box_class_key();
const char *__cdecl marquee_class_key();
Dword __fastcall static_image_drawing_state(Widget *widget, void *, Dword);
Dword __fastcall static_image_resource(Widget *widget, void *);
void __fastcall image_list_item_operation_48(Widget *widget, void *);
Dword __fastcall scroll_bar_load(Widget *, void *, Dword);
Dword __fastcall scroll_bar_save(Widget *, void *, Dword);
void __fastcall scroll_bar_initialize(Widget *widget, void *);
void __fastcall scroll_bar_configure_geometry(Widget *widget, void *, Dword sender_address,
                                              Dword event, Dword state, Dword modifiers);
void __fastcall scroll_bar_prepare_draw(Widget *widget, void *, Dword render_address);
void __fastcall slider_bar_prepare_draw(Widget *widget, void *, Dword render_address);
void __fastcall slider_bar_initialize(Widget *widget, void *);
void __fastcall slider_bar_item_operation_47(Widget *widget, void *);
Dword __fastcall slider_bar_pointer_release(Widget *, void *, Dword, Dword);
void __fastcall slider_bar_parent_enter(Widget *, void *, Dword);
void __fastcall slider_bar_pointer_move(Widget *widget, void *, Dword first,
                                        Dword modifiers);
Dword __fastcall slider_bar_pointer_press(Widget *widget, void *, Dword button,
                                         Dword modifiers);
Dword __fastcall slider_bar_event(Widget *, void *, Dword, Dword, Dword);
void __fastcall slider_bar_configure_geometry(Widget *widget, void *, Dword sender_address,
                                              Dword event, Dword state, Dword);
Dword __fastcall slider_bar_load(Widget *, void *, Dword);
Dword __fastcall slider_bar_save(Widget *, void *, Dword);
Dword __fastcall marquee_load(Widget *, void *, Dword);
Dword __fastcall marquee_save(Widget *, void *, Dword);
Dword __fastcall marquee_drawing_state(Widget *widget, void *, Dword render_address);
void __fastcall marquee_draw(Widget *widget, void *, Dword render_address);
void __fastcall marquee_parent_leave(Widget *, void *, Dword);
void __fastcall marquee_parent_enter(Widget *, void *, Dword);
Dword __fastcall marquee_key_down(Widget *widget, void *, Dword key, Dword modifiers);
Dword __fastcall marquee_key_up(Widget *widget, void *, Dword key, Dword modifiers);
Dword __fastcall marquee_pointer_release(Widget *, void *, Dword, Dword);
Dword __fastcall marquee_pointer_press(Widget *widget, void *, Dword button,
                                      Dword modifiers);
void __fastcall marquee_pointer_idle(Widget *widget, void *, Dword first,
                                     Dword modifiers);
Dword __fastcall marquee_pointer_move(Widget *widget, void *, Dword delta_address,
                                     Dword modifiers);
void __fastcall frame_initialize(Widget *widget, void *);
Dword __fastcall frame_drawing_state(Widget *widget, void *, Dword render_address);
Dword __fastcall frame_pointer_move(Widget *widget, void *, Dword first, Dword modifiers);
Dword __fastcall frame_pointer_release(Widget *widget, void *, Dword button,
                                      Dword modifiers);
Dword __fastcall frame_pointer_idle(Widget *, void *, Dword, Dword);
Dword __fastcall frame_hit_test(Widget *widget, void *);
Dword __fastcall frame_pointer_press(Widget *widget, void *, Dword button,
                                    Dword modifiers);
void __fastcall frame_prepare_draw(Widget *widget, void *, Dword render_address);
Dword __fastcall frame_load(Widget *, void *, Dword);
Dword __fastcall frame_save(Widget *, void *, Dword);
Dword __fastcall scroll_view_load(Widget *, void *, Dword);
Dword __fastcall scroll_view_save(Widget *, void *, Dword);
void __fastcall scroll_view_prepare_draw(Widget *widget, void *, Dword render_address);
void __fastcall scroll_view_scrollbar(Widget *widget, void *, Dword bar_address);
Dword __fastcall scroll_view_scroll_command(Widget *widget, void *, Dword command,
                                           Dword position);
Dword __fastcall scroll_view_key_down(Widget *widget, void *, Dword key, Dword modifiers);
Dword __fastcall scroll_view_scroll_notify(Widget *widget, void *, Dword command,
                                          Dword position);
Dword __fastcall tool_tip_load(Widget *, void *, Dword);
Dword __fastcall tool_tip_save(Widget *, void *, Dword);
void __fastcall tool_tip_prepare_draw(Widget *, void *, Dword);
void __fastcall tool_tip_initialize(Widget *widget, void *);
Dword __fastcall tool_tip_event(Widget *widget, void *, Dword event, Dword first,
                               Dword second);
void __fastcall tool_tip_tick(Widget *widget, void *, Dword ticks);
void __fastcall tool_tip_draw(Widget *widget, void *, Dword);
void __fastcall text_view_set_text(Widget *widget, void *, const String *text);
const String *__fastcall text_view_text(Widget *widget, void *);
void __fastcall text_view_initialize(Widget *widget, void *);
Dword __fastcall tool_tip_is_name(Widget *, void *, const char *);
Dword __fastcall tool_tip_is_type(Widget *, void *, const void *);
const char *__fastcall tool_tip_name(Widget *, void *);
Dword __fastcall scroll_bar_is_name(Widget *, void *, const char *);
Dword __fastcall scroll_bar_is_type(Widget *, void *, const void *);
const char *__fastcall scroll_bar_name(Widget *, void *);
Dword __fastcall scroll_bar_pointer_press(Widget *widget, void *, Dword, Dword);
Dword __fastcall scroll_bar_pointer_move(Widget *widget, void *, Dword, Dword);
Dword __fastcall scroll_bar_pointer_release(Widget *widget, void *, Dword, Dword);
Dword __fastcall scroll_bar_pointer_idle(Widget *widget, void *, Dword, Dword);
Widget *__fastcall scroll_bar_delete(Widget *, void *, Dword);
Dword __fastcall slider_bar_is_name(Widget *, void *, const char *);
Dword __fastcall slider_bar_is_type(Widget *, void *, const void *);
const char *__fastcall slider_bar_name(Widget *, void *);
Widget *__fastcall slider_bar_delete(Widget *, void *, Dword);
Dword __fastcall marquee_is_name(Widget *, void *, const char *);
Dword __fastcall marquee_is_type(Widget *, void *, const void *);
const char *__fastcall marquee_name(Widget *, void *);
Widget *__fastcall marquee_delete(Widget *, void *, Dword);
Dword __fastcall frame_is_name(Widget *, void *, const char *);
Dword __fastcall frame_is_type(Widget *, void *, const void *);
const char *__fastcall frame_name(Widget *, void *);
Widget *__fastcall frame_delete(Widget *, void *, Dword);
Dword __fastcall scroll_view_is_name(Widget *, void *, const char *);
Dword __fastcall scroll_view_is_type(Widget *, void *, const void *);
const char *__fastcall scroll_view_name(Widget *, void *);
Widget *__fastcall scroll_view_delete(Widget *, void *, Dword);
Widget *__fastcall tool_tip_delete(Widget *, void *, Dword);
Dword __fastcall text_view_is_name(Widget *, void *, const char *);
Dword __fastcall text_view_is_type(Widget *, void *, const void *);
const char *__fastcall text_view_name(Widget *, void *);
Widget *__fastcall text_view_delete(Widget *, void *, Dword);
void __fastcall static_image_push_bounds(Widget *, void *, Dword);
void __fastcall image_box_draw(Widget *widget, void *, Dword render_address);
Dword __fastcall image_box_load(Widget *, void *, Dword);
Dword __fastcall image_box_save(Widget *, void *, Dword);
Dword __fastcall static_image_event(Widget *, void *, Dword, Dword, Dword);
void __fastcall static_image_draw(Widget *, void *, Dword);
void __fastcall static_image_layout(Widget *, void *, Dword);
Dword __fastcall static_image_load(Widget *, void *, Dword);
Dword __fastcall static_image_save(Widget *, void *, Dword);
Dword __fastcall anim_image_load(Widget *, void *, Dword);
Dword __fastcall anim_image_save(Widget *, void *, Dword);
Dword __fastcall anim_image_event(Widget *, void *, Dword, Dword, Dword);
void __fastcall anim_image_animate(Widget *, void *, Dword);
void __fastcall image_bar_initialize(Widget *widget, void *);
void __fastcall image_bar_prepare_draw(Widget *widget, void *, Dword render_address);
Dword __fastcall image_bar_load(Widget *, void *, Dword);
Dword __fastcall image_bar_save(Widget *, void *, Dword);
Dword __fastcall gripper_load(Widget *, void *, Dword);
Dword __fastcall gripper_save(Widget *, void *, Dword);
Dword __fastcall gripper_drawing_state(Widget *, void *, Dword);
Dword __fastcall gripper_pointer_move(Widget *, void *, Dword, Dword);
Dword __fastcall gripper_pointer_press(Widget *, void *, Dword, Dword);
Dword __fastcall gripper_pointer_release(Widget *, void *, Dword, Dword);
Dword __fastcall image_box_is_name(Widget *, void *, const char *);
Dword __fastcall image_box_is_type(Widget *, void *, const void *);
const char *__fastcall image_box_name(Widget *, void *);
Widget *__fastcall image_box_delete(Widget *, void *, Dword);
Dword __fastcall static_image_is_name(Widget *, void *, const char *);
Dword __fastcall static_image_is_type(Widget *, void *, const void *);
const char *__fastcall static_image_name(Widget *, void *);
Widget *__fastcall static_image_delete(Widget *, void *, Dword);
Dword __fastcall anim_image_is_name(Widget *, void *, const char *);
Dword __fastcall anim_image_is_type(Widget *, void *, const void *);
const char *__fastcall anim_image_name(Widget *, void *);
Dword __fastcall animation_sequence(Widget *widget, void *);
Dword __fastcall animation_image_resource(Widget *widget, void *);
Widget *__fastcall anim_image_delete(Widget *, void *, Dword);
Dword __fastcall image_bar_is_name(Widget *, void *, const char *);
Dword __fastcall image_bar_is_type(Widget *, void *, const void *);
const char *__fastcall image_bar_name(Widget *, void *);
Widget *__fastcall image_bar_delete(Widget *, void *, Dword);
Dword __fastcall gripper_is_name(Widget *, void *, const char *);
Dword __fastcall gripper_is_type(Widget *, void *, const void *);
const char *__fastcall gripper_name(Widget *, void *);
Widget *__fastcall gripper_delete(Widget *, void *, Dword);
Dword __fastcall dialog_pointer_press(Widget *, void *, Dword, Dword);
Dword __fastcall dialog_parent_leave(Widget *, void *, Dword);
Dword __fastcall dialog_load(Widget *, void *, Dword);
Dword __fastcall dialog_save(Widget *, void *, Dword);
Dword __fastcall show_dialog(Widget *, void *);
void __fastcall dialog_tick(Widget *, void *, Dword);
void __fastcall hide_dialog(Widget *, void *);
Dword __fastcall dialog_enter_modal(Widget *, void *);
void __fastcall dialog_leave_modal(Widget *, void *);
void __fastcall dialog_accept(Widget *, void *);
void __fastcall dialog_cancel(Widget *, void *);
void __fastcall dialog_configure_geometry(Widget *, void *, Dword, Dword, Dword, Dword);
void __fastcall dialog_initialize(Widget *, void *);
Dword __fastcall dialog_key_down(Widget *, void *, Dword, Dword);
Dword __fastcall dialog_event(Widget *, void *, Dword, Dword, Dword);
Dword __fastcall dialog_button_down(Widget *, void *, Dword, Dword);
void __fastcall msg_box_initialize(Widget *, void *);
Dword __fastcall dialog_is_name(Widget *, void *, const char *);
Dword __fastcall dialog_is_type(Widget *, void *, const void *);
const char *__fastcall dialog_name(Widget *, void *);
Widget *__fastcall dialog_delete(Widget *, void *, Dword);
Dword __fastcall msg_box_is_name(Widget *, void *, const char *);
Dword __fastcall msg_box_is_type(Widget *, void *, const void *);
const char *__fastcall msg_box_name(Widget *, void *);
Widget *__fastcall msg_box_delete(Widget *, void *, Dword);
void __fastcall image_list_item_operation_47(Widget *widget, void *);
void __fastcall image_list_pointer_idle(Widget *widget, void *, Dword first,
                                        Dword modifiers);
Dword __fastcall image_list_key_down(Widget *widget, void *, Dword key, Dword modifiers);
void __fastcall image_list_animate(Widget *widget, void *, Dword timer);
Dword __fastcall image_list_event(Widget *, void *, Dword, Dword, Dword);
Dword __fastcall image_list_load(Widget *widget, void *, Dword address);
Dword __fastcall image_list_save(Widget *, void *, Dword);
Dword __fastcall image_list_cell_colour(Widget *widget, void *, engine::Engine *render,
                                       Dword id);
void __fastcall draw_image_list_cell(Widget *, void *, engine::Engine *render, Dword id,
                                     const Rectangle *bounds, const Dword *colour);
void __fastcall image_list_draw(Widget *widget, void *, Dword render_address);
Dword __fastcall image_list_pointer_press(Widget *, void *, Dword, Dword);
Dword __fastcall image_list_pointer_release(Widget *, void *, Dword, Dword);
void __fastcall image_list_initialize(Widget *widget, void *);
Dword __fastcall image_list_is_name(Widget *, void *, const char *);
Dword __fastcall image_list_is_type(Widget *, void *, const void *);
const char *__fastcall image_list_name(Widget *, void *);
Widget *__fastcall image_list_delete(Widget *, void *, Dword);
void __fastcall button_set_text(Widget *widget, void *, Dword text_address);
Dword __fastcall button_visual_state(Widget *widget, void *);
Dword __fastcall button_button_down(Widget *widget, void *, Dword button,
                                   Dword modifiers);
Dword __fastcall button_key_down(Widget *, void *, Dword, Dword);
void __fastcall button_parent_enter(Widget *widget, void *, Dword parent);
void __fastcall activate_button(Widget *widget, void *);
void __fastcall button_item_operation_50(Widget *widget, void *, Dword);
Dword __fastcall button_event(Widget *widget, void *, Dword event, Dword first,
                             Dword second);
void __fastcall button_initialize(Widget *widget, void *);
void __fastcall button_set_checked_event(Widget *widget, void *, Dword checked);
void __fastcall set_button_checked(Widget *widget, void *, Dword checked);
Dword __fastcall button_pointer_press(Widget *widget, void *, Dword button,
                                     Dword modifiers);
Dword __fastcall button_pointer_release(Widget *, void *, Dword, Dword);
Dword __fastcall button_load(Widget *, void *, Dword);
Dword __fastcall button_save(Widget *, void *, Dword);
void __fastcall toggle_button_initialize(Widget *widget, void *);
Dword __fastcall toggle_button_button_down(Widget *widget, void *, Dword button,
                                          Dword modifiers);
Dword __fastcall toggle_button_key_down(Widget *widget, void *, Dword key,
                                       Dword modifiers);
void __fastcall toggle_button_draw(Widget *widget, void *, Dword render_address);
void __fastcall advance_toggle_button(Widget *widget, void *);
Dword __fastcall tab_button_load(Widget *widget, void *, Dword address);
Dword __fastcall tab_button_save(Widget *widget, void *, Dword address);
void __fastcall set_tab_view(Widget *widget, void *, Dword name_address);
void __fastcall tab_button_initialize(Widget *widget, void *);
void __fastcall tab_button_selection_changed(Widget *widget, void *, Dword selected);
Dword __fastcall spin_button_load(Widget *widget, void *, Dword address);
Dword __fastcall spin_button_save(Widget *widget, void *, Dword address);
void __fastcall spin_button_configure_geometry(Widget *widget, void *, Dword sender_address,
                                               Dword event, Dword state, Dword);
void __fastcall spin_button_tick(Widget *widget, void *, Dword ticks);
void __fastcall spin_button_initialize(Widget *widget, void *);
Dword __fastcall button_is_name(Widget *, void *, const char *);
Dword __fastcall button_is_type(Widget *, void *, const void *);
const char *__fastcall button_name(Widget *, void *);
Widget *__fastcall button_delete(Widget *, void *, Dword);
Dword __fastcall toggle_button_is_name(Widget *, void *, const char *);
Dword __fastcall toggle_button_is_type(Widget *, void *, const void *);
const char *__fastcall toggle_button_name(Widget *, void *);
Widget *__fastcall toggle_button_delete(Widget *, void *, Dword);
Dword __fastcall tab_button_is_name(Widget *, void *, const char *);
Dword __fastcall tab_button_is_type(Widget *, void *, const void *);
const char *__fastcall tab_button_name(Widget *, void *);
Widget *__fastcall tab_button_delete(Widget *, void *, Dword);
Dword __fastcall spin_button_is_name(Widget *, void *, const char *);
Dword __fastcall spin_button_is_type(Widget *, void *, const void *);
const char *__fastcall spin_button_name(Widget *, void *);
Widget *__fastcall spin_button_delete(Widget *, void *, Dword);
Dword __fastcall text_load(Widget *, void *, Dword);
Dword __fastcall text_save(Widget *, void *, Dword);
void __fastcall text_layout(Widget *widget, void *, Dword flags);
void __fastcall draw_text_line(Widget *, void *, engine::Engine *render, const String *text,
                               const Rectangle *bounds, const Dword *colour, Dword);
void __fastcall text_draw(Widget *widget, void *, Dword render_address);
Dword __fastcall edit_text_drawing_state(Widget *widget, void *, Dword render_address);
void __fastcall draw_edit_caret(Widget *, void *, engine::Engine *render, Dword point_methods,
                                Dword x, Dword y);
void __fastcall draw_edit_line(Widget *widget, void *, engine::Engine *render, const String *line,
                               const Rectangle *bounds, const Dword *colour,
                               Dword line_start);
void __fastcall edit_text_draw(Widget *widget, void *, Dword render_address);
Dword __fastcall edit_text_pointer_release(Widget *, void *, Dword, Dword);
Dword __fastcall edit_text_pointer_move(Widget *widget, void *, Dword first,
                                       Dword modifiers);
Dword __fastcall edit_text_pointer_press(Widget *widget, void *, Dword button,
                                        Dword modifiers);
void __fastcall edit_text_prepare_draw(Widget *widget, void *, Dword render_address);
void __fastcall edit_text_parent_leave(Widget *, void *, Dword);
void __fastcall edit_text_parent_enter(Widget *, void *, Dword);
Dword __fastcall edit_text_load(Widget *, void *, Dword);
Dword __fastcall edit_text_save(Widget *, void *, Dword);
void __fastcall edit_text_cancel(Widget *, void *);
void __fastcall validate_edit_text(Widget *widget, void *, Dword);
void __fastcall edit_text_accept(Widget *widget, void *);
void __fastcall begin_edit_session(Widget *widget, void *);
void __fastcall edit_text_item_operation_50(Widget *widget, void *, Dword);
void __fastcall edit_text_item_operation_51(Widget *widget, void *, Dword);
Dword __fastcall reject_edit_character(Widget *widget, void *, Dword character,
                                      Dword position);
Dword __fastcall edit_text_item_operation_53(Widget *widget, void *, Dword);
Dword __fastcall edit_text_event(Widget *widget, void *, Dword event, Dword first,
                                Dword second);
Dword __fastcall edit_text_character(Widget *widget, void *, Dword character,
                                    Dword modifiers);
Dword __fastcall edit_text_button_down(Widget *widget, void *, Dword button,
                                      Dword modifiers);
Dword __fastcall edit_text_key_down(Widget *widget, void *, Dword key, Dword modifiers);
Dword __fastcall text_is_name(Widget *, void *, const char *);
Dword __fastcall text_is_type(Widget *, void *, const void *);
const char *__fastcall text_name(Widget *, void *);
Widget *__fastcall text_delete(Widget *, void *, Dword);
Dword __fastcall edit_text_is_name(Widget *, void *, const char *);
Dword __fastcall edit_text_is_type(Widget *, void *, const void *);
const char *__fastcall edit_text_name(Widget *, void *);
Widget *__fastcall edit_text_delete(Widget *, void *, Dword);
Dword __fastcall list_box_load(Widget *, void *, Dword);
Dword __fastcall list_box_save(Widget *, void *, Dword);
Dword __fastcall list_box_header(Widget *, void *, Dword);
Dword __fastcall list_box_column(Widget *, void *, Dword);
Dword __fastcall list_box_check_image(Widget *, void *, Dword);
Dword __fastcall list_box_uncheck_image(Widget *, void *, Dword);
void __fastcall list_box_initialize(Widget *widget, void *);
void __fastcall list_box_configure_geometry(Widget *widget, void *, Dword source, Dword event,
                                            Dword state, Dword);
Dword __fastcall list_cell_colour(Widget *widget, void *, engine::Engine *render, Dword row,
                                 Dword);
void __fastcall draw_list_cell(Widget *widget, void *, engine::Engine *render, Dword row,
                               Dword column);
void __fastcall list_box_prepare_draw(Widget *widget, void *, Dword render_address);
void __fastcall draw_list_selection(Widget *, void *, engine::Engine *render,
                                    const Rectangle *bounds);
void __fastcall draw_list_column_divider(Widget *, void *, engine::Engine *render,
                                         const Point *first, const Point *second);
void __fastcall draw_list_row_divider(Widget *, void *, engine::Engine *render, const Point *first,
                                      const Point *second);
void __fastcall draw_list_header_background(Widget *widget, void *, engine::Engine *render,
                                            const Rectangle *bounds);
void __fastcall list_box_draw(Widget *widget, void *, Dword render_address);
Dword __fastcall list_box_scroll_command(Widget *widget, void *, Dword command,
                                        Dword position);
void __fastcall list_box_scrollbar(Widget *widget, void *, Dword bar_address);
void __fastcall list_box_parent_enter(Widget *, void *, Dword);
Dword __fastcall list_box_button_down(Widget *widget, void *, Dword button,
                                     Dword modifiers);
Dword __fastcall list_box_key_down(Widget *widget, void *, Dword key, Dword modifiers);
void __fastcall list_box_character(Widget *widget, void *, Dword character,
                                   Dword modifiers);
void __fastcall list_box_pointer_idle(Widget *widget, void *, Dword first, Dword second);
void __fastcall list_box_pointer_move(Widget *widget, void *, Dword first, Dword second);
Dword __fastcall list_box_pointer_press(Widget *widget, void *, Dword button,
                                       Dword modifiers);
Dword __fastcall list_box_pointer_release(Widget *widget, void *, Dword button,
                                         Dword modifiers);
Dword __fastcall list_box_event(Widget *widget, void *, Dword event, Dword first,
                               Dword second);
void __fastcall list_box_item_operation_57(Widget *widget, void *);
Dword __fastcall list_box_is_name(Widget *, void *, const char *);
Dword __fastcall list_box_is_type(Widget *, void *, const void *);
const char *__fastcall list_box_name(Widget *, void *);
Widget *__fastcall list_box_delete(Widget *, void *, Dword);
Dword __fastcall combo_box_load(Widget *, void *, Dword);
Dword __fastcall combo_box_save(Widget *, void *, Dword);
void __fastcall combo_box_initialize(Widget *widget, void *);
void __fastcall combo_box_prepare_draw(Widget *widget, void *, Dword render_address);
void __fastcall combo_box_parent_enter(Widget *widget, void *, Dword parent_address);
Dword __fastcall combo_box_event(Widget *widget, void *, Dword event, Dword first,
                                Dword second);
void __fastcall validate_combo_text(Widget *widget, void *, Dword source);
void __fastcall combo_box_cancel(Widget *widget, void *);
void __fastcall combo_box_item_operation_55(Widget *widget, void *);
void __fastcall combo_box_configure_geometry(Widget *widget, void *, Dword source_address,
                                             Dword event, Dword first, Dword second);
Dword __fastcall combo_box_key_down(Widget *widget, void *, Dword key, Dword modifiers);
Dword __fastcall combo_box_is_name(Widget *, void *, const char *);
Dword __fastcall combo_box_is_type(Widget *, void *, const void *);
const char *__fastcall combo_box_name(Widget *, void *);
void __fastcall combo_box_item_operation_56(Widget *widget, void *, Dword);
Widget *__fastcall combo_box_delete(Widget *, void *, Dword);
Dword __fastcall tree_load(Widget *, void *, Dword);
Dword __fastcall tree_save(Widget *, void *, Dword);
Dword __fastcall tree_scroll_command(Widget *widget, void *, Dword command,
                                    Dword position);
void __fastcall tree_scrollbar(Widget *widget, void *, Dword address);
void __fastcall draw_tree_selection(Widget *, void *, engine::Engine *render,
                                    const Rectangle *bounds);
void __fastcall draw_tree_expander(Widget *widget, void *, engine::Engine *render, TreeNode *node,
                                   const Point *centre);
void __fastcall draw_tree_row(Widget *widget, void *, engine::Engine *render,
                              TreeNode *node);
void __fastcall tree_item_operation_49(Widget *widget, void *);
void __fastcall tree_prepare_draw(Widget *, void *, Dword);
void __fastcall tree_initialize(Widget *widget, void *);
void __fastcall tree_draw(Widget *widget, void *, Dword render_address);
Dword __fastcall tree_event(Widget *, void *, Dword, Dword, Dword);
void __fastcall tree_accept(Widget *widget, void *);
void __fastcall tree_button_down(Widget *widget, void *, Dword button, Dword modifiers);
Dword __fastcall tree_key_down(Widget *widget, void *, Dword key, Dword modifiers);
Dword __fastcall tree_pointer_press(Widget *widget, void *, Dword button,
                                   Dword modifiers);
void __fastcall tree_parent_enter(Widget *, void *, Dword);
Dword __fastcall tree_pointer_release(Widget *widget, void *, Dword button,
                                     Dword modifiers);
Dword __fastcall tree_is_name(Widget *, void *, const char *);
Dword __fastcall tree_is_type(Widget *, void *, const void *);
const char *__fastcall tree_name(Widget *, void *);
Widget *__fastcall tree_delete(Widget *, void *, Dword);
void __fastcall menu_initialize(Widget *widget, void *);
Dword __fastcall menu_load(Widget *, void *, Dword);
Dword __fastcall menu_save(Widget *, void *, Dword);
void __fastcall copy_menu_visual_settings(Widget *widget, void *, Dword source_address);
Dword __fastcall menu_enter_modal(Widget *widget, void *);
Dword __fastcall menu_key_down(Widget *widget, void *, Dword key, Dword modifiers);
void __fastcall menu_parent_enter(Widget *widget, void *, Dword parent_address);
void __fastcall menu_accept(Widget *widget, void *);
void __fastcall menu_cancel(Widget *widget, void *);
Dword __fastcall menu_pointer_release(Widget *widget, void *, Dword, Dword);
Dword __fastcall menu_event(Widget *widget, void *, Dword event, Dword first,
                           Dword second);
void __fastcall synchronize_menu_ancestors(Widget *widget, void *, Dword,
                                           Widget *source);
Dword __fastcall menu_pointer_press(Widget *widget, void *, Dword, Dword modifiers);
Dword __fastcall menu_pointer_move(Widget *widget, void *, Dword, Dword);
Dword __fastcall menu_pointer_idle(Widget *widget, void *, Dword ticks, Dword);
Dword __fastcall menu_item_colour(Widget *widget, void *, engine::Engine *render,
                                 Dword index);
void __fastcall draw_menu_item(Widget *widget, void *, engine::Engine *render, Dword index,
                               const Rectangle *bounds, const Dword *colour);
void __fastcall menu_draw(Widget *widget, void *, Dword render_address);
void __fastcall menu_prepare_draw(Widget *widget, void *, Dword render_address);
Dword __fastcall menu_is_name(Widget *, void *, const char *);
Dword __fastcall menu_is_type(Widget *, void *, const void *);
const char *__fastcall menu_name(Widget *, void *);
Dword __fastcall menu_item_font_style(Widget *widget, void *, Dword, Dword);
Widget *__fastcall menu_delete(Widget *, void *, Dword);
} // namespace nfs::gui::widgets::family
namespace nfs::gui::widgets::pending {
void *construct_text(void *, void *);
void *construct_edit_text(void *, void *);
void *construct_tool_tip(void *, void *);
void *construct_static_image(void *, void *);
void *construct_anim_image(void *, void *);
void *construct_image_box(void *, void *);
void *construct_image_bar(void *, void *);
void *construct_image_list(void *, void *);
void *construct_button(void *, void *);
void *construct_toggle_button(void *, void *);
void *construct_tab_button(void *, void *);
void *construct_spin_button(void *, void *);
void *construct_list_box(void *, void *);
void *construct_combo_box(void *, void *);
void *construct_tree(void *, void *);
void *construct_frame(void *, void *);
void *construct_scroll_bar(void *, void *);
void *construct_scroll_view(void *, void *);
void *construct_text_view(void *, void *);
void *construct_gripper(void *, void *);
void *construct_slider_bar(void *, void *);
void *construct_menu(void *, void *);
void *construct_dialog(void *, void *);
void *construct_msg_box(void *, void *);
void *construct_marquee(void *, void *);
} // namespace nfs::gui::widgets::pending
