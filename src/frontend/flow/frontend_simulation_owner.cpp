#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "../../gui/widgets/gui_widget_shared_methods.hpp"
#include "frontend_simulation.hpp"
#include <cstdlib>
#include <cstring>
namespace nfs::application::frontend {
using namespace gui::widgets;
void __fastcall initialize_simulation(Widget *, void *);
void __fastcall release_simulation(Widget *, void *);
void __fastcall tick_simulation(Widget *, void *, Dword);
void __fastcall draw_simulation(Widget *, void *, void *);
Dword __fastcall simulation_key_down(Widget *, void *, Dword, Dword);
Dword __fastcall simulation_key_up(Widget *, void *, Dword, Dword);
Dword __fastcall simulation_event(Widget *, void *, Dword, Dword, Dword);

static void destroy_simulation(Widget *screen) {
    screen->methods = simulation_methods.data();
    destroy_interface(screen);
}

static Widget *__fastcall delete_simulation(Widget *screen, void *, Dword flags) {
    destroy_simulation(screen);
    if (flags & 1)
        gui::release(screen);
    return screen;
}

const char *__cdecl simulation_class_key() {
    return "SimGUIInterface";
}

static const char *__fastcall simulation_name(Widget *, void *) {
    return "SimGUIInterface";
}

static Dword __fastcall simulation_is_type(Widget *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(simulation_class_key) ||
           type == reinterpret_cast<const void *>(interface_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

static Dword __fastcall simulation_is_name(Widget *, void *, const char *name) {
    return !std::strncmp(name, "SimGUIInterface", 16) || !std::strncmp(name, "GInterface", 11) ||
           !std::strncmp(name, "GUI", 4);
}

const std::array<Method, 42> simulation_methods = {
    reinterpret_cast<Method>(::nfs::application::frontend::delete_simulation), // 0.
    reinterpret_cast<Method>(::nfs::application::frontend::simulation_name), // 1.
    reinterpret_cast<Method>(::nfs::application::frontend::simulation_is_type), // 2.
    reinterpret_cast<Method>(::nfs::application::frontend::simulation_is_name), // 3.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::load_interface), // 4.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::save_interface), // 5.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::layout), // 6.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_text), // 7.
    reinterpret_cast<Method>(::nfs::gui::widgets::text), // 8.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::hit_test), // 9.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::navigate), // 10.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::enter_interface), // 11.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::leave_modal), // 12.
    reinterpret_cast<Method>(::nfs::application::frontend::simulation_event), // 13.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_four_arguments), // 14.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_two_arguments), // 15.
    reinterpret_cast<Method>(::nfs::gui::widgets::apply_drawing_state), // 16.
    reinterpret_cast<Method>(::nfs::gui::widgets::prepare_draw), // 17.
    reinterpret_cast<Method>(::nfs::gui::widgets::push_draw_bounds), // 18.
    reinterpret_cast<Method>(::nfs::application::frontend::draw_simulation), // 19.
    reinterpret_cast<Method>(::nfs::gui::widgets::pop_draw_bounds), // 20.
    reinterpret_cast<Method>(::nfs::application::frontend::initialize_simulation), // 21.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_no_arguments), // 22.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_render_argument), // 23.
    reinterpret_cast<Method>(::nfs::application::frontend::tick_simulation), // 24.
    reinterpret_cast<Method>(::nfs::application::frontend::release_simulation), // 25.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_layout_argument), // 26.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_parent_enter), // 27.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_parent_leave), // 28.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_parent_argument), // 29.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_child_argument), // 30.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_modal_state), // 31.
    reinterpret_cast<Method>(::nfs::application::frontend::simulation_key_down), // 32.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_result), // 33.
    reinterpret_cast<Method>(::nfs::application::frontend::simulation_key_up), // 34.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_35), // 35.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::interface_button), // 36.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_37), // 37.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_38), // 38.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::update_active_style), // 39.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::update_focus_style), // 40.
    reinterpret_cast<Method>(::nfs::gui::widgets::capture_style), // 41.
};

void *create_simulation_screen(void *parent) {
    auto *screen = static_cast<Widget *>(gui::allocate_flags(256, 0, "SimGUIInterface"));
    if (!screen)
        return nullptr;
    try {
        gui::widgets::pending::construct_interface(screen, parent);
        screen->methods = simulation_methods.data();
        return screen;
    } catch (...) {
        gui::release(screen);
        throw;
    }
}
static ClassRecord simulation_class;

static void unregister_simulation() {
    unregister_class(&simulation_class);
}

static void register_simulation() {
    register_class(&simulation_class, "SimGUIInterface", create_simulation_screen, "GInterface");
    std::atexit(unregister_simulation);
}
namespace {
NFS_KERNEL_INITIALIZER(startup_register_simulation, ".CRT$XCU_NFS_0069012C",
                       register_simulation);
}
} // namespace nfs::application::frontend
