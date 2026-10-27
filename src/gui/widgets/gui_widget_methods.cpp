#include "gui_widgets.hpp"
#include "gui_widget_settings.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include "nfs/core/application/application.hpp"
#include "nfs/core/errors/assertions.hpp"
#include <cstring>
namespace nfs::gui::engine {
const Rectangle *current_viewport_bounds(const Base *);
}
namespace nfs::gui::widgets {
using game::runtime::invoke;
namespace pending {
Dword __fastcall load_configuration(Widget *, void *, void *);
Dword __fastcall save_configuration(Widget *, void *, void *);
void __fastcall layout(Widget *, void *, Dword);
Dword __fastcall hit_test(Widget *, void *);
Dword __fastcall navigate(Widget *, void *, Dword, Dword, Dword);
Dword __fastcall enter_modal(Widget *, void *);
void __fastcall leave_modal(Widget *, void *);
Dword __fastcall handle_event(Widget *, void *, Dword, Dword, Dword);
Dword __fastcall update_active_style(Widget *, void *, Dword, Dword);
Dword __fastcall update_focus_style(Widget *, void *, Dword, Dword);
} // namespace pending
Dword __fastcall apply_drawing_state(Widget *, void *, engine::Engine *);
void __fastcall prepare_draw(Widget *, void *, engine::Engine *);
void __fastcall push_draw_bounds(Widget *, void *, engine::Engine *);
void __fastcall pop_draw_bounds(Widget *, void *, engine::Engine *);
void __fastcall request_layout(Widget *, void *);
Dword read_image_resource(void *, const char *, ImageResource *);
Dword write_image_resource(void *, const char *, ImageResource *);

const char *__cdecl base_class_name() {
    return "GUI";
}

static Dword __fastcall is_base_name(Widget *, void *, const char *name) {
    return std::strncmp(name, "GUI", 4) == 0;
}

static Dword __fastcall is_base_type(Widget *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(base_class_name);
}

static Widget *__fastcall delete_base(Widget *widget, void *, Dword flags) {
    destroy_base(widget);
    if (flags & 1)
        release(widget);
    return widget;
}

void __fastcall set_text(Widget *widget, void *, const String *text) {
    string_copy(&widget->text, text);
}

const String *__fastcall text(Widget *widget, void *) {
    return (widget->flags & 1048576) && widget->parent ? invoke<const String *>(widget->parent, 8)
                                                        : &widget->text;
}
// Complete body is RET16.
void __fastcall reserved_four_arguments(Widget *, void *, Dword, Dword, Dword, Dword) {
}
// Complete body is RET8.
void __fastcall reserved_two_arguments(Widget *, void *, Dword, Dword) {
}
// Complete body is RET4.
void __fastcall draw_content(Widget *, void *, engine::Engine *) {
}
 
void __fastcall reserved_no_arguments(Widget *, void *) {
}
// Complete body is RET4.
void __fastcall reserved_render_argument(Widget *, void *, Dword) {
}
// Complete body is RET4.
void __fastcall reserved_style_argument(Widget *, void *, Dword) {
}

void __fastcall clear_layout_request(Widget *widget, void *) {
    widget->options &= ~Dword(2);
}
// Complete body is RET4.
void __fastcall reserved_layout_argument(Widget *, void *, Dword) {
}
// The ignored stack argument is present in the RET4.
void __fastcall notify_parent_enter(Widget *widget, void *, Dword) {
    if (widget->parent)
        events::dispatch(widget->parent, 16, widget->id, 0);
}
// The ignored stack argument is present in the RET4.
void __fastcall notify_parent_leave(Widget *widget, void *, Dword) {
    if (widget->parent)
        events::dispatch(widget->parent, 17, widget->id, 0);
}
// Complete body is RET4.
void __fastcall reserved_parent_argument(Widget *, void *, Dword) {
}
// Complete body is RET4.
void __fastcall reserved_child_argument(Widget *, void *, Dword) {
}

Dword __fastcall set_modal_state(Widget *widget, void *, Dword value, Dword) {
    widget->modal_result = value;
    return 1;
}

