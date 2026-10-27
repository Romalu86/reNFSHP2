#include "nfs/core/errors/assertions.hpp"
#include "gui_widget_resources.hpp"
#include "gui_widget_shared_methods.hpp"
#include "gui_widget_family_style.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include <cstring>
namespace nfs::gui::widgets {
}
namespace nfs::gui::widgets::family {
using game::runtime::at;
using game::runtime::invoke;
using namespace engine::pending;

void destroy_image_widget(Widget *widget) {
    widget->methods = image_widget_methods;
    destroy_general(reinterpret_cast<GeneralWidget *>(widget));
}

void *construct_image_widget(void *storage, void *parent) {
    auto *widget = static_cast<GeneralWidget *>(construct_widget(storage, parent));
    widget->base.flags = (widget->base.flags & ~Dword(6)) | 1;
    widget->base.methods = image_widget_methods;
    at<Dword>(widget, 264) = 0;
    return widget;
}

static Widget *__fastcall delete_image_widget(Widget *widget, void *, Dword flags) {
    destroy_image_widget(widget);
    if (flags & 1)
        release(widget);
    return widget;
}

static Dword __fastcall is_image_type(Widget *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(image_class_key) ||
           type == reinterpret_cast<const void *>(general_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

static Dword __fastcall is_image_name(Widget *, void *, const char *name) {
    return !std::strncmp(name, "GImage", 7) || !std::strncmp(name, "GWidget", 8) ||
           !std::strncmp(name, "GUI", 4);
}

 Dword __fastcall load_image_widget(Widget *widget, void *, void *configuration) {
    read_flags(configuration, "GImage.mImageStyle", &at<Dword>(widget, 264), image_style_flags, 0);
    return pending::load_general(reinterpret_cast<GeneralWidget *>(widget), nullptr, configuration);
}

 Dword __fastcall save_image_widget(Widget *widget, void *, void *configuration) {
    write_flags(configuration, "GImage.mImageStyle", at<Dword>(widget, 264), image_style_flags, 0);
    return pending::save_general(reinterpret_cast<GeneralWidget *>(widget), nullptr, configuration);
}

void __fastcall static_image_push_bounds(Widget *widget, void *, Dword render_address) {
    push_draw_bounds(widget, nullptr, reinterpret_cast<engine::Engine *>(render_address));
    if ((at<Dword>(widget, 264) & 1) && widget->parent)
        widget->colour = widget->parent->colour;
}

void __fastcall static_image_layout(Widget *widget, void *, Dword flags) {
    auto *resource = invoke<ImageResource *>(widget, 47);
    int left = widget->bounds.left, top = widget->bounds.top, height = widget->bounds.height,
        width = widget->bounds.width;
    const Dword style = at<Dword>(widget, 268);
    if (!(flags & 64)) {
        if (resource->bounds.width && (style & 2))
            height = int(Dword(resource->bounds.height) * Dword(width)) / resource->bounds.width;
        else if (resource->bounds.height && (style & 4))
            width = int(Dword(resource->bounds.width) * Dword(height)) / resource->bounds.height;
    } else if (!(style & 160)) {
        height = resource->bounds.height;
        width = resource->bounds.width;
    } else {
        height = resource->bounds.width;
        width = resource->bounds.height;
    }
    if (style & 1) {
        left = resource->bounds.left;
        top = resource->bounds.top;
    }
    widget->bounds.left = left;
    widget->bounds.top = top;
    widget->bounds.height = height;
    widget->bounds.width = width;
    pending::layout(widget, nullptr, flags);
}
// The unregistered GImage interface owns the shared methods used by all image classes.
#define METHOD(function) reinterpret_cast<Method>(function)
const Method image_widget_methods[47] = {METHOD(delete_image_widget),
                                         METHOD(::nfs::assertions::pure_virtual_call),
                                         METHOD(is_image_type),
                                         METHOD(is_image_name),
                                         METHOD(load_image_widget),
                                         METHOD(save_image_widget),
                                         METHOD(pending::layout),
                                         METHOD(set_text),
                                         METHOD(text),
                                         METHOD(pending::hit_test),
                                         METHOD(pending::navigate),
                                         METHOD(pending::enter_modal),
                                         METHOD(pending::leave_modal),
                                         METHOD(pending::general_event),
                                         METHOD(reserved_four_arguments),
                                         METHOD(reserved_two_arguments),
                                         METHOD(static_image_drawing_state),
                                         METHOD(prepare_draw),
                                         METHOD(static_image_push_bounds),
                                         METHOD(draw_content),
                                         METHOD(pop_draw_bounds),
                                         METHOD(pending::request_general_layout),
                                         METHOD(reserved_no_arguments),
                                         METHOD(reserved_render_argument),
                                         METHOD(reserved_style_argument),
                                         METHOD(clear_layout_request),
                                         METHOD(reserved_layout_argument),
                                         METHOD(notify_parent_enter),
                                         METHOD(notify_parent_leave),
                                         METHOD(reserved_parent_argument),
                                         METHOD(reserved_child_argument),
                                         METHOD(set_modal_state),
                                         METHOD(pending::general_key),
                                         METHOD(reserved_result),
                                         METHOD(style_result_34),
                                         METHOD(style_result_35),
                                         METHOD(pending::general_button),
                                         METHOD(style_result_37),
                                         METHOD(style_result_38),
                                         METHOD(pending::update_active_style),
                                         METHOD(pending::update_focus_style),
                                         METHOD(capture_style),
                                         METHOD(accept_general),
                                         METHOD(cancel_general),
                                         METHOD(notify_scroll),
                                         METHOD(process_general_command),
                                         METHOD(process_general_notification)};
#undef METHOD
} // namespace nfs::gui::widgets::family
