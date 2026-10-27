#include "frontend_dialogs.hpp"
namespace nfs::application::pending {
void *create_widget(const gui::String *, const gui::String *, const gui::String *, void *);

Widget *widget_root(Widget *widget) {
    while (widget->parent)
        widget = widget->parent;
    return widget;
}

void set_widget_tab_order(Widget *widget, Dword order) {
    widget->tab_index = int(order);
    if (Widget *parent = widget->parent) {
        gui::container_remove(&parent->tab_order, widget);
        gui::container_sorted_insert(&parent->tab_order, gui::widgets::compare_tab,
                                     reinterpret_cast<void *>(order), widget, nullptr);
    }
}

Dword set_widget_callback(Widget *widget, Dword event, gui::widgets::Callback callback) {
    if (!widget->handlers) {
        widget->handlers = gui::widgets::create_hook_container();
        if (!widget->handlers)
            return 1;
    }
    return gui::widgets::set_hook_callback(widget->handlers, event, callback);
}

Widget *create_message_widget(const gui::String *file, const gui::String *name,
                              const gui::String *type, void *parent, const gui::String *text,
                              const gui::String *title) {
    auto *widget = static_cast<Widget *>(create_widget(file, name, type, parent));
    if (!widget)
        return nullptr;
    if (!invoke<Dword>(widget, 2, gui::widgets::family::msg_box_class_key)) {
        destroy_screen(widget);
        return nullptr;
    }
    if (void *child = gui::events::find_widget(gui::context_state, at<Dword>(widget, 292)))
        invoke(child, 7, text);
    if (void *child = gui::events::find_widget(gui::context_state, at<Dword>(widget, 288)))
        invoke(child, 7, title);
    return widget;
}

gui::engine::Point *construct_dialog_point(gui::engine::Point *point, int x, int y) {
    *point = {gui::engine::point_methods, x, y};
    return point;
}

gui::engine::Rectangle *copy_dialog_rectangle(gui::engine::Rectangle *rectangle,
                                              const gui::engine::Rectangle *source) {
    *rectangle = {gui::engine::rectangle_methods, source->top, source->left, source->width,
                  source->height};
    return rectangle;
}
} // namespace nfs::application::pending
