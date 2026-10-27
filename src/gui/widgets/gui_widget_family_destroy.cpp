#include "nfs/core/lifecycle/scope_cleanup.hpp"
#include "gui_widget_resources.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
namespace nfs::gui::widgets::family {
using game::runtime::at;
using game::runtime::bytes;
void destroy_image_widget(Widget *);
template <class T> static T *member(Widget *widget, Dword offset) {
    return reinterpret_cast<T *>(bytes(widget) + offset);
}

void text_destroy(Widget *widget) {
    widget->methods = text_methods;
    destroy_general(reinterpret_cast<GeneralWidget *>(widget));
}

void edit_text_destroy(Widget *widget) {
    widget->methods = edit_text_methods;
    string_destruct(member<String>(widget, 288));
    widget->methods = text_methods;
    destroy_general(reinterpret_cast<GeneralWidget *>(widget));
}

void tool_tip_destroy(Widget *widget) {
    widget->methods = tool_tip_methods;
    if (tooltip_owner == widget->id)
        tooltip_owner = 0;
    widget->methods = general_methods;
    destroy_base(widget);
}

void static_image_destroy(Widget *widget) {
    widget->methods = static_image_methods;
    kernel::lifecycle::Cleanup base([&] { destroy_image_widget(widget); });
    destroy_image_resource(member<ImageResource>(widget, 272));
}

void anim_image_destroy(Widget *widget) {
    widget->methods = anim_image_methods;
    kernel::lifecycle::Cleanup base([&] { static_image_destroy(widget); });
    kernel::lifecycle::Cleanup sequence([&] { destroy_image_sequence(member<ImageSequence>(widget, 328)); });
    if (at<Dword>(widget, 396)) {
        remove_widget_timer(at<Dword>(widget, 396));
        at<Dword>(widget, 396) = 0;
    }
    sequence.run();
}

void image_box_destroy(Widget *widget) {
    widget->methods = image_widget_methods;
    destroy_general(reinterpret_cast<GeneralWidget *>(widget));
}

void image_bar_destroy(Widget *widget) {
    widget->methods = image_widget_methods;
    destroy_general(reinterpret_cast<GeneralWidget *>(widget));
}

void image_list_destroy(Widget *widget) {
    widget->methods = image_list_methods;
    kernel::lifecycle::Cleanup base([&] { destroy_general(reinterpret_cast<GeneralWidget *>(widget)); });
    kernel::lifecycle::Cleanup sequence([&] { destroy_image_sequence(member<ImageSequence>(widget, 264)); });
    kernel::lifecycle::Cleanup images([&] { container_destruct(member<Container>(widget, 320)); });
    kernel::lifecycle::Cleanup first([&] { engine::destroy_point(member<Point>(widget, 336)); });
    kernel::lifecycle::Cleanup second([&] { engine::destroy_point(member<Point>(widget, 348)); });
    remove_widget_timer(at<Dword>(widget, 376));
    second.run();
    first.run();
    images.run();
    sequence.run();
}

void button_destroy(Widget *widget) {
    widget->methods = button_methods;
    destroy_general(reinterpret_cast<GeneralWidget *>(widget));
}

void toggle_button_destroy(Widget *widget) {
    widget->methods = toggle_button_methods;
    member<ListItems>(widget, 312)->methods = toggle_items_methods;
    kernel::lifecycle::Cleanup base([&] { button_destroy(widget); });
    destroy_list_items(member<ListItems>(widget, 312));
}

void tab_button_destroy(Widget *widget) {
    widget->methods = tab_button_methods;
    string_destruct(member<String>(widget, 336));
    string_destruct(member<String>(widget, 312));
    button_destroy(widget);
}

void spin_button_destroy(Widget *widget) {
    widget->methods = spin_button_methods;
    destroy_general(reinterpret_cast<GeneralWidget *>(widget));
}

void list_box_destroy(Widget *widget) {
    widget->methods = list_box_methods;
    member<ListItems>(widget, 264)->methods = list_box_items_methods;
    // Native state0 protects only GWidget, including during the inlined String release.
    kernel::lifecycle::Cleanup base([&] { destroy_general(reinterpret_cast<GeneralWidget *>(widget)); });
    string_destruct(member<String>(widget, 328));
    destroy_list_items(member<ListItems>(widget, 264));
}

void combo_box_destroy(Widget *widget) {
    widget->methods = combo_box_methods;
    kernel::lifecycle::Cleanup base([&] { edit_text_destroy(widget); });
    container_remove(&combo_boxes, widget);
}

void tree_destroy(Widget *widget) {
    widget->methods = tree_methods;
    kernel::lifecycle::Cleanup base([&] { destroy_general(reinterpret_cast<GeneralWidget *>(widget)); });
    destroy_tree_node(member<TreeNode>(widget, 264));
}

void frame_destroy(Widget *widget) {
    widget->methods = general_methods;
    destroy_base(widget);
}

void scroll_bar_destroy(Widget *widget) {
    engine::destroy_rectangle(member<Rectangle>(widget, 288));
    widget->methods = general_methods;
    destroy_base(widget);
}

void scroll_view_destroy(Widget *widget) {
    engine::destroy_point(member<Point>(widget, 284));
    engine::destroy_rectangle(member<Rectangle>(widget, 264));
    widget->methods = general_methods;
    destroy_base(widget);
}

void text_view_destroy(Widget *widget) {
    engine::destroy_point(member<Point>(widget, 284));
    engine::destroy_rectangle(member<Rectangle>(widget, 264));
    widget->methods = general_methods;
    destroy_base(widget);
}

void gripper_destroy(Widget *widget) {
    anim_image_destroy(widget);
}

void slider_bar_destroy(Widget *widget) {
    engine::destroy_rectangle(member<Rectangle>(widget, 304));
    widget->methods = general_methods;
    destroy_base(widget);
}

void menu_destroy(Widget *widget) {
    widget->methods = menu_methods;
    kernel::lifecycle::Cleanup base([&] { destroy_general(reinterpret_cast<GeneralWidget *>(widget)); });
    kernel::lifecycle::Cleanup items([&] { container_destruct(member<Container>(widget, 292)); });
    destroy_image_resource(member<ImageResource>(widget, 316));
    items.run();
}

void dialog_destroy(Widget *widget) {
    widget->methods = dialog_methods;
    destroy_general(reinterpret_cast<GeneralWidget *>(widget));
}

void msg_box_destroy(Widget *widget) {
    widget->methods = dialog_methods;
    destroy_general(reinterpret_cast<GeneralWidget *>(widget));
}

void marquee_destroy(Widget *widget) {
    engine::destroy_point(member<Point>(widget, 276));
    engine::destroy_point(member<Point>(widget, 264));
    widget->methods = general_methods;
    destroy_base(widget);
}
} // namespace nfs::gui::widgets::family
