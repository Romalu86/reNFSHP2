#include "gui_widget_family.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
namespace nfs::gui::widgets::family {
using game::runtime::at;
using game::runtime::bytes;
// Complete saved leaf body.
void __fastcall edit_text_item_operation_50(Widget *widget, void *, Dword) {
}
// Complete saved leaf body.
void __fastcall edit_text_item_operation_51(Widget *widget, void *, Dword) {
}
// Complete saved leaf body.
Dword __fastcall edit_text_item_operation_53(Widget *widget, void *, Dword) {
    return 0;
}
// Complete saved leaf body.
void __fastcall tool_tip_draw(Widget *widget, void *, Dword) {
}
// Complete saved leaf body.
Dword __fastcall static_image_drawing_state(Widget *widget, void *, Dword) {
    return 0;
}
// Complete saved leaf body.
Dword __fastcall static_image_resource(Widget *widget, void *) {
    return reinterpret_cast<Dword>(bytes(widget) + 272);
}
// Complete saved leaf body.
Dword __fastcall animation_image_resource(Widget *widget, void *) {
    return reinterpret_cast<Dword>(bytes(widget) + 344);
}
// Complete saved leaf body.
Dword __fastcall animation_sequence(Widget *widget, void *) {
    return reinterpret_cast<Dword>(bytes(widget) + 328);
}
// Complete saved leaf body.
void __fastcall image_list_item_operation_47(Widget *widget, void *) {
}
// Complete saved leaf body.
void __fastcall image_list_item_operation_48(Widget *widget, void *) {
}
// Complete saved leaf body.
void __fastcall button_item_operation_50(Widget *widget, void *, Dword) {
}
// Complete saved leaf body.
void __fastcall list_box_item_operation_57(Widget *widget, void *) {
}
// Complete saved leaf body.
void __fastcall combo_box_item_operation_55(Widget *widget, void *) {
}
// Complete saved leaf body.
void __fastcall combo_box_item_operation_56(Widget *widget, void *, Dword) {
}
// Complete saved leaf body.
void __fastcall tree_item_operation_49(Widget *widget, void *) {
}
// Complete saved leaf body.
Dword __fastcall scroll_bar_pointer_idle(Widget *widget, void *, Dword, Dword) {
    return 1;
}
// Complete saved leaf body.
Dword __fastcall scroll_bar_pointer_move(Widget *widget, void *, Dword, Dword) {
    return 1;
}
// Complete saved leaf body.
Dword __fastcall scroll_bar_pointer_press(Widget *widget, void *, Dword, Dword) {
    return 1;
}
// Complete saved leaf body.
Dword __fastcall scroll_bar_pointer_release(Widget *widget, void *, Dword, Dword) {
    return 1;
}
// Complete saved leaf body.
void __fastcall slider_bar_item_operation_47(Widget *widget, void *) {
}
} // namespace nfs::gui::widgets::family
