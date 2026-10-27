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
using game::runtime::invoke;
using game::runtime::system;
struct ScreenNavigation {
    family::Button base;
    gui::String next_screen, next_class;
};
static_assert(sizeof(ScreenNavigation) == 352);
extern const std::array<Method, 52> navigation_methods;

static const char *__cdecl navigation_class_key() {
    return "CmnScreenNav";
}

static const char *__fastcall navigation_name(Widget *, void *) {
    return "CmnScreenNav";
}

static Dword __fastcall navigation_is_type(Widget *widget, void *, const void *type) {
    return type == reinterpret_cast<const void *>(navigation_class_key) ||
           reinterpret_cast<Dword(__thiscall *)(Widget *, const void *)>(family::button_methods[2])(
               widget, type);
}

static Dword __fastcall navigation_is_name(Widget *, void *, const char *name) {
    return !std::strncmp(name, "CmnScreenNav", 13) || !std::strncmp(name, "GButton", 8) ||
           !std::strncmp(name, "GWidget", 8) || !std::strncmp(name, "GUI", 4);
}

static ScreenNavigation *construct_navigation(ScreenNavigation *widget, void *parent) {
    gui::widgets::pending::construct_button(widget, parent);
    try {
        gui::string_construct_copy(&widget->next_screen,
                                   &lifecycle::construction::unresolved::shared_string);
        try {
            gui::string_construct(&widget->next_class);
            widget->base.base.base.methods = navigation_methods.data();
        } catch (...) {
            gui::string_destruct(&widget->next_screen);
            throw;
        }
    } catch (...) {
        family::button_destroy(&widget->base.base.base);
        throw;
    }
    return widget;
}

static void destroy_navigation(ScreenNavigation *widget) {
    widget->base.base.base.methods = navigation_methods.data();
    gui::string_destruct(&widget->next_class);
    gui::string_destruct(&widget->next_screen);
    family::button_destroy(&widget->base.base.base);
}

static ScreenNavigation *__fastcall delete_navigation(ScreenNavigation *widget, void *,
                                                      Dword flags) {
    destroy_navigation(widget);
    if (flags & 1)
        gui::release(widget);
    return widget;
}

static void __fastcall activate_navigation(ScreenNavigation *widget, void *) {
    auto *button = &widget->base.base.base;
    family::activate_button(button, nullptr);
    if (!gui::string_data(&widget->next_screen))
        return;
    auto *root = application::pending::widget_root(button);
    if (root && invoke<Dword>(root, 2, interface_class_name)) {
        application::pending::LocalFrontendString type(&widget->next_class),
            name(&widget->next_screen);
        application::pending::enqueue_screen(system(96), name.release(), type.release());
        family::finish_modal(root, 4, 0);
    }
}

static Dword __fastcall load_navigation(ScreenNavigation *widget, void *, void *configuration) {
    const auto *empty = &lifecycle::construction::unresolved::shared_string;
    gui::engine::pending::read_reference_string(configuration, "CmnScreenNav.mNextScreen",
                                                &widget->next_screen, empty);
    gui::engine::pending::read_reference_string(configuration, "CmnScreenNav.mNextScreenClass",
                                                &widget->next_class, empty);
    return reinterpret_cast<Dword(__thiscall *)(void *, void *)>(family::button_methods[4])(
        widget, configuration);
}

static Dword __fastcall save_navigation(ScreenNavigation *widget, void *, void *configuration) {
    const auto *empty = &lifecycle::construction::unresolved::shared_string;
    gui::engine::pending::write_reference_string(configuration, "CmnScreenNav.mNextScreen",
                                                 &widget->next_screen, empty);
    gui::engine::pending::write_reference_string(configuration, "CmnScreenNav.mNextScreenClass",
                                                 &widget->next_class, empty);
    return reinterpret_cast<Dword(__thiscall *)(void *, void *)>(family::button_methods[5])(
        widget, configuration);
}

const std::array<Method, 52> navigation_methods = {
    reinterpret_cast<Method>(::nfs::application::frontend::delete_navigation), // 0.
    reinterpret_cast<Method>(::nfs::application::frontend::navigation_name), // 1.
    reinterpret_cast<Method>(::nfs::application::frontend::navigation_is_type), // 2.
    reinterpret_cast<Method>(::nfs::application::frontend::navigation_is_name), // 3.
    reinterpret_cast<Method>(::nfs::application::frontend::load_navigation), // 4.
    reinterpret_cast<Method>(::nfs::application::frontend::save_navigation), // 5.
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
    reinterpret_cast<Method>(::nfs::application::frontend::activate_navigation), // 49.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::button_item_operation_50), // 50.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::button_set_checked_event), // 51.
};

static void *create_navigation(void *parent) {
    auto *storage = static_cast<ScreenNavigation *>(
        gui::allocate_flags(sizeof(ScreenNavigation), 0, "CmnScreenNav"));
    if (!storage)
        return nullptr;
    try {
        return construct_navigation(storage, parent);
    } catch (...) {
        gui::release(storage);
        throw;
    }
}
static ClassRecord navigation_class;

static void unregister_navigation() {
    unregister_class(&navigation_class);
}

static void register_navigation() {
    register_class(&navigation_class, "CmnScreenNav", create_navigation, "GButton");
    std::atexit(unregister_navigation);
}
namespace {
NFS_KERNEL_INITIALIZER(startup_register_navigation, ".CRT$XCU_NFS_00690128",
                       register_navigation);
} // namespace
} // namespace nfs::application::frontend
