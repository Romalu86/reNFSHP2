#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "../../gui/widgets/gui_widget_shared_methods.hpp"
#include "../flow/frontend_application_widgets.hpp"
#include "../flow/frontend_runtime.hpp"
#include "../../gui/lifecycle/gui_configuration_cache.hpp"
#include "../../gui/widgets/gui_widget_settings.hpp"
#include "../../gui/lifecycle/gui_input.hpp"
#include "nfs/network/session/network_runtime.hpp"
#include <cstdlib>
#include <cstring>
namespace nfs::gui::input {
int input_idle_ticks();
}
namespace nfs::gui::engine {
int seconds_to_ticks(float);
}
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::at;
using game::runtime::invoke;
using game::runtime::system;
struct MainMenu {
    Widget base;
    std::uint8_t another_instance, padding[3];
};
static_assert(sizeof(MainMenu) == 260);
static Dword last_focused_name{};
extern const std::array<Method, 42> main_menu_methods;

static const char *__cdecl main_menu_class_key() {
    return "MainMenu";
}

static const char *__fastcall main_menu_name(Widget *, void *) {
    return "MainMenu";
}

static Dword __fastcall main_menu_is_type(Widget *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(main_menu_class_key) ||
           type == reinterpret_cast<const void *>(interface_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

static Dword __fastcall main_menu_is_name(Widget *, void *, const char *name) {
    return !std::strncmp(name, "MainMenu", 9) || !std::strncmp(name, "GInterface", 11) ||
           !std::strncmp(name, "GUI", 4);
}

static void destroy_main_menu(Widget *widget) {
    widget->methods = main_menu_methods.data();
    destroy_interface(widget);
}

static Widget *__fastcall delete_main_menu(Widget *widget, void *, Dword flags) {
    destroy_main_menu(widget);
    if (flags & 1)
        gui::release(widget);
    return widget;
}

static Dword __fastcall main_menu_result(Widget *widget, void *, Dword result, Dword) {
    auto *focused =
        static_cast<Widget *>(gui::events::find_widget(gui::context_state, active_widget_id));
    if (focused)
        gui::engine::pending::hash_name(&last_focused_name, gui::string_ascii(&focused->name));
    widget->modal_result = result;
    return 1;
}

static Dword __fastcall enter_main_menu(Widget *widget, void *) {
    if (!gui::widgets::pending::enter_interface(widget, nullptr))
        return 0;
    if (last_focused_name)
        for (int index = 0; index < int(widget->children.count); ++index)
            if (Widget *child = child_at(widget, index, 0)) {
                Dword hash;
                gui::engine::pending::hash_name(&hash, gui::string_ascii(&child->name));
                if (hash == last_focused_name) {
                    activate(child);
                    return 1;
                }
            }
    return 1;
}

static void __fastcall initialize_main_menu(MainMenu *widget, void *) {
    gui::widgets::pending::request_interface_layout(&widget->base, nullptr);
    void *rules = system(72);
    at<Dword>(rules, 204) = 0;
    at<Dword>(rules, 208) = Dword(-1);
    if (!network::has_instance_mutex() && !network::acquire_instance_mutex())
        widget->another_instance = 1;
}

static void __fastcall update_main_menu(MainMenu *widget, void *, Dword) {
    if (widget->another_instance) {
        application::pending::clear_screen_queue(system(96));
        family::finish_modal(&widget->base, 6, 0);
        return;
    }
    if (gui::events::is_within_focus(widget) && !widget->base.modal_result) {
        const int timeout = gui::engine::seconds_to_ticks(60.0f);
        if (gui::input::input_idle_ticks() > timeout) {
            application::pending::clear_screen_queue(system(96));
            family::finish_modal(&widget->base, 8, 0);
        }
    }
}

const std::array<Method, 42> main_menu_methods = {
    reinterpret_cast<Method>(::nfs::application::frontend::delete_main_menu), // 0.
    reinterpret_cast<Method>(::nfs::application::frontend::main_menu_name), // 1.
    reinterpret_cast<Method>(::nfs::application::frontend::main_menu_is_type), // 2.
    reinterpret_cast<Method>(::nfs::application::frontend::main_menu_is_name), // 3.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::load_interface), // 4.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::save_interface), // 5.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::layout), // 6.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_text), // 7.
    reinterpret_cast<Method>(::nfs::gui::widgets::text), // 8.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::hit_test), // 9.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::navigate), // 10.
    reinterpret_cast<Method>(::nfs::application::frontend::enter_main_menu), // 11.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::leave_modal), // 12.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::handle_event), // 13.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_four_arguments), // 14.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_two_arguments), // 15.
    reinterpret_cast<Method>(::nfs::gui::widgets::apply_drawing_state), // 16.
    reinterpret_cast<Method>(::nfs::gui::widgets::prepare_draw), // 17.
    reinterpret_cast<Method>(::nfs::gui::widgets::push_draw_bounds), // 18.
    reinterpret_cast<Method>(::nfs::gui::widgets::draw_content), // 19.
    reinterpret_cast<Method>(::nfs::gui::widgets::pop_draw_bounds), // 20.
    reinterpret_cast<Method>(::nfs::application::frontend::initialize_main_menu), // 21.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_no_arguments), // 22.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_render_argument), // 23.
    reinterpret_cast<Method>(::nfs::application::frontend::update_main_menu), // 24.
    reinterpret_cast<Method>(::nfs::gui::widgets::clear_layout_request), // 25.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_layout_argument), // 26.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_parent_enter), // 27.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_parent_leave), // 28.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_parent_argument), // 29.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_child_argument), // 30.
    reinterpret_cast<Method>(::nfs::application::frontend::main_menu_result), // 31.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::interface_key), // 32.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_result), // 33.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_34), // 34.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_35), // 35.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::interface_button), // 36.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_37), // 37.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_38), // 38.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::update_active_style), // 39.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::update_focus_style), // 40.
    reinterpret_cast<Method>(::nfs::gui::widgets::capture_style), // 41.
};

static void *create_main_menu(void *parent) {
    auto *widget = static_cast<MainMenu *>(gui::allocate_flags(sizeof(MainMenu), 0, "MainMenu"));
    if (!widget)
        return nullptr;
    try {
        gui::widgets::pending::construct_interface(widget, parent);
        try {
            widget->another_instance = 0;
            widget->base.methods = main_menu_methods.data();
            gui::reset_registry_time();

        } catch (...) {
            destroy_interface(&widget->base);
            throw;
        }
    } catch (...) {
        gui::release(widget);
        throw;
    }
    return widget;
}
static ClassRecord main_menu_class;

static void unregister_main_menu() {
    unregister_class(&main_menu_class);
}

static void register_main_menu() {
    register_class(&main_menu_class, "MainMenu", create_main_menu, "GInterface");
    std::atexit(unregister_main_menu);
}
namespace {
NFS_KERNEL_INITIALIZER(startup_register_main_menu_callbacks, ".CRT$XCU_NFS_006900D4",
                       register_main_menu_callbacks);
NFS_KERNEL_INITIALIZER(startup_register_main_menu, ".CRT$XCU_NFS_006900D8",
                       register_main_menu);
} // namespace
} // namespace nfs::application::frontend
