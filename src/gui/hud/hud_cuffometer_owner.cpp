#include "../widgets/gui_widget_shared_methods.hpp"
#include "hud_display.hpp"
#include <cstring>
namespace nfs::application::frontend::hud {
using namespace gui::widgets;
static Cuffometer *__fastcall delete_cuffometer(Cuffometer *, void *, Dword);

static Cuffometer *__fastcall delete_cuffometer_binding(Binding *binding, void *, Dword flags) {
    return delete_cuffometer(
        reinterpret_cast<Cuffometer *>(reinterpret_cast<Byte *>(binding) - 264), nullptr, flags);
}
static const Method cuffometer_binding_methods[1] = {
    reinterpret_cast<Method>(delete_cuffometer_binding)};

static Cuffometer *construct_cuffometer(Cuffometer *widget, void *parent) {
    construct_widget(widget, parent);
    construct_binding(&widget->binding, &widget->base.base);
    widget->active = 0;
    widget->state = 0;
    construct_progress_pair(&widget->progress);
    gui::engine::construct_point(&widget->position);
    widget->chase_state = Dword(-1);
    widget->base.base.methods = cuffometer_methods.data();
    widget->binding.methods = cuffometer_binding_methods;
    widget->stars[0] = widget->stars[1] = widget->stars[2] = widget->stars[3] = nullptr;
    return widget;
}

static void destroy_cuffometer(Cuffometer *widget) {
    widget->base.base.methods = cuffometer_methods.data();
    widget->binding.methods = cuffometer_binding_methods;
    gui::engine::destroy_point(&widget->position);
    construct_progress_pair(&widget->progress);
    destroy_binding(&widget->binding);
    destroy_general(&widget->base);
}

static Cuffometer *__fastcall delete_cuffometer(Cuffometer *widget, void *, Dword flags) {
    destroy_cuffometer(widget);
    if (flags & 1)
        gui::release(widget);
    return widget;
}

const char *__cdecl cuffometer_class_key() {
    return "WCuffometer";
}

static const char *__fastcall cuffometer_name(Cuffometer *, void *) {
    return "WCuffometer";
}

static Dword __fastcall cuffometer_is_type(Cuffometer *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(cuffometer_class_key) ||
           type == reinterpret_cast<const void *>(general_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

static Dword __fastcall cuffometer_is_name(Cuffometer *, void *, const char *name) {
    return !std::strncmp(name, "WCuffometer", 12) || !std::strncmp(name, "GWidget", 8) ||
           !std::strncmp(name, "GUI", 4);
}

const std::array<Method, 47> cuffometer_methods = {
    reinterpret_cast<Method>(::nfs::application::frontend::hud::delete_cuffometer), // 0.
    reinterpret_cast<Method>(::nfs::application::frontend::hud::cuffometer_name), // 1.
    reinterpret_cast<Method>(::nfs::application::frontend::hud::cuffometer_is_type), // 2.
    reinterpret_cast<Method>(::nfs::application::frontend::hud::cuffometer_is_name), // 3.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::load_general), // 4.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::save_general), // 5.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::layout), // 6.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_text), // 7.
    reinterpret_cast<Method>(::nfs::gui::widgets::text), // 8.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::hit_test), // 9.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::navigate), // 10.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::enter_modal), // 11.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::leave_modal), // 12.
    reinterpret_cast<Method>(::nfs::application::frontend::hud::cuffometer_event), // 13.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_four_arguments), // 14.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_two_arguments), // 15.
    reinterpret_cast<Method>(::nfs::gui::widgets::apply_drawing_state), // 16.
    reinterpret_cast<Method>(::nfs::application::frontend::hud::prepare_cuffometer), // 17.
    reinterpret_cast<Method>(::nfs::gui::widgets::push_draw_bounds), // 18.
    reinterpret_cast<Method>(::nfs::gui::widgets::draw_content), // 19.
    reinterpret_cast<Method>(::nfs::gui::widgets::pop_draw_bounds), // 20.
    reinterpret_cast<Method>(::nfs::application::frontend::hud::initialize_cuffometer), // 21.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_no_arguments), // 22.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_render_argument), // 23.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_style_argument), // 24.
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

void *create_cuffometer(void *parent) {
    auto *widget =
        static_cast<Cuffometer *>(gui::allocate_flags(sizeof(Cuffometer), 0, "WCuffometer"));
    if (!widget)
        return nullptr;
    try {
        return construct_cuffometer(widget, parent);
    } catch (...) {
        gui::release(widget);
        throw;
    }
}
} // namespace nfs::application::frontend::hud
