#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "../../gui/widgets/gui_widget_shared_methods.hpp"
#include "../flow/frontend_scene_widgets.hpp"
#include <cstdlib>
#include <cstring>
namespace nfs::application::frontend {
using namespace gui::widgets;

const char *__cdecl menu_image_list_class_key() {
    return "HPImgList";
}

static const char *__fastcall menu_image_list_name(Widget *, void *) {
    return "HPImgList";
}

static Dword __fastcall menu_image_list_is_type(Widget *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(menu_image_list_class_key) ||
           type == reinterpret_cast<const void *>(family::image_list_class_key) ||
           type == reinterpret_cast<const void *>(general_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

static Dword __fastcall menu_image_list_is_name(Widget *, void *, const char *name) {
    return !std::strncmp(name, "HPImgList", 10) || !std::strncmp(name, "GImageList", 11) ||
           !std::strncmp(name, "GWidget", 8) || !std::strncmp(name, "GUI", 4);
}

static void destroy_menu_image_list(MenuImageList *page) {
    reinterpret_cast<Widget *>(page)->methods = menu_image_list_methods.data();
    family::image_list_destroy(reinterpret_cast<Widget *>(page));
}

static MenuImageList *__fastcall delete_menu_image_list(MenuImageList *page, void *, Dword flags) {
    destroy_menu_image_list(page);
    if (flags & 1)
        gui::release(page);
    return page;
}

const std::array<Method, 51> menu_image_list_methods = {
    reinterpret_cast<Method>(::nfs::application::frontend::delete_menu_image_list), // 0.
    reinterpret_cast<Method>(::nfs::application::frontend::menu_image_list_name), // 1.
    reinterpret_cast<Method>(::nfs::application::frontend::menu_image_list_is_type), // 2.
    reinterpret_cast<Method>(::nfs::application::frontend::menu_image_list_is_name), // 3.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::image_list_load), // 4.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::image_list_save), // 5.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::layout), // 6.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_text), // 7.
    reinterpret_cast<Method>(::nfs::gui::widgets::text), // 8.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::hit_test), // 9.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::navigate), // 10.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::enter_modal), // 11.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::leave_modal), // 12.
    reinterpret_cast<Method>(::nfs::application::frontend::menu_image_list_event), // 13.
    reinterpret_cast<Method>(::nfs::application::frontend::menu_image_list_child), // 14.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_two_arguments), // 15.
    reinterpret_cast<Method>(::nfs::gui::widgets::apply_drawing_state), // 16.
    reinterpret_cast<Method>(::nfs::gui::widgets::prepare_draw), // 17.
    reinterpret_cast<Method>(::nfs::gui::widgets::push_draw_bounds), // 18.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::image_list_draw), // 19.
    reinterpret_cast<Method>(::nfs::gui::widgets::pop_draw_bounds), // 20.
    reinterpret_cast<Method>(::nfs::application::frontend::initialize_menu_image_list), // 21.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_no_arguments), // 22.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::image_list_animate), // 23.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_style_argument), // 24.
    reinterpret_cast<Method>(::nfs::gui::widgets::clear_layout_request), // 25.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_layout_argument), // 26.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_parent_enter), // 27.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_parent_leave), // 28.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_parent_argument), // 29.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_child_argument), // 30.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_modal_state), // 31.
    reinterpret_cast<Method>(::nfs::application::frontend::menu_image_list_key), // 32.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_result), // 33.
    reinterpret_cast<Method>(::nfs::application::frontend::menu_image_list_key_up), // 34.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_35), // 35.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::general_button), // 36.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_37), // 37.
    reinterpret_cast<Method>(::nfs::application::frontend::menu_image_list_release), // 38.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::update_active_style), // 39.
    reinterpret_cast<Method>(::nfs::application::frontend::menu_image_list_press), // 40.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::image_list_pointer_release), // 41.
    reinterpret_cast<Method>(::nfs::gui::widgets::accept_general), // 42.
    reinterpret_cast<Method>(::nfs::gui::widgets::cancel_general), // 43.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_scroll), // 44.
    reinterpret_cast<Method>(::nfs::gui::widgets::process_general_command), // 45.
    reinterpret_cast<Method>(::nfs::gui::widgets::process_general_notification), // 46.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::image_list_item_operation_47), // 47.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::image_list_item_operation_48), // 48.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::draw_image_list_cell), // 49.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::image_list_cell_colour), // 50.
};

void *create_menu_image_list(void *parent) {
    auto *page =
        static_cast<MenuImageList *>(gui::allocate_flags(sizeof(MenuImageList), 0, "HPImgList"));
    if (!page)
        return nullptr;
    try {
        gui::widgets::pending::construct_image_list(page, parent);
        page->navigation_count = 0;
        page->reserved = page->left = page->right = 0;
        reinterpret_cast<Widget *>(page)->methods = menu_image_list_methods.data();
        return page;
    } catch (...) {
        gui::release_flags(page, 0, "HPImgList");
        throw;
    }
}
static ClassRecord menu_image_list_class;

static void unregister_menu_image_list() {
    unregister_class(&menu_image_list_class);
}

static void register_menu_image_list() {
    register_class(&menu_image_list_class, "HPImgList", create_menu_image_list, "GImageList");
    std::atexit(unregister_menu_image_list);
}
namespace {
NFS_KERNEL_INITIALIZER(startup_register_menu_image_list, ".CRT$XCU_NFS_006900A8",
                       register_menu_image_list);
} // namespace
} // namespace nfs::application::frontend
