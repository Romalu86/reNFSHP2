#include "../widgets/gui_widget_shared_methods.hpp"
#include "hud_instruments.hpp"
#include <cstring>
namespace nfs::application::frontend::hud {
using namespace gui::widgets;
static Speedometer *__fastcall delete_speedometer(Speedometer *, void *, Dword);

static Speedometer *__fastcall delete_speedometer_binding(Binding *binding, void *, Dword flags) {
    return delete_speedometer(reinterpret_cast<Speedometer *>(reinterpret_cast<Byte *>(binding) - 264), nullptr, flags);
}
static const Method speedometer_binding_methods[1] = {reinterpret_cast<Method>(delete_speedometer_binding)};

static Speedometer *construct_speedometer(Speedometer *widget, void *parent) {
    construct_needle(&widget->base, parent);
    widget->minimum_mph = widget->maximum_mph = widget->minimum_kph = widget->maximum_kph = 0;
    widget->base.base.base.methods = speedometer_methods.data();
    widget->base.binding.methods = speedometer_binding_methods;
    widget->base.base.base.flags = (widget->base.base.base.flags & ~Dword(4)) | 1;
    return widget;
}

static void destroy_speedometer(Speedometer *widget) {
    widget->base.base.base.methods = speedometer_methods.data();
    widget->base.binding.methods = speedometer_binding_methods;
    destroy_needle(&widget->base);
}

static Speedometer *__fastcall delete_speedometer(Speedometer *widget, void *, Dword flags) {
    destroy_speedometer(widget);
    if (flags & 1) gui::release(widget);
    return widget;
}

const char *__cdecl speedometer_class_key() { return "HudSpeedometer"; }

static const char *__fastcall speedometer_name(Speedometer *, void *) { return "HudSpeedometer"; }

static Dword __fastcall speedometer_is_type(Speedometer *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(speedometer_class_key) ||
        type == reinterpret_cast<const void *>(needle_class_key) ||
        type == reinterpret_cast<const void *>(general_class_name) ||
        type == reinterpret_cast<const void *>(base_class_name);
}

static Dword __fastcall speedometer_is_name(Speedometer *, void *, const char *name) {
    return !std::strncmp(name, "HudSpeedometer", 15) ||
        !std::strncmp(name, "MNeedle", 8) ||
        !std::strncmp(name, "GWidget", 8) ||
        !std::strncmp(name, "GUI", 4);
}

const std::array<Method, 47> speedometer_methods = {
    reinterpret_cast<Method>(::nfs::application::frontend::hud::delete_speedometer), // 0.
    reinterpret_cast<Method>(::nfs::application::frontend::hud::speedometer_name), // 1.
    reinterpret_cast<Method>(::nfs::application::frontend::hud::speedometer_is_type), // 2.
    reinterpret_cast<Method>(::nfs::application::frontend::hud::speedometer_is_name), // 3.
    reinterpret_cast<Method>(::nfs::application::frontend::hud::load_speedometer), // 4.
    reinterpret_cast<Method>(::nfs::application::frontend::hud::save_speedometer), // 5.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::layout), // 6.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_text), // 7.
    reinterpret_cast<Method>(::nfs::gui::widgets::text), // 8.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::hit_test), // 9.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::navigate), // 10.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::enter_modal), // 11.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::leave_modal), // 12.
    reinterpret_cast<Method>(::nfs::application::frontend::hud::speedometer_event), // 13.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_four_arguments), // 14.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_two_arguments), // 15.
    reinterpret_cast<Method>(::nfs::gui::widgets::apply_drawing_state), // 16.
    reinterpret_cast<Method>(::nfs::gui::widgets::prepare_draw), // 17.
    reinterpret_cast<Method>(::nfs::gui::widgets::push_draw_bounds), // 18.
    reinterpret_cast<Method>(::nfs::application::frontend::hud::draw_needle), // 19.
    reinterpret_cast<Method>(::nfs::gui::widgets::pop_draw_bounds), // 20.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::request_general_layout), // 21.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_no_arguments), // 22.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_render_argument), // 23.
    reinterpret_cast<Method>(::nfs::application::frontend::hud::tick_needle), // 24.
    reinterpret_cast<Method>(::nfs::gui::widgets::clear_layout_request), // 25.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_layout_argument), // 26.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_parent_enter), // 27.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_parent_leave), // 28.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_parent_argument), // 29.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_child_argument), // 30.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_modal_state), // 31.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::general_key), // 32.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_result), // 33.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_34), // 34.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_35), // 35.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::general_button), // 36.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_37), // 37.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_38), // 38.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::update_active_style), // 39.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::update_focus_style), // 40.
    reinterpret_cast<Method>(::nfs::gui::widgets::capture_style), // 41.
    reinterpret_cast<Method>(::nfs::gui::widgets::accept_general), // 42.
    reinterpret_cast<Method>(::nfs::gui::widgets::cancel_general), // 43.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_scroll), // 44.
    reinterpret_cast<Method>(::nfs::gui::widgets::process_general_command), // 45.
    reinterpret_cast<Method>(::nfs::gui::widgets::process_general_notification), // 46.
};

void *create_speedometer(void *parent) {
    auto *widget = static_cast<Speedometer *>(gui::allocate_flags(sizeof(Speedometer), 0, "HudSpeedometer"));
    if (!widget) return nullptr;
    try { return construct_speedometer(widget, parent); }
    catch (...) { gui::release_flags(widget, 0, "HudSpeedometer"); throw; }
}
}
