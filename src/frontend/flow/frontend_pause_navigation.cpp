#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "../../gui/widgets/gui_widget_shared_methods.hpp"
#include "frontend_application_widgets.hpp"
#include "frontend_runtime.hpp"
#include "../../gui/widgets/gui_widget_controls.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include <cstdlib>
#include <cstring>
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::at;
using game::runtime::system;
struct PauseNavigation {
    family::Button base;
    gui::String next_menu;
};
static_assert(sizeof(PauseNavigation) == 332 && offsetof(PauseNavigation, next_menu) == 312);
extern const std::array<Method, 52> pause_navigation_methods;

static PauseNavigation *construct_pause_navigation(PauseNavigation *widget, void *parent) {
    gui::widgets::pending::construct_button(widget, parent);
    try {
        gui::string_construct_copy(&widget->next_menu,
                                   &lifecycle::construction::unresolved::shared_string);
    } catch (...) {
        family::button_destroy(&widget->base.base.base);
        throw;
    }
    widget->base.base.base.methods = pause_navigation_methods.data();
    return widget;
}

static void destroy_pause_navigation(PauseNavigation *widget) {
    widget->base.base.base.methods = pause_navigation_methods.data();
    gui::string_destruct(&widget->next_menu);
    family::button_destroy(&widget->base.base.base);
}

static PauseNavigation *__fastcall delete_pause_navigation(PauseNavigation *widget, void *,
                                                           Dword flags) {
    destroy_pause_navigation(widget);
    if (flags & 1)
        gui::release(widget);
    return widget;
}

static const char *__cdecl pause_navigation_class_key() {
    return "CmnPauseNav";
}

static const char *__fastcall pause_navigation_name(PauseNavigation *, void *) {
    return "CmnPauseNav";
}

static Dword __fastcall pause_navigation_is_type(PauseNavigation *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(pause_navigation_class_key) ||
           type == reinterpret_cast<const void *>(family::button_class_key) ||
           type == reinterpret_cast<const void *>(general_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

static Dword __fastcall pause_navigation_is_name(PauseNavigation *, void *, const char *name) {
    return !std::strncmp(name, "CmnPauseNav", 12) || !std::strncmp(name, "GButton", 8) ||
           !std::strncmp(name, "GWidget", 8) || !std::strncmp(name, "GUI", 4);
}

static void __fastcall activate_pause_navigation(PauseNavigation *widget, void *) {
    auto *button = &widget->base.base.base;
    family::activate_button(button, nullptr);
    if (gui::string_data(&widget->next_menu)) {
        gui::string_copy(&at<gui::String>(system(96), 16), &widget->next_menu);
        if (button->parent)
            family::finish_modal(button->parent, 4, 0);
    }
}

static Dword __fastcall load_pause_navigation(PauseNavigation *widget, void *, void *configuration) {
    gui::engine::pending::read_string(configuration, "CmnPauseNav.mNextPauseMenu",
                                      &widget->next_menu,
                                      &lifecycle::construction::unresolved::shared_string);
    return family::button_load(&widget->base.base.base, nullptr,
                               reinterpret_cast<Dword>(configuration));
}

static Dword __fastcall save_pause_navigation(PauseNavigation *widget, void *, void *configuration) {
    gui::engine::pending::write_text(configuration, "CmnPauseNav.mNextPauseMenu",
                                     gui::string_ascii(&widget->next_menu), nullptr);
    return family::button_save(&widget->base.base.base, nullptr,
                               reinterpret_cast<Dword>(configuration));
}

const std::array<Method, 52> pause_navigation_methods = {
    reinterpret_cast<Method>(::nfs::application::frontend::delete_pause_navigation), // 0.
    reinterpret_cast<Method>(::nfs::application::frontend::pause_navigation_name), // 1.
    reinterpret_cast<Method>(::nfs::application::frontend::pause_navigation_is_type), // 2.
    reinterpret_cast<Method>(::nfs::application::frontend::pause_navigation_is_name), // 3.
    reinterpret_cast<Method>(::nfs::application::frontend::load_pause_navigation), // 4.
    reinterpret_cast<Method>(::nfs::application::frontend::save_pause_navigation), // 5.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::layout), // 6.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::button_set_text), // 7.
    reinterpret_cast<Method>(::nfs::gui::widgets::text), // 8.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::hit_test), // 9.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::navigate), // 10.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::enter_modal), // 11.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::leave_modal), // 12.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::button_event), // 13.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_four_arguments), // 14.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_two_arguments), // 15.
    reinterpret_cast<Method>(::nfs::gui::widgets::apply_drawing_state), // 16.
    reinterpret_cast<Method>(::nfs::gui::widgets::prepare_draw), // 17.
    reinterpret_cast<Method>(::nfs::gui::widgets::push_draw_bounds), // 18.
    reinterpret_cast<Method>(::nfs::gui::widgets::draw_content), // 19.
    reinterpret_cast<Method>(::nfs::gui::widgets::pop_draw_bounds), // 20.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::button_initialize), // 21.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_no_arguments), // 22.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_render_argument), // 23.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_style_argument), // 24.
    reinterpret_cast<Method>(::nfs::gui::widgets::clear_layout_request), // 25.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_layout_argument), // 26.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::button_parent_enter), // 27.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_parent_leave), // 28.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_parent_argument), // 29.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_child_argument), // 30.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_modal_state), // 31.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::button_key_down), // 32.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_result), // 33.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_34), // 34.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_35), // 35.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::button_button_down), // 36.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_37), // 37.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_38), // 38.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::update_active_style), // 39.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::button_pointer_press), // 40.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::button_pointer_release), // 41.
    reinterpret_cast<Method>(::nfs::gui::widgets::accept_general), // 42.
    reinterpret_cast<Method>(::nfs::gui::widgets::cancel_general), // 43.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_scroll), // 44.
    reinterpret_cast<Method>(::nfs::gui::widgets::process_general_command), // 45.
    reinterpret_cast<Method>(::nfs::gui::widgets::process_general_notification), // 46.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::set_button_checked), // 47.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::button_visual_state), // 48.
    reinterpret_cast<Method>(::nfs::application::frontend::activate_pause_navigation), // 49.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::button_item_operation_50), // 50.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::button_set_checked_event), // 51.
};

static void *create_pause_navigation(void *parent) {
    auto *widget = static_cast<PauseNavigation *>(
        gui::allocate_flags(sizeof(PauseNavigation), 0, "CmnPauseNav"));
    if (!widget)
        return nullptr;
    try {
        return construct_pause_navigation(widget, parent);
    } catch (...) {
        gui::release(widget);
        throw;
    }
}
static ClassRecord pause_navigation_class;

static void unregister_pause_navigation() {
    unregister_class(&pause_navigation_class);
}

static void register_pause_navigation() {
    register_class(&pause_navigation_class, "CmnPauseNav", create_pause_navigation, "GButton");
    std::atexit(unregister_pause_navigation);
}
namespace {
NFS_KERNEL_INITIALIZER(startup_register_pause_navigation, ".CRT$XCU_NFS_0069005C",
                       register_pause_navigation);
} // namespace
} // namespace nfs::application::frontend
