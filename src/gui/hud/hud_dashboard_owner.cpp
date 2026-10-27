#include "nfs/core/lifecycle/scope_cleanup.hpp"
#include "../widgets/gui_widget_shared_methods.hpp"
#include "hud_instruments.hpp"
#include <cstring>
namespace nfs::application::frontend::hud {
using namespace gui::widgets;
static Dashboard *__fastcall delete_dashboard(Dashboard *, void *, Dword);

static Dashboard *__fastcall delete_dashboard_binding(Binding *binding, void *, Dword flags) {
    return delete_dashboard(reinterpret_cast<Dashboard *>(reinterpret_cast<Byte *>(binding) - 264), nullptr, flags);
}
static const Method dashboard_binding_methods[1] = {reinterpret_cast<Method>(delete_dashboard_binding)};

static Dashboard *construct_dashboard(Dashboard *widget, void *parent) {
    construct_widget(widget, parent);
    kernel::lifecycle::Cleanup base_cleanup([=] { destroy_general(&widget->base); });
    construct_binding(&widget->binding, &widget->base.base);
    kernel::lifecycle::Cleanup binding_cleanup([=] { destroy_binding(&widget->binding); });
    gui::string_construct(&widget->model);
    kernel::lifecycle::Cleanup model_cleanup([=] { gui::string_destruct(&widget->model); });
    widget->base.base.methods = dashboard_methods.data();
    widget->binding.methods = dashboard_binding_methods;
    model_cleanup.dismiss();
    binding_cleanup.dismiss();
    base_cleanup.dismiss();
    return widget;
}

static void destroy_dashboard(Dashboard *widget) {
    widget->base.base.methods = dashboard_methods.data();
    widget->binding.methods = dashboard_binding_methods;
    kernel::lifecycle::Cleanup base_cleanup([=] { destroy_general(&widget->base); });
    kernel::lifecycle::Cleanup binding_cleanup([=] { destroy_binding(&widget->binding); });
    gui::string_destruct(&widget->model);
}

static Dashboard *__fastcall delete_dashboard(Dashboard *widget, void *, Dword flags) {
    destroy_dashboard(widget);
    if (flags & 1) gui::release(widget);
    return widget;
}

const char *__cdecl dashboard_class_key() { return "HudDashboard"; }

static const char *__fastcall dashboard_name(Dashboard *, void *) { return "HudDashboard"; }

static Dword __fastcall dashboard_is_type(Dashboard *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(dashboard_class_key) ||
        type == reinterpret_cast<const void *>(general_class_name) ||
        type == reinterpret_cast<const void *>(base_class_name);
}

static Dword __fastcall dashboard_is_name(Dashboard *, void *, const char *name) {
    return !std::strncmp(name, "HudDashboard", 13) ||
        !std::strncmp(name, "GWidget", 8) ||
        !std::strncmp(name, "GUI", 4);
}

const std::array<Method, 47> dashboard_methods = {
    reinterpret_cast<Method>(::nfs::application::frontend::hud::delete_dashboard), // 0.
    reinterpret_cast<Method>(::nfs::application::frontend::hud::dashboard_name), // 1.
    reinterpret_cast<Method>(::nfs::application::frontend::hud::dashboard_is_type), // 2.
    reinterpret_cast<Method>(::nfs::application::frontend::hud::dashboard_is_name), // 3.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::load_general), // 4.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::save_general), // 5.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::layout), // 6.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_text), // 7.
    reinterpret_cast<Method>(::nfs::gui::widgets::text), // 8.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::hit_test), // 9.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::navigate), // 10.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::enter_modal), // 11.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::leave_modal), // 12.
    reinterpret_cast<Method>(::nfs::application::frontend::hud::dashboard_event), // 13.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_four_arguments), // 14.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_two_arguments), // 15.
    reinterpret_cast<Method>(::nfs::gui::widgets::apply_drawing_state), // 16.
    reinterpret_cast<Method>(::nfs::gui::widgets::prepare_draw), // 17.
    reinterpret_cast<Method>(::nfs::gui::widgets::push_draw_bounds), // 18.
    reinterpret_cast<Method>(::nfs::gui::widgets::draw_content), // 19.
    reinterpret_cast<Method>(::nfs::gui::widgets::pop_draw_bounds), // 20.
    reinterpret_cast<Method>(::nfs::application::frontend::hud::initialize_dashboard), // 21.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_no_arguments), // 22.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_render_argument), // 23.
    reinterpret_cast<Method>(::nfs::application::frontend::hud::tick_dashboard), // 24.
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

void *create_dashboard(void *parent) {
    auto *widget = static_cast<Dashboard *>(gui::allocate_flags(sizeof(Dashboard), 0, "HudDashboard"));
    if (!widget) return nullptr;
    try { return construct_dashboard(widget, parent); }
    catch (...) { gui::release_flags(widget, 0, "HudDashboard"); throw; }
}
}
