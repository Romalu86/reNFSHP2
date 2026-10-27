#include "../widgets/gui_widget_shared_methods.hpp"
#include "hud_instruments.hpp"
#include <cstring>
namespace nfs::application::frontend::hud {
using namespace gui::widgets;
static Tachometer *__fastcall delete_tachometer(Tachometer *, void *, Dword);

static Tachometer *__fastcall delete_tachometer_binding(Binding *binding, void *, Dword flags) {
    return delete_tachometer(reinterpret_cast<Tachometer *>(reinterpret_cast<Byte *>(binding) - 264), nullptr, flags);
}
static const Method tachometer_binding_methods[1] = {reinterpret_cast<Method>(delete_tachometer_binding)};

static Tachometer *construct_tachometer(Tachometer *widget, void *parent) {
    construct_needle(&widget->base, parent);
    widget->minimum_rpm = widget->maximum_rpm = 0;
    widget->base.base.base.methods = tachometer_methods.data();
    widget->base.binding.methods = tachometer_binding_methods;
    widget->base.base.base.flags = (widget->base.base.base.flags & ~Dword(4)) | 1;
    return widget;
}

static void destroy_tachometer(Tachometer *widget) {
    widget->base.base.base.methods = tachometer_methods.data();
    widget->base.binding.methods = tachometer_binding_methods;
    destroy_needle(&widget->base);
}

static Tachometer *__fastcall delete_tachometer(Tachometer *widget, void *, Dword flags) {
    destroy_tachometer(widget);
    if (flags & 1) gui::release(widget);
    return widget;
}

const char *__cdecl tachometer_class_key() { return "HudTachometer"; }

static const char *__fastcall tachometer_name(Tachometer *, void *) { return "HudTachometer"; }

static Dword __fastcall tachometer_is_type(Tachometer *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(tachometer_class_key) ||
        type == reinterpret_cast<const void *>(needle_class_key) ||
        type == reinterpret_cast<const void *>(general_class_name) ||
        type == reinterpret_cast<const void *>(base_class_name);
}

static Dword __fastcall tachometer_is_name(Tachometer *, void *, const char *name) {
    return !std::strncmp(name, "HudTachometer", 14) ||
        !std::strncmp(name, "MNeedle", 8) ||
        !std::strncmp(name, "GWidget", 8) ||
        !std::strncmp(name, "GUI", 4);
}

const std::array<Method, 47> tachometer_methods = {
    reinterpret_cast<Method>(::nfs::application::frontend::hud::delete_tachometer), // 0.
    reinterpret_cast<Method>(::nfs::application::frontend::hud::tachometer_name), // 1.
    reinterpret_cast<Method>(::nfs::application::frontend::hud::tachometer_is_type), // 2.
    reinterpret_cast<Method>(::nfs::application::frontend::hud::tachometer_is_name), // 3.
    reinterpret_cast<Method>(::nfs::application::frontend::hud::load_tachometer), // 4.
    reinterpret_cast<Method>(::nfs::application::frontend::hud::save_tachometer), // 5.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::layout), // 6.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_text), // 7.
    reinterpret_cast<Method>(::nfs::gui::widgets::text), // 8.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::hit_test), // 9.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::navigate), // 10.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::enter_modal), // 11.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::leave_modal), // 12.
    reinterpret_cast<Method>(::nfs::application::frontend::hud::tachometer_event), // 13.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_four_arguments), // 14.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_two_arguments), // 15.
    reinterpret_cast<Method>(::nfs::gui::widgets::apply_drawing_state), // 16.
    reinterpret_cast<Method>(::nfs::gui::widgets::prepare_draw), // 17.
    reinterpret_cast<Method>(::nfs::gui::widgets::push_draw_bounds), // 18.
    reinterpret_cast<Method>(::nfs::application::frontend::hud::draw_needle), // 19.
    reinterpret_cast<Method>(::nfs::gui::widgets::pop_draw_bounds), // 20.
    reinterpret_cast<Method>(::nfs::application::frontend::hud::initialize_tachometer), // 21.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_no_arguments), // 22.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_render_argument), // 23.
    reinterpret_cast<Method>(::nfs::application::frontend::hud::tick_tachometer), // 24.
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

void *create_tachometer(void *parent) {
    auto *widget = static_cast<Tachometer *>(gui::allocate_flags(sizeof(Tachometer), 0, "HudTachometer"));
    if (!widget) return nullptr;
    try { return construct_tachometer(widget, parent); }
    catch (...) { gui::release_flags(widget, 0, "HudTachometer"); throw; }
}
}
