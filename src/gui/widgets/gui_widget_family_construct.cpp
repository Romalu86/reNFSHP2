#include "gui_widget_resources.hpp"
#include "gui_widget_shared_methods.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
namespace nfs::gui::widgets::family {
void destroy_image_widget(Widget *);
}
namespace nfs::gui::widgets::pending {
using game::runtime::at;
using game::runtime::bytes;
using namespace family;
namespace {
template <class T> T *member(void *object, Dword offset) {
    return reinterpret_cast<T *>(bytes(object) + offset);
}
void initialize_text_fields(Widget *widget) {
    at<Dword>(widget, 264) = 0;
    widget->flags = (widget->flags & ~Dword(6)) | 1;
    at<std::uint8_t>(widget, 268) = '*';
    widget->colour = 4294967295u;
    at<Dword>(widget, 272) = 0;
    at<Dword>(widget, 276) = 0;
}
void initialize_button_fields(Widget *widget) {
    for (Dword offset :
         {264u, 268u, 272u, 280u, 284u, 288u, 292u, 276u, 300u, 296u, 304u})
        at<Dword>(widget, offset) = 0;
    widget->methods = button_methods;
    widget->flags = (widget->flags & ~Dword(3)) | 4;
    at<Dword>(widget, 308) = 4294967295u;
}
void initialize_scroll_fields(Widget *widget) {
    at<Dword>(widget, 256) = 0;
    at<Dword>(widget, 260) = 31;
    widget->flags |= 4;
    engine::construct_rectangle(member<Rectangle>(widget, 264));
    engine::construct_point(member<Point>(widget, 284));
    member<Point>(widget, 284)->methods = vector_methods;
    at<Dword>(widget, 296) = 0;
    at<Dword>(widget, 288) = 1;
    at<Dword>(widget, 292) = 1;
}
} // namespace

void *construct_text(void *storage, void *parent) {
    auto *widget = static_cast<Widget *>(construct_widget(storage, parent));
    at<Dword>(widget, 264) = 0;
    widget->methods = text_methods;
    initialize_text_fields(widget);
    return widget;
}

void *construct_edit_text(void *storage, void *parent) {
    auto *widget = static_cast<Widget *>(construct_widget(storage, parent));
    initialize_text_fields(widget);
    bool text_complete = false;
    try {
        string_construct(member<String>(widget, 288));
        text_complete = true;
        widget->methods = edit_text_methods;
        at<Dword>(widget, 308) = 0;
        set_enabled(widget, 1);
        at<Dword>(widget, 280) = at<Dword>(widget, 284) = 0;
        at<Dword>(widget, 316) = at<Dword>(widget, 320) = at<Dword>(widget, 324) = 0;
        widget->layout_flags &= ~Dword(64);
        at<Dword>(widget, 272) = 5;
        at<const char *>(widget, 312) = edit_characters;
    } catch (...) {
        if (text_complete)
            string_destruct(member<String>(widget, 288));
        text_destroy(widget);
        throw;
    }
    return widget;
}

void *construct_tool_tip(void *storage, void *parent) {
    auto *widget = static_cast<Widget *>(construct_base(storage, parent));
    for (Dword offset : {256u, 280u, 284u, 268u, 264u})
        at<Dword>(widget, offset) = 0;
    at<Dword>(widget, 260) = 31;
    widget->methods = tool_tip_methods;
    widget->flags |= 15;
    widget->layout_flags |= 131136;
    at<float>(widget, 272) = 0.5f;
    at<float>(widget, 276) = 6;
    return widget;
}

void *construct_static_image(void *storage, void *parent) {
    auto *widget = static_cast<Widget *>(construct_widget(storage, parent));
    widget->methods = image_widget_methods;
    widget->flags = (widget->flags & ~Dword(6)) | 1;
    at<Dword>(widget, 264) = 0;
    try {
        construct_image_resource(member<ImageResource>(widget, 272));
    } catch (...) {
        destroy_image_widget(widget);
        throw;
    }
    at<Dword>(widget, 268) = at<Dword>(widget, 312) = 0;
    at<float>(widget, 316) = at<float>(widget, 320) = 1;
    widget->methods = static_image_methods;
    return widget;
}

void *construct_anim_image(void *storage, void *parent) {
    auto *widget = static_cast<Widget *>(construct_static_image(storage, parent));
    bool sequence_complete = false;
    try {
        construct_image_sequence(member<ImageSequence>(widget, 328));
        sequence_complete = true;
        widget->methods = anim_image_methods;
        at<Dword>(widget, 324) = at<Dword>(widget, 384) = at<Dword>(widget, 388) =
            at<Dword>(widget, 392) = 0;
        at<Dword>(widget, 396) = create_widget_timer(widget->id, 0);
    } catch (...) {
        if (sequence_complete)
            destroy_image_sequence(member<ImageSequence>(widget, 328));
        static_image_destroy(widget);
        throw;
    }
    return widget;
}

void *construct_image_box(void *storage, void *parent) {
    auto *widget = static_cast<Widget *>(construct_widget(storage, parent));
    at<Dword>(widget, 264) = 0;
    widget->flags = (widget->flags & ~Dword(6)) | 1;
    for (Dword offset : {272u, 276u, 280u, 284u, 268u})
        at<Dword>(widget, offset) = 0;
    widget->methods = image_box_methods;
    return widget;
}

void *construct_image_bar(void *storage, void *parent) {
    auto *widget = static_cast<Widget *>(construct_widget(storage, parent));
    for (Dword offset : {264u, 268u, 272u, 276u, 280u})
        at<Dword>(widget, offset) = 0;
    widget->flags = (widget->flags & ~Dword(6)) | 1;
    widget->methods = image_bar_methods;
    return widget;
}

void *construct_image_list(void *storage, void *parent) {
    auto *widget = static_cast<Widget *>(construct_widget(storage, parent));
    int completed = 0;
    try {
        construct_image_sequence(member<ImageSequence>(widget, 264));
        completed = 1;
        container_construct(member<Container>(widget, 320));
        completed = 2;
        engine::construct_point(member<Point>(widget, 336));
        member<Point>(widget, 336)->methods = vector_methods;
        completed = 3;
        engine::construct_point(member<Point>(widget, 348));
        member<Point>(widget, 348)->methods = vector_methods;
        completed = 4;
        widget->methods = image_list_methods;
        at<Dword>(widget, 372) = at<Dword>(widget, 368) = at<Dword>(widget, 384) = 0;
        at<float>(widget, 380) = 1;
        at<Dword>(widget, 376) = create_widget_timer(widget->id, 0.0333333350718021392822265625f);
    } catch (...) {
        if (completed >= 4)
            engine::destroy_point(member<Point>(widget, 348));
        if (completed >= 3)
            engine::destroy_point(member<Point>(widget, 336));
        if (completed >= 2)
            container_destruct(member<Container>(widget, 320));
        if (completed >= 1)
            destroy_image_sequence(member<ImageSequence>(widget, 264));
        destroy_general(reinterpret_cast<GeneralWidget *>(widget));
        throw;
    }
    return widget;
}

void *construct_button(void *storage, void *parent) {
    auto *widget = static_cast<Widget *>(construct_widget(storage, parent));
    initialize_button_fields(widget);
    return widget;
}

void *construct_toggle_button(void *storage, void *parent) {
    auto *widget = static_cast<Widget *>(construct_button(storage, parent));
    try {
        construct_list_items(member<ListItems>(widget, 312));
    } catch (...) {
        button_destroy(widget);
        throw;
    }
    member<ListItems>(widget, 312)->methods = toggle_items_methods;
    widget->methods = toggle_button_methods;
    return widget;
}

void *construct_tab_button(void *storage, void *parent) {
    auto *widget = static_cast<Widget *>(construct_button(storage, parent));
    string_construct(member<String>(widget, 312));
    string_construct(member<String>(widget, 336));
    at<Dword>(widget, 332) = 0;
    widget->methods = tab_button_methods;
    at<Dword>(widget, 296) = 68;
    return widget;
}

void *construct_spin_button(void *storage, void *parent) {
    auto *widget = static_cast<Widget *>(construct_widget(storage, parent));
    at<Dword>(widget, 268) = at<Dword>(widget, 272) = 100;
    for (Dword offset : {264u, 276u, 284u, 280u, 288u, 292u, 296u})
        at<Dword>(widget, offset) = 0;
    widget->methods = spin_button_methods;
    widget->flags = (widget->flags & ~Dword(4)) | 4608;
    return widget;
}

void *construct_list_box(void *storage, void *parent) {
    auto *widget = static_cast<Widget *>(construct_widget(storage, parent));
    try {
        construct_list_items(member<ListItems>(widget, 264));
    } catch (...) {
        destroy_general(reinterpret_cast<GeneralWidget *>(widget));
        throw;
    }
    string_construct(member<String>(widget, 328));
    for (Dword offset : {320u, 324u, 348u, 352u, 356u, 360u, 364u})
        at<Dword>(widget, offset) = 0;
    at<Dword>(widget, 312) = at<Dword>(widget, 316) = 4294967295u;
    widget->methods = list_box_methods;
    at<Dword>(widget, 308) = 15;
    member<ListItems>(widget, 264)->methods = list_box_items_methods;
    return widget;
}

void *construct_combo_box(void *storage, void *parent) {
    auto *widget = static_cast<Widget *>(construct_edit_text(storage, parent));
    widget->methods = combo_box_methods;
    at<Dword>(widget, 336) = at<Dword>(widget, 328) = at<Dword>(widget, 332) = 0;
    widget->layout_flags |= 64;
    at<Dword>(widget, 340) = 4294967295u;
    try {
        container_append(&combo_boxes, widget, nullptr);
    } catch (...) {
        edit_text_destroy(widget);
        throw;
    }
    return widget;
}

void *construct_tree(void *storage, void *parent) {
    auto *widget = static_cast<Widget *>(construct_widget(storage, parent));
    try {
        construct_tree_node(member<TreeNode>(widget, 264),
                        &lifecycle::construction::unresolved::shared_string, 0);
    } catch (...) {
        destroy_general(reinterpret_cast<GeneralWidget *>(widget));
        throw;
    }
    at<Dword>(widget, 336) = at<Dword>(widget, 332) = at<Dword>(widget, 352) = 0;
    at<Dword>(widget, 348) = at<Dword>(widget, 344) = 4294967295u;
    widget->methods = tree_methods;
    at<Dword>(widget, 340) = 15;
    return widget;
}

void *construct_frame(void *storage, void *parent) {
    auto *widget = static_cast<Widget *>(construct_base(storage, parent));
    widget->flags = (widget->flags & ~Dword(4)) | 4128;
    for (Dword offset : {256u, 268u, 272u, 276u, 280u, 264u, 284u, 288u, 292u,
                        296u, 304u, 308u, 312u, 316u, 300u})
        at<Dword>(widget, offset) = 0;
    at<Dword>(widget, 260) = 31;
    widget->methods = frame_methods;
    widget->layout_flags |= 64;
    return widget;
}

void *construct_scroll_bar(void *storage, void *parent) {
    auto *widget = static_cast<Widget *>(construct_base(storage, parent));
    widget->flags |= 4;
    at<Dword>(widget, 256) = 0;
    at<Dword>(widget, 260) = 31;
    engine::construct_rectangle(member<Rectangle>(widget, 288));
    for (Dword offset : {264u, 268u, 272u, 280u, 284u, 328u, 312u, 308u, 316u,
                        320u, 324u, 276u})
        at<Dword>(widget, offset) = 0;
    widget->methods = scroll_bar_methods;
    widget->flags = (widget->flags & ~Dword(4)) | 4192;
    at<Dword>(widget, 332) = 10;
    return widget;
}

void *construct_scroll_view(void *storage, void *parent) {
    auto *widget = static_cast<Widget *>(construct_base(storage, parent));
    initialize_scroll_fields(widget);
    widget->methods = scroll_view_methods;
    return widget;
}

void *construct_text_view(void *storage, void *parent) {
    auto *widget = static_cast<Widget *>(construct_base(storage, parent));
    initialize_scroll_fields(widget);
    at<Dword>(widget, 300) = 0;
    widget->methods = text_view_methods;
    return widget;
}

void *construct_gripper(void *storage, void *parent) {
    auto *widget = static_cast<Widget *>(construct_anim_image(storage, parent));
    at<Dword>(widget, 400) = 0;
    widget->flags = (widget->flags & ~Dword(5)) | 4096;
    widget->methods = gripper_methods;
    return widget;
}

void *construct_slider_bar(void *storage, void *parent) {
    auto *widget = static_cast<Widget *>(construct_base(storage, parent));
    widget->flags |= 4;
    at<Dword>(widget, 256) = 0;
    at<Dword>(widget, 260) = 31;
    engine::construct_rectangle(member<Rectangle>(widget, 304));
    for (Dword offset :
         {264u, 268u, 272u, 276u, 280u, 300u, 284u, 292u, 296u, 288u})
        at<Dword>(widget, offset) = 0;
    widget->methods = slider_bar_methods;
    return widget;
}

void *construct_menu(void *storage, void *parent) {
    auto *widget = static_cast<Widget *>(construct_widget(storage, parent));
    container_construct(member<Container>(widget, 292));
    try {
        construct_image_resource(member<ImageResource>(widget, 316));
    } catch (...) {
        container_destruct(member<Container>(widget, 292));
        destroy_general(reinterpret_cast<GeneralWidget *>(widget));
        throw;
    }
    for (Dword offset : {264u, 268u, 272u, 276u, 356u, 312u, 280u, 284u, 360u})
        at<Dword>(widget, offset) = 0;
    widget->flags |= 96;
    widget->methods = menu_methods;
    at<Dword>(widget, 308) = 4294967295u;
    at<float>(widget, 288) = 1;
    return widget;
}

void *construct_dialog(void *storage, void *parent) {
    auto *widget = static_cast<Widget *>(construct_widget(storage, parent));
    widget->flags |= 139;
    for (Dword offset : {264u, 268u, 276u, 280u, 284u})
        at<Dword>(widget, offset) = 0;
    widget->methods = dialog_methods;
    at<Dword>(widget, 272) = 3;
    return widget;
}

void *construct_msg_box(void *storage, void *parent) {
    auto *widget = static_cast<Widget *>(construct_widget(storage, parent));
    for (Dword offset : {264u, 268u, 276u, 280u, 284u, 288u, 292u})
        at<Dword>(widget, offset) = 0;
    at<Dword>(widget, 272) = 3;
    widget->methods = msg_box_methods;
    widget->flags |= 139;
    return widget;
}

void *construct_marquee(void *storage, void *parent) {
    auto *widget = static_cast<Widget *>(construct_base(storage, parent));
    at<Dword>(widget, 256) = 0;
    at<Dword>(widget, 260) = 31;
    engine::construct_point(member<Point>(widget, 264));
    engine::construct_point(member<Point>(widget, 276));
    at<Dword>(widget, 296) = at<Dword>(widget, 300) = at<Dword>(widget, 288) = 0;
    widget->methods = marquee_methods;
    widget->flags = 40;
    at<Dword>(widget, 292) = 3;
    widget->colour = 4278255360u;
    at<Dword>(widget, 300) = 2130738944;
    at<Dword>(widget, 296) = 4278255423u;
    return widget;
}
} // namespace nfs::gui::widgets::pending
