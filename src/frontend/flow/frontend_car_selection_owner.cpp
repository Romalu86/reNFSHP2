#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "../../gui/widgets/gui_widget_shared_methods.hpp"
#include "../screens/frontend_menu_roots.hpp"
#include <cstdlib>
#include <cstring>
namespace nfs::application::frontend {
using namespace gui::widgets;

const char *__cdecl car_selection_class_key() {
    return "CarSelectScreen";
}

static const char *__fastcall car_selection_name(Widget *, void *) {
    return "CarSelectScreen";
}

static Dword __fastcall car_selection_is_type(Widget *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(car_selection_class_key) ||
           type == reinterpret_cast<const void *>(interface_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

static Dword __fastcall car_selection_is_name(Widget *, void *, const char *name) {
    return !std::strncmp(name, "CarSelectScreen", 16) || !std::strncmp(name, "GInterface", 11) ||
           !std::strncmp(name, "GUI", 4);
}

static CarSelection *__fastcall delete_car_selection(CarSelection *page, void *, Dword flags) {
    destroy_car_selection(page);
    if (flags & 1)
        gui::release(page);
    return page;
}

const std::array<Method, 42> car_selection_methods = {
    reinterpret_cast<Method>(::nfs::application::frontend::delete_car_selection), // 0.
    reinterpret_cast<Method>(::nfs::application::frontend::car_selection_name), // 1.
    reinterpret_cast<Method>(::nfs::application::frontend::car_selection_is_type), // 2.
    reinterpret_cast<Method>(::nfs::application::frontend::car_selection_is_name), // 3.
    reinterpret_cast<Method>(::nfs::application::frontend::load_car_selection), // 4.
    reinterpret_cast<Method>(::nfs::application::frontend::save_car_selection), // 5.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::layout), // 6.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_text), // 7.
    reinterpret_cast<Method>(::nfs::gui::widgets::text), // 8.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::hit_test), // 9.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::navigate), // 10.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::enter_interface), // 11.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::leave_modal), // 12.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::handle_event), // 13.
    reinterpret_cast<Method>(::nfs::application::frontend::car_selection_child), // 14.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_two_arguments), // 15.
    reinterpret_cast<Method>(::nfs::gui::widgets::apply_drawing_state), // 16.
    reinterpret_cast<Method>(::nfs::gui::widgets::prepare_draw), // 17.
    reinterpret_cast<Method>(::nfs::gui::widgets::push_draw_bounds), // 18.
    reinterpret_cast<Method>(::nfs::gui::widgets::draw_content), // 19.
    reinterpret_cast<Method>(::nfs::gui::widgets::pop_draw_bounds), // 20.
    reinterpret_cast<Method>(::nfs::application::frontend::initialize_car_selection), // 21.
    reinterpret_cast<Method>(::nfs::application::frontend::resume_car_selection), // 22.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_render_argument), // 23.
    reinterpret_cast<Method>(::nfs::application::frontend::update_car_selection), // 24.
    reinterpret_cast<Method>(::nfs::gui::widgets::clear_layout_request), // 25.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_layout_argument), // 26.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_parent_enter), // 27.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_parent_leave), // 28.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_parent_argument), // 29.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_child_argument), // 30.
    reinterpret_cast<Method>(::nfs::application::frontend::finish_car_selection), // 31.
    reinterpret_cast<Method>(::nfs::application::frontend::car_selection_key), // 32.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_result), // 33.
    reinterpret_cast<Method>(::nfs::application::frontend::car_selection_key_up), // 34.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_35), // 35.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::interface_button), // 36.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_37), // 37.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_38), // 38.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::update_active_style), // 39.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::update_focus_style), // 40.
    reinterpret_cast<Method>(::nfs::gui::widgets::capture_style), // 41.
};

void *create_car_selection(void *parent) {
    auto *page = static_cast<CarSelection *>(
        gui::allocate_flags(sizeof(CarSelection), 0, "CarSelectScreen"));
    if (!page)
        return nullptr;
    try {
        construct_car_selection(page, parent);
        reinterpret_cast<Widget *>(page)->methods = car_selection_methods.data();
        return page;
    } catch (...) {
        gui::release(page);
        throw;
    }
}
static ClassRecord car_selection_class;

static void unregister_car_selection() {
    unregister_class(&car_selection_class);
}

static void register_car_selection() {
    register_class(&car_selection_class, "CarSelectScreen", create_car_selection, "GInterface");
    std::atexit(unregister_car_selection);
}
namespace {
NFS_KERNEL_INITIALIZER(startup_register_car_selection, ".CRT$XCU_NFS_0069004C",
                       register_car_selection);
} // namespace
} // namespace nfs::application::frontend