 Dword __fastcall style_result_32(Widget *widget, void *, Dword, Dword) {
    return widget->flags & 16512;
}
// Complete body is XOR EAX,EAX; RET8.
Dword __fastcall reserved_result(Widget *, void *, Dword, Dword) {
    return 0;
}

Dword __fastcall style_result_34(Widget *widget, void *, Dword, Dword) {
    return widget->flags & 16512;
}

Dword __fastcall style_result_35(Widget *widget, void *, Dword, Dword) {
    return widget->flags & 16512;
}

 Dword __fastcall style_result_36(Widget *widget, void *, Dword, Dword) {
    return widget->flags & 16512;
}

Dword __fastcall style_result_37(Widget *widget, void *, Dword, Dword) {
    return widget->flags & 16512;
}

Dword __fastcall style_result_38(Widget *widget, void *, Dword, Dword) {
    return widget->flags & 82048;
}

Dword __fastcall capture_style(Widget *widget, void *, Dword, Dword) {
    return application::pending::captured_widget_id == widget->id ? 1 : widget->flags & 82048;
}

void *construct_simple_image(void *storage, void *parent) {
    construct_base(storage, parent);
    auto *widget = static_cast<SimpleImage *>(storage);
    try {
        construct_image_resource(&widget->resource);
    } catch (...) {
        destroy_base(&widget->base);
        throw;
    }
    widget->base.flags = (widget->base.flags & ~Dword(6)) | 1;
    widget->base.methods = simple_image_methods;
    return widget;
}

static void destroy_simple_image(SimpleImage *widget) {
    widget->base.methods = simple_image_methods;
    destroy_image_resource(&widget->resource);
    destroy_base(&widget->base);
}

static SimpleImage *__fastcall delete_simple_image(SimpleImage *widget, void *, Dword flags) {
    destroy_simple_image(widget);
    if (flags & 1)
        release(widget);
    return widget;
}

static const char *__fastcall simple_image_name(Widget *, void *) {
    return "GSimpleImage";
}

static const char *__cdecl simple_image_class_name() {
    return "GSimpleImage";
}

static Dword __fastcall is_simple_image_type(Widget *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(simple_image_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

static Dword __fastcall is_simple_image_name(Widget *, void *, const char *name) {
    return !std::strncmp(name, "GSimpleImage", 13) || !std::strncmp(name, "GUI", 4);
}

static Dword __fastcall load_simple_image(SimpleImage *widget, void *, void *configuration) {
    read_image_resource(configuration, "GSimpleImage.mShape", &widget->resource);
    return pending::load_configuration(&widget->base, nullptr, configuration);
}

static Dword __fastcall save_simple_image(SimpleImage *widget, void *, void *configuration) {
    write_image_resource(configuration, "GSimpleImage.mShape", &widget->resource);
    return pending::save_configuration(&widget->base, nullptr, configuration);
}

static void __fastcall draw_simple_image(SimpleImage *widget, void *, engine::Engine *engine) {
    if (widget->resource.image)
        invoke(engine, 180 / 4, widget->resource.image,
               engine::current_viewport_bounds(&engine->base), &widget->base.colour, Dword(0));
}

static Dword __fastcall simple_image_event(SimpleImage *widget, void *, Dword event, Dword first,
                                          Dword second) {
    if (event != 52)
        return pending::handle_event(&widget->base, nullptr, event, first, second);
    if (first)
        copy_image_resource(&widget->resource, reinterpret_cast<const ImageResource *>(first));
    else
        clear_image_resource(&widget->resource);
    return 1;
}

static void __fastcall layout_simple_image(SimpleImage *widget, void *, Dword flags) {
    if ((flags & 64) && widget->resource.image) {
        widget->base.bounds.width = widget->resource.bounds.width;
        widget->base.bounds.height = widget->resource.bounds.height;
    }
    pending::layout(&widget->base, nullptr, flags);
}

#define METHOD(function) reinterpret_cast<Method>(function)
const Method widget_methods[42] = {

    METHOD(delete_base),
    METHOD(::nfs::assertions::pure_virtual_call),
    METHOD(is_base_type),
    METHOD(is_base_name),
    METHOD(pending::load_configuration),
    METHOD(pending::save_configuration),
    METHOD(pending::layout),
    METHOD(set_text),
    METHOD(text),
    METHOD(pending::hit_test),
    METHOD(pending::navigate),
    METHOD(pending::enter_modal),
    METHOD(pending::leave_modal),
    METHOD(pending::handle_event),
    METHOD(reserved_four_arguments),
    METHOD(reserved_two_arguments),
    METHOD(apply_drawing_state),
    METHOD(prepare_draw),
    METHOD(push_draw_bounds),
    METHOD(draw_content),
    METHOD(pop_draw_bounds),
    METHOD(request_layout),
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
    METHOD(style_result_32),
    METHOD(reserved_result),
    METHOD(style_result_34),
    METHOD(style_result_35),
    METHOD(style_result_36),
    METHOD(style_result_37),
    METHOD(style_result_38),
    METHOD(pending::update_active_style),
    METHOD(pending::update_focus_style),
    METHOD(capture_style),
};
const Method simple_image_methods[42] = {

    METHOD(delete_simple_image),
    METHOD(simple_image_name),
    METHOD(is_simple_image_type),
    METHOD(is_simple_image_name),
    METHOD(load_simple_image),
    METHOD(save_simple_image),
    METHOD(layout_simple_image),
    METHOD(set_text),
    METHOD(text),
    METHOD(pending::hit_test),
    METHOD(pending::navigate),
    METHOD(pending::enter_modal),
    METHOD(pending::leave_modal),
    METHOD(simple_image_event),
    METHOD(reserved_four_arguments),
    METHOD(reserved_two_arguments),
    METHOD(apply_drawing_state),
    METHOD(prepare_draw),
    METHOD(push_draw_bounds),
    METHOD(draw_simple_image),
    METHOD(pop_draw_bounds),
    METHOD(request_layout),
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
    METHOD(style_result_32),
    METHOD(reserved_result),
    METHOD(style_result_34),
    METHOD(style_result_35),
    METHOD(style_result_36),
    METHOD(style_result_37),
    METHOD(style_result_38),
    METHOD(pending::update_active_style),
    METHOD(pending::update_focus_style),
    METHOD(capture_style),
};

const char *__cdecl general_class_name() {
    return "GWidget";
}

static const char *__fastcall general_name(Widget *, void *) {
    return "GWidget";
}

static Dword __fastcall is_general_type(Widget *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(general_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

static Dword __fastcall is_general_name(Widget *, void *, const char *name) {
    return !std::strncmp(name, "GWidget", 8) || !std::strncmp(name, "GUI", 4);
}

static GeneralWidget *__fastcall delete_general(GeneralWidget *widget, void *, Dword flags) {
    destroy_general(widget);
    if (flags & 1)
        release(widget);
    return widget;
}
 
void __fastcall accept_general(Widget *, void *) {
}
 
void __fastcall cancel_general(Widget *, void *) {
}
// Body is RET4.
void __fastcall notify_scroll(Widget *, void *, Widget *) {
}
// Body is XOR EAX,EAX; RET8.
Dword __fastcall process_general_command(Widget *, void *, Dword, Dword) {
    return 0;
}
// Body is XOR EAX,EAX; RET8.
Dword __fastcall process_general_notification(Widget *, void *, Dword, Dword) {
    return 0;
}
const Method general_methods[47] = {
    METHOD(delete_general),
    METHOD(general_name),
    METHOD(is_general_type),
    METHOD(is_general_name),
    METHOD(pending::load_general),
    METHOD(pending::save_general),
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
    METHOD(apply_drawing_state),
    METHOD(prepare_draw),
    METHOD(push_draw_bounds),
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

const char *__cdecl interface_class_name() {
    return "GInterface";
}

static const char *__fastcall interface_name(Widget *, void *) {
    return "GInterface";
}

static Dword __fastcall is_interface_type(Widget *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(interface_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

static Dword __fastcall is_interface_name(Widget *, void *, const char *name) {
    return !std::strncmp(name, "GInterface", 11) || !std::strncmp(name, "GUI", 4);
}

void destroy_interface(Widget *widget) {
    widget->methods = interface_methods;
    destroy_base(widget);
}

static Widget *__fastcall delete_interface(Widget *widget, void *, Dword flags) {
    destroy_interface(widget);
    if (flags & 1)
        release(widget);
    return widget;
}

static const char *__cdecl simple_text_class_name() {
    return "GSimpleText";
}

static const char *__fastcall simple_text_name(Widget *, void *) {
    return "GSimpleText";
}

static Dword __fastcall is_simple_text_type(Widget *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(simple_text_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

static Dword __fastcall is_simple_text_name(Widget *, void *, const char *name) {
    return !std::strncmp(name, "GSimpleText", 12) || !std::strncmp(name, "GUI", 4);
}

static void destroy_simple_text(Widget *widget) {
    widget->methods = simple_text_methods;
    destroy_base(widget);
}

static Widget *__fastcall delete_simple_text(Widget *widget, void *, Dword flags) {
    destroy_simple_text(widget);
    if (flags & 1)
        release(widget);
    return widget;
}
const Method interface_methods[42] = {
    METHOD(delete_interface),
    METHOD(interface_name),
    METHOD(is_interface_type),
    METHOD(is_interface_name),
    METHOD(pending::load_interface),
    METHOD(pending::save_interface),
    METHOD(pending::layout),
    METHOD(set_text),
    METHOD(text),
    METHOD(pending::hit_test),
    METHOD(pending::navigate),
    METHOD(pending::enter_interface),
    METHOD(pending::leave_modal),
    METHOD(pending::handle_event),
    METHOD(reserved_four_arguments),
    METHOD(reserved_two_arguments),
    METHOD(apply_drawing_state),
    METHOD(prepare_draw),
    METHOD(push_draw_bounds),
    METHOD(draw_content),
    METHOD(pop_draw_bounds),
    METHOD(pending::request_interface_layout),
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
    METHOD(pending::interface_key),
    METHOD(reserved_result),
    METHOD(style_result_34),
    METHOD(style_result_35),
    METHOD(pending::interface_button),
    METHOD(style_result_37),
    METHOD(style_result_38),
    METHOD(pending::update_active_style),
    METHOD(pending::update_focus_style),
    METHOD(capture_style)};
const Method simple_text_methods[42] = {
    METHOD(delete_simple_text),
    METHOD(simple_text_name),
    METHOD(is_simple_text_type),
    METHOD(is_simple_text_name),
    METHOD(pending::load_configuration),
    METHOD(pending::save_configuration),
    METHOD(pending::layout_simple_text),
    METHOD(set_text),
    METHOD(text),
    METHOD(pending::hit_test),
    METHOD(pending::navigate),
    METHOD(pending::enter_modal),
    METHOD(pending::leave_modal),
    METHOD(pending::handle_event),
    METHOD(reserved_four_arguments),
    METHOD(reserved_two_arguments),
    METHOD(apply_drawing_state),
    METHOD(prepare_draw),
    METHOD(push_draw_bounds),
    METHOD(pending::draw_simple_text),
    METHOD(pop_draw_bounds),
    METHOD(request_layout),
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
    METHOD(style_result_32),
    METHOD(reserved_result),
    METHOD(style_result_34),
    METHOD(style_result_35),
    METHOD(style_result_36),
    METHOD(style_result_37),
    METHOD(style_result_38),
    METHOD(pending::update_active_style),
    METHOD(pending::update_focus_style),
    METHOD(capture_style)};
#undef METHOD
} // namespace nfs::gui::widgets
