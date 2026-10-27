#include "../../gui/widgets/gui_widget_shared_methods.hpp"
#include "frontend_menu_roots.hpp"
#include <cstdlib>
#include <cstring>
namespace nfs::application::frontend {
using namespace gui::widgets;

const char *__cdecl tree_screen_class_key() {
    return "TreeScreen";
}

static const char *__fastcall tree_screen_name(Widget *, void *) {
    return "TreeScreen";
}

static Dword __fastcall tree_screen_is_type(Widget *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(tree_screen_class_key) ||
           type == reinterpret_cast<const void *>(interface_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

static Dword __fastcall tree_screen_is_name(Widget *, void *, const char *name) {
    return !std::strncmp(name, "TreeScreen", 11) || !std::strncmp(name, "GInterface", 11) ||
           !std::strncmp(name, "GUI", 4);
}

static TreeScreen *__fastcall delete_tree_screen(TreeScreen *page, void *, Dword flags) {
    destroy_tree_screen(page);
    if (flags & 1)
        gui::release(page);
    return page;
}

const std::array<Method, 42> tree_screen_methods = {
    reinterpret_cast<Method>(::nfs::application::frontend::delete_tree_screen), // 0.
    reinterpret_cast<Method>(::nfs::application::frontend::tree_screen_name), // 1.
    reinterpret_cast<Method>(::nfs::application::frontend::tree_screen_is_type), // 2.
    reinterpret_cast<Method>(::nfs::application::frontend::tree_screen_is_name), // 3.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::load_interface), // 4.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::save_interface), // 5.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::layout), // 6.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_text), // 7.
    reinterpret_cast<Method>(::nfs::gui::widgets::text), // 8.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::hit_test), // 9.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::navigate), // 10.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::enter_interface), // 11.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::leave_modal), // 12.
    reinterpret_cast<Method>(::nfs::application::frontend::tree_screen_event), // 13.
    reinterpret_cast<Method>(::nfs::application::frontend::tree_screen_child), // 14.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_two_arguments), // 15.
    reinterpret_cast<Method>(::nfs::gui::widgets::apply_drawing_state), // 16.
    reinterpret_cast<Method>(::nfs::gui::widgets::prepare_draw), // 17.
    reinterpret_cast<Method>(::nfs::gui::widgets::push_draw_bounds), // 18.
    reinterpret_cast<Method>(::nfs::gui::widgets::draw_content), // 19.
    reinterpret_cast<Method>(::nfs::gui::widgets::pop_draw_bounds), // 20.
    reinterpret_cast<Method>(::nfs::application::frontend::initialize_tree_screen), // 21.
    reinterpret_cast<Method>(::nfs::application::frontend::resume_tree_screen), // 22.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_render_argument), // 23.
    reinterpret_cast<Method>(::nfs::application::frontend::tick_tree_screen), // 24.
    reinterpret_cast<Method>(::nfs::gui::widgets::clear_layout_request), // 25.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_layout_argument), // 26.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_parent_enter), // 27.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_parent_leave), // 28.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_parent_argument), // 29.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_child_argument), // 30.
    reinterpret_cast<Method>(::nfs::application::frontend::finish_tree_screen), // 31.
    reinterpret_cast<Method>(::nfs::application::frontend::tree_screen_key), // 32.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_result), // 33.
    reinterpret_cast<Method>(::nfs::application::frontend::tree_screen_key_up), // 34.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_35), // 35.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::interface_button), // 36.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_37), // 37.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_38), // 38.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::update_active_style), // 39.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::update_focus_style), // 40.
    reinterpret_cast<Method>(::nfs::gui::widgets::capture_style), // 41.
};

void *create_tree_screen(void *parent) {
    auto *page =
        static_cast<TreeScreen *>(gui::allocate_flags(sizeof(TreeScreen), 0, "TreeScreen"));
    if (!page)
        return nullptr;
    try {
        construct_tree_screen(page, parent);
        reinterpret_cast<Widget *>(page)->methods = tree_screen_methods.data();
        return page;
    } catch (...) {
        gui::release_flags(page, 0, "TreeScreen");
        throw;
    }
}
} // namespace nfs::application::frontend
