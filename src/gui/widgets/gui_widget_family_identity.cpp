#include "gui_widget_family.hpp"
namespace nfs::gui::widgets::pending {
const char *__cdecl scroll_bar_class_name();
}
#include "gui_widget_shared_methods.hpp"
#include <cstring>
namespace nfs::gui::widgets::family {

const char *__cdecl text_class_key() {
    return "GText";
}

const char *__cdecl edit_text_class_key() {
    return "GEditText";
}

const char *__cdecl tool_tip_class_key() {
    return "GToolTip";
}

const char *__cdecl static_image_class_key() {
    return "GStaticImage";
}

const char *__cdecl image_class_key() {
    return "GImage";
}

const char *__cdecl anim_image_class_key() {
    return "GAnimImage";
}

const char *__cdecl image_box_class_key() {
    return "GImageBox";
}

const char *__cdecl image_bar_class_key() {
    return "GImageBar";
}

const char *__cdecl image_list_class_key() {
    return "GImageList";
}

const char *__cdecl button_class_key() {
    return "GButton";
}

const char *__cdecl toggle_button_class_key() {
    return "GToggleButton";
}

const char *__cdecl tab_button_class_key() {
    return "GTabButton";
}

const char *__cdecl spin_button_class_key() {
    return "GSpinButton";
}

const char *__cdecl list_box_class_key() {
    return "GListBox";
}

const char *__cdecl combo_box_class_key() {
    return "GComboBox";
}

const char *__cdecl tree_class_key() {
    return "GTree";
}

const char *__cdecl frame_class_key() {
    return "GFrame";
}

const char *__cdecl scroll_view_class_key() {
    return "GScrollView";
}

const char *__cdecl text_view_class_key() {
    return "GTextView";
}

const char *__cdecl gripper_class_key() {
    return "GGripper";
}

const char *__cdecl slider_bar_class_key() {
    return "GSliderBar";
}

const char *__cdecl menu_class_key() {
    return "GMenu";
}

const char *__cdecl dialog_class_key() {
    return "GDialog";
}

const char *__cdecl msg_box_class_key() {
    return "GMsgBox";
}

const char *__cdecl marquee_class_key() {
    return "GMarquee";
}

const char *__fastcall text_name(Widget *, void *) {
    return "GText";
}

Dword __fastcall text_is_type(Widget *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(text_class_key) ||
           type == reinterpret_cast<const void *>(general_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

Dword __fastcall text_is_name(Widget *, void *, const char *name) {
    return !std::strncmp(name, "GText", 6) || !std::strncmp(name, "GWidget", 8) ||
           !std::strncmp(name, "GUI", 4);
}

Widget *__fastcall text_delete(Widget *widget, void *, Dword flags) {
    text_destroy(widget);
    if (flags & 1)
        release(widget);
    return widget;
}

const char *__fastcall edit_text_name(Widget *, void *) {
    return "GEditText";
}

Dword __fastcall edit_text_is_type(Widget *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(edit_text_class_key) ||
           type == reinterpret_cast<const void *>(text_class_key) ||
           type == reinterpret_cast<const void *>(general_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

Dword __fastcall edit_text_is_name(Widget *, void *, const char *name) {
    return !std::strncmp(name, "GEditText", 10) || !std::strncmp(name, "GText", 6) ||
           !std::strncmp(name, "GWidget", 8) || !std::strncmp(name, "GUI", 4);
}

Widget *__fastcall edit_text_delete(Widget *widget, void *, Dword flags) {
    edit_text_destroy(widget);
    if (flags & 1)
        release(widget);
    return widget;
}

const char *__fastcall tool_tip_name(Widget *, void *) {
    return "GToolTip";
}

Dword __fastcall tool_tip_is_type(Widget *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(tool_tip_class_key) ||
           type == reinterpret_cast<const void *>(general_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

Dword __fastcall tool_tip_is_name(Widget *, void *, const char *name) {
    return !std::strncmp(name, "GToolTip", 9) || !std::strncmp(name, "GWidget", 8) ||
           !std::strncmp(name, "GUI", 4);
}

Widget *__fastcall tool_tip_delete(Widget *widget, void *, Dword flags) {
    tool_tip_destroy(widget);
    if (flags & 1)
        release(widget);
    return widget;
}

const char *__fastcall static_image_name(Widget *, void *) {
    return "GStaticImage";
}

Dword __fastcall static_image_is_type(Widget *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(static_image_class_key) ||
           type == reinterpret_cast<const void *>(image_class_key) ||
           type == reinterpret_cast<const void *>(general_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

Dword __fastcall static_image_is_name(Widget *, void *, const char *name) {
    return !std::strncmp(name, "GStaticImage", 13) || !std::strncmp(name, "GImage", 7) ||
           !std::strncmp(name, "GWidget", 8) || !std::strncmp(name, "GUI", 4);
}

Widget *__fastcall static_image_delete(Widget *widget, void *, Dword flags) {
    static_image_destroy(widget);
    if (flags & 1)
        release(widget);
    return widget;
}

const char *__fastcall anim_image_name(Widget *, void *) {
    return "GAnimImage";
}

Dword __fastcall anim_image_is_type(Widget *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(anim_image_class_key) ||
           type == reinterpret_cast<const void *>(static_image_class_key) ||
           type == reinterpret_cast<const void *>(image_class_key) ||
           type == reinterpret_cast<const void *>(general_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

Dword __fastcall anim_image_is_name(Widget *, void *, const char *name) {
    return !std::strncmp(name, "GAnimImage", 11) || !std::strncmp(name, "GStaticImage", 13) ||
           !std::strncmp(name, "GImage", 7) || !std::strncmp(name, "GWidget", 8) ||
           !std::strncmp(name, "GUI", 4);
}

Widget *__fastcall anim_image_delete(Widget *widget, void *, Dword flags) {
    anim_image_destroy(widget);
    if (flags & 1)
        release(widget);
    return widget;
}

const char *__fastcall image_box_name(Widget *, void *) {
    return "GImageBox";
}

Dword __fastcall image_box_is_type(Widget *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(image_box_class_key) ||
           type == reinterpret_cast<const void *>(image_class_key) ||
           type == reinterpret_cast<const void *>(general_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

Dword __fastcall image_box_is_name(Widget *, void *, const char *name) {
    return !std::strncmp(name, "GImageBox", 10) || !std::strncmp(name, "GImage", 7) ||
           !std::strncmp(name, "GWidget", 8) || !std::strncmp(name, "GUI", 4);
}

Widget *__fastcall image_box_delete(Widget *widget, void *, Dword flags) {
    image_box_destroy(widget);
    if (flags & 1)
        release(widget);
    return widget;
}

const char *__fastcall image_bar_name(Widget *, void *) {
    return "GImageBar";
}

Dword __fastcall image_bar_is_type(Widget *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(image_bar_class_key) ||
           type == reinterpret_cast<const void *>(image_class_key) ||
           type == reinterpret_cast<const void *>(general_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

Dword __fastcall image_bar_is_name(Widget *, void *, const char *name) {
    return !std::strncmp(name, "GImageBar", 10) || !std::strncmp(name, "GImage", 7) ||
           !std::strncmp(name, "GWidget", 8) || !std::strncmp(name, "GUI", 4);
}

Widget *__fastcall image_bar_delete(Widget *widget, void *, Dword flags) {
    image_bar_destroy(widget);
    if (flags & 1)
        release(widget);
    return widget;
}

const char *__fastcall image_list_name(Widget *, void *) {
    return "GImageList";
}

Dword __fastcall image_list_is_type(Widget *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(image_list_class_key) ||
           type == reinterpret_cast<const void *>(general_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

Dword __fastcall image_list_is_name(Widget *, void *, const char *name) {
    return !std::strncmp(name, "GImageList", 11) || !std::strncmp(name, "GWidget", 8) ||
           !std::strncmp(name, "GUI", 4);
}

Widget *__fastcall image_list_delete(Widget *widget, void *, Dword flags) {
    image_list_destroy(widget);
    if (flags & 1)
        release(widget);
    return widget;
}

const char *__fastcall button_name(Widget *, void *) {
    return "GButton";
}

Dword __fastcall button_is_type(Widget *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(button_class_key) ||
           type == reinterpret_cast<const void *>(general_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

Dword __fastcall button_is_name(Widget *, void *, const char *name) {
    return !std::strncmp(name, "GButton", 8) || !std::strncmp(name, "GWidget", 8) ||
           !std::strncmp(name, "GUI", 4);
}

Widget *__fastcall button_delete(Widget *widget, void *, Dword flags) {
    button_destroy(widget);
    if (flags & 1)
        release(widget);
    return widget;
}

const char *__fastcall toggle_button_name(Widget *, void *) {
    return "GToggleButton";
}

Dword __fastcall toggle_button_is_type(Widget *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(toggle_button_class_key) ||
           type == reinterpret_cast<const void *>(button_class_key) ||
           type == reinterpret_cast<const void *>(general_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

Dword __fastcall toggle_button_is_name(Widget *, void *, const char *name) {
    return !std::strncmp(name, "GToggleButton", 14) || !std::strncmp(name, "GButton", 8) ||
           !std::strncmp(name, "GWidget", 8) || !std::strncmp(name, "GUI", 4);
}

Widget *__fastcall toggle_button_delete(Widget *widget, void *, Dword flags) {
    toggle_button_destroy(widget);
    if (flags & 1)
        release(widget);
    return widget;
}

const char *__fastcall tab_button_name(Widget *, void *) {
    return "GTabButton";
}

Dword __fastcall tab_button_is_type(Widget *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(tab_button_class_key) ||
           type == reinterpret_cast<const void *>(button_class_key) ||
           type == reinterpret_cast<const void *>(general_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

Dword __fastcall tab_button_is_name(Widget *, void *, const char *name) {
    return !std::strncmp(name, "GTabButton", 11) || !std::strncmp(name, "GButton", 8) ||
           !std::strncmp(name, "GWidget", 8) || !std::strncmp(name, "GUI", 4);
}

Widget *__fastcall tab_button_delete(Widget *widget, void *, Dword flags) {
    tab_button_destroy(widget);
    if (flags & 1)
        release(widget);
    return widget;
}

const char *__fastcall spin_button_name(Widget *, void *) {
    return "GSpinButton";
}

Dword __fastcall spin_button_is_type(Widget *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(spin_button_class_key) ||
           type == reinterpret_cast<const void *>(general_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

Dword __fastcall spin_button_is_name(Widget *, void *, const char *name) {
    return !std::strncmp(name, "GSpinButton", 12) || !std::strncmp(name, "GWidget", 8) ||
           !std::strncmp(name, "GUI", 4);
}

Widget *__fastcall spin_button_delete(Widget *widget, void *, Dword flags) {
    spin_button_destroy(widget);
    if (flags & 1)
        release(widget);
    return widget;
}

const char *__fastcall list_box_name(Widget *, void *) {
    return "GListBox";
}

Dword __fastcall list_box_is_type(Widget *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(list_box_class_key) ||
           type == reinterpret_cast<const void *>(general_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

Dword __fastcall list_box_is_name(Widget *, void *, const char *name) {
    return !std::strncmp(name, "GListBox", 9) || !std::strncmp(name, "GWidget", 8) ||
           !std::strncmp(name, "GUI", 4);
}

Widget *__fastcall list_box_delete(Widget *widget, void *, Dword flags) {
    list_box_destroy(widget);
    if (flags & 1)
        release(widget);
    return widget;
}

const char *__fastcall combo_box_name(Widget *, void *) {
    return "GComboBox";
}

Dword __fastcall combo_box_is_type(Widget *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(combo_box_class_key) ||
           type == reinterpret_cast<const void *>(general_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

Dword __fastcall combo_box_is_name(Widget *, void *, const char *name) {
    return !std::strncmp(name, "GComboBox", 10) || !std::strncmp(name, "GWidget", 8) ||
           !std::strncmp(name, "GUI", 4);
}

Widget *__fastcall combo_box_delete(Widget *widget, void *, Dword flags) {
    combo_box_destroy(widget);
    if (flags & 1)
        release(widget);
    return widget;
}

const char *__fastcall tree_name(Widget *, void *) {
    return "GTree";
}

Dword __fastcall tree_is_type(Widget *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(tree_class_key) ||
           type == reinterpret_cast<const void *>(general_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

Dword __fastcall tree_is_name(Widget *, void *, const char *name) {
    return !std::strncmp(name, "GTree", 6) || !std::strncmp(name, "GWidget", 8) ||
           !std::strncmp(name, "GUI", 4);
}

Widget *__fastcall tree_delete(Widget *widget, void *, Dword flags) {
    tree_destroy(widget);
    if (flags & 1)
        release(widget);
    return widget;
}

const char *__fastcall frame_name(Widget *, void *) {
    return "GFrame";
}

Dword __fastcall frame_is_type(Widget *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(frame_class_key) ||
           type == reinterpret_cast<const void *>(general_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

Dword __fastcall frame_is_name(Widget *, void *, const char *name) {
    return !std::strncmp(name, "GFrame", 7) || !std::strncmp(name, "GWidget", 8) ||
           !std::strncmp(name, "GUI", 4);
}

Widget *__fastcall frame_delete(Widget *widget, void *, Dword flags) {
    frame_destroy(widget);
    if (flags & 1)
        release(widget);
    return widget;
}

const char *__fastcall scroll_bar_name(Widget *, void *) {
    return "GScrollBar";
}

Dword __fastcall scroll_bar_is_type(Widget *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(pending::scroll_bar_class_name) ||
           type == reinterpret_cast<const void *>(general_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

Dword __fastcall scroll_bar_is_name(Widget *, void *, const char *name) {
    return !std::strncmp(name, "GScrollBar", 11) || !std::strncmp(name, "GWidget", 8) ||
           !std::strncmp(name, "GUI", 4);
}

Widget *__fastcall scroll_bar_delete(Widget *widget, void *, Dword flags) {
    scroll_bar_destroy(widget);
    if (flags & 1)
        release(widget);
    return widget;
}

const char *__fastcall scroll_view_name(Widget *, void *) {
    return "GScrollView";
}

Dword __fastcall scroll_view_is_type(Widget *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(scroll_view_class_key) ||
           type == reinterpret_cast<const void *>(general_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

Dword __fastcall scroll_view_is_name(Widget *, void *, const char *name) {
    return !std::strncmp(name, "GScrollView", 12) || !std::strncmp(name, "GWidget", 8) ||
           !std::strncmp(name, "GUI", 4);
}

Widget *__fastcall scroll_view_delete(Widget *widget, void *, Dword flags) {
    scroll_view_destroy(widget);
    if (flags & 1)
        release(widget);
    return widget;
}

const char *__fastcall text_view_name(Widget *, void *) {
    return "GTextView";
}

Dword __fastcall text_view_is_type(Widget *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(text_view_class_key) ||
           type == reinterpret_cast<const void *>(scroll_view_class_key) ||
           type == reinterpret_cast<const void *>(general_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

Dword __fastcall text_view_is_name(Widget *, void *, const char *name) {
    return !std::strncmp(name, "GTextView", 10) || !std::strncmp(name, "GScrollView", 12) ||
           !std::strncmp(name, "GWidget", 8) || !std::strncmp(name, "GUI", 4);
}

Widget *__fastcall text_view_delete(Widget *widget, void *, Dword flags) {
    text_view_destroy(widget);
    if (flags & 1)
        release(widget);
    return widget;
}

const char *__fastcall gripper_name(Widget *, void *) {
    return "GGripper";
}

Dword __fastcall gripper_is_type(Widget *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(gripper_class_key) ||
           type == reinterpret_cast<const void *>(anim_image_class_key) ||
           type == reinterpret_cast<const void *>(static_image_class_key) ||
           type == reinterpret_cast<const void *>(image_class_key) ||
           type == reinterpret_cast<const void *>(general_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

Dword __fastcall gripper_is_name(Widget *, void *, const char *name) {
    return !std::strncmp(name, "GGripper", 9) || !std::strncmp(name, "GAnimImage", 11) ||
           !std::strncmp(name, "GStaticImage", 13) || !std::strncmp(name, "GImage", 7) ||
           !std::strncmp(name, "GWidget", 8) || !std::strncmp(name, "GUI", 4);
}

Widget *__fastcall gripper_delete(Widget *widget, void *, Dword flags) {
    gripper_destroy(widget);
    if (flags & 1)
        release(widget);
    return widget;
}

const char *__fastcall slider_bar_name(Widget *, void *) {
    return "GSliderBar";
}

Dword __fastcall slider_bar_is_type(Widget *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(slider_bar_class_key) ||
           type == reinterpret_cast<const void *>(general_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

Dword __fastcall slider_bar_is_name(Widget *, void *, const char *name) {
    return !std::strncmp(name, "GSliderBar", 11) || !std::strncmp(name, "GWidget", 8) ||
           !std::strncmp(name, "GUI", 4);
}

Widget *__fastcall slider_bar_delete(Widget *widget, void *, Dword flags) {
    slider_bar_destroy(widget);
    if (flags & 1)
        release(widget);
    return widget;
}

const char *__fastcall menu_name(Widget *, void *) {
    return "GMenu";
}

Dword __fastcall menu_is_type(Widget *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(menu_class_key) ||
           type == reinterpret_cast<const void *>(general_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

Dword __fastcall menu_is_name(Widget *, void *, const char *name) {
    return !std::strncmp(name, "GMenu", 6) || !std::strncmp(name, "GWidget", 8) ||
           !std::strncmp(name, "GUI", 4);
}

Widget *__fastcall menu_delete(Widget *widget, void *, Dword flags) {
    menu_destroy(widget);
    if (flags & 1)
        release(widget);
    return widget;
}

const char *__fastcall dialog_name(Widget *, void *) {
    return "GDialog";
}

Dword __fastcall dialog_is_type(Widget *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(dialog_class_key) ||
           type == reinterpret_cast<const void *>(general_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

Dword __fastcall dialog_is_name(Widget *, void *, const char *name) {
    return !std::strncmp(name, "GDialog", 8) || !std::strncmp(name, "GWidget", 8) ||
           !std::strncmp(name, "GUI", 4);
}

Widget *__fastcall dialog_delete(Widget *widget, void *, Dword flags) {
    dialog_destroy(widget);
    if (flags & 1)
        release(widget);
    return widget;
}

const char *__fastcall msg_box_name(Widget *, void *) {
    return "GMsgBox";
}

Dword __fastcall msg_box_is_type(Widget *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(msg_box_class_key) ||
           type == reinterpret_cast<const void *>(dialog_class_key) ||
           type == reinterpret_cast<const void *>(general_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

Dword __fastcall msg_box_is_name(Widget *, void *, const char *name) {
    return !std::strncmp(name, "GMsgBox", 8) || !std::strncmp(name, "GDialog", 8) ||
           !std::strncmp(name, "GWidget", 8) || !std::strncmp(name, "GUI", 4);
}

Widget *__fastcall msg_box_delete(Widget *widget, void *, Dword flags) {
    msg_box_destroy(widget);
    if (flags & 1)
        release(widget);
    return widget;
}

const char *__fastcall marquee_name(Widget *, void *) {
    return "GMarquee";
}

Dword __fastcall marquee_is_type(Widget *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(marquee_class_key) ||
           type == reinterpret_cast<const void *>(general_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

Dword __fastcall marquee_is_name(Widget *, void *, const char *name) {
    return !std::strncmp(name, "GMarquee", 9) || !std::strncmp(name, "GWidget", 8) ||
           !std::strncmp(name, "GUI", 4);
}

Widget *__fastcall marquee_delete(Widget *widget, void *, Dword flags) {
    marquee_destroy(widget);
    if (flags & 1)
        release(widget);
    return widget;
}
} // namespace nfs::gui::widgets::family
