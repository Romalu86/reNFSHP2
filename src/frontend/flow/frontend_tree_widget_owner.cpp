#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "../../gui/widgets/gui_widget_shared_methods.hpp"
#include "../screens/frontend_menu_roots.hpp"
#include <cstdlib>
#include <cstring>
namespace nfs::application::frontend {
using namespace gui::widgets;

const char *__cdecl tree_widget_class_key() {
    return "TreeWidget";
}

static const char *__fastcall tree_widget_name(Widget *, void *) {
    return "TreeWidget";
}

static Dword __fastcall tree_widget_is_type(Widget *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(tree_widget_class_key) ||
           type == reinterpret_cast<const void *>(actor_preview_class_key) ||
           type == reinterpret_cast<const void *>(scene_widget_class_key) ||
           type == reinterpret_cast<const void *>(family::image_class_key) ||
           type == reinterpret_cast<const void *>(general_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

static Dword __fastcall tree_widget_is_name(Widget *, void *, const char *name) {
    return !std::strncmp(name, "TreeWidget", 11) || !std::strncmp(name, "ActorWidget", 12) ||
           !std::strncmp(name, "EAGLWidget", 11) || !std::strncmp(name, "GImage", 7) ||
           !std::strncmp(name, "GWidget", 8) || !std::strncmp(name, "GUI", 4);
}

static TreeWidget *__fastcall delete_tree_widget(TreeWidget *page, void *, Dword flags) {
    destroy_tree_widget(page);
    if (flags & 1)
        gui::release(page);
    return page;
}

const std::array<Method, 47> tree_widget_methods = {
    reinterpret_cast<Method>(::nfs::application::frontend::delete_tree_widget), // 0.
    reinterpret_cast<Method>(::nfs::application::frontend::tree_widget_name), // 1.
    reinterpret_cast<Method>(::nfs::application::frontend::tree_widget_is_type), // 2.
    reinterpret_cast<Method>(::nfs::application::frontend::tree_widget_is_name), // 3.
    reinterpret_cast<Method>(::nfs::application::frontend::load_tree_widget), // 4.
    reinterpret_cast<Method>(::nfs::application::frontend::save_tree_widget), // 5.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::layout), // 6.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_text), // 7.
    reinterpret_cast<Method>(::nfs::gui::widgets::text), // 8.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::hit_test), // 9.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::navigate), // 10.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::enter_modal), // 11.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::leave_modal), // 12.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::general_event), // 13.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_four_arguments), // 14.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_two_arguments), // 15.
    reinterpret_cast<Method>(::nfs::application::frontend::cursor_scene_widget), // 16.
    reinterpret_cast<Method>(::nfs::gui::widgets::prepare_draw), // 17.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::static_image_push_bounds), // 18.
    reinterpret_cast<Method>(::nfs::application::frontend::draw_tree_widget), // 19.
    reinterpret_cast<Method>(::nfs::gui::widgets::pop_draw_bounds), // 20.
    reinterpret_cast<Method>(::nfs::application::frontend::initialize_tree_widget), // 21.
    reinterpret_cast<Method>(::nfs::application::frontend::resume_tree_widget), // 22.
    reinterpret_cast<Method>(::nfs::application::frontend::tick_actor_preview), // 23.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_style_argument), // 24.
    reinterpret_cast<Method>(::nfs::application::frontend::release_scene_widget), // 25.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_layout_argument), // 26.
    reinterpret_cast<Method>(::nfs::application::frontend::leave_scene_widget), // 27.
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
    reinterpret_cast<Method>(::nfs::application::frontend::drag_scene_widget), // 39.
    reinterpret_cast<Method>(::nfs::application::frontend::press_scene_widget), // 40.
    reinterpret_cast<Method>(::nfs::application::frontend::release_scene_pointer), // 41.
    reinterpret_cast<Method>(::nfs::gui::widgets::accept_general), // 42.
    reinterpret_cast<Method>(::nfs::gui::widgets::cancel_general), // 43.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_scroll), // 44.
    reinterpret_cast<Method>(::nfs::gui::widgets::process_general_command), // 45.
    reinterpret_cast<Method>(::nfs::gui::widgets::process_general_notification), // 46.
};

void *create_tree_widget(void *parent) {
    auto *page =
        static_cast<TreeWidget *>(gui::allocate_flags(sizeof(TreeWidget), 0, "TreeWidget"));
    if (!page)
        return nullptr;
    try {
        construct_tree_widget(page, parent);
        reinterpret_cast<Widget *>(page)->methods = tree_widget_methods.data();
        return page;
    } catch (...) {
        gui::release_flags(page, 0, "TreeWidget");
        throw;
    }
}
static ClassRecord tree_widget_class;

static void unregister_tree_widget() {
    unregister_class(&tree_widget_class);
}

static void register_tree_widget() {
    register_class(&tree_widget_class, "TreeWidget", create_tree_widget, "ActorWidget");
    std::atexit(unregister_tree_widget);
}
namespace {
NFS_KERNEL_INITIALIZER(startup_register_tree_widget, ".CRT$XCU_NFS_00690164",
                       register_tree_widget);
} // namespace
} // namespace nfs::application::frontend
