#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "../../gui/widgets/gui_widget_shared_methods.hpp"
#include "frontend_menu_pages.hpp"
#include <cstdlib>
#include <cstring>
namespace nfs::application::frontend {
using namespace gui::widgets;
extern const std::array<Method, 47> free_run_methods;

const char *__cdecl free_run_class_key() {
    return "FreeRunScreen";
}

static const char *__fastcall free_run_name(Widget *, void *) {
    return "FreeRunScreen";
}

static Dword __fastcall free_run_is_type(Widget *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(free_run_class_key) ||
           type == reinterpret_cast<const void *>(general_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

static Dword __fastcall free_run_is_name(Widget *, void *, const char *name) {
    return !std::strncmp(name, "FreeRunScreen", 14) || !std::strncmp(name, "GWidget", 8) ||
           !std::strncmp(name, "GUI", 4);
}

static void destroy_free_run(FreeRunPage *page) {
    page->base.base.methods = free_run_methods.data();
    destroy_general(&page->base);
}

static FreeRunPage *__fastcall delete_free_run(FreeRunPage *page, void *, Dword flags) {
    destroy_free_run(page);
    if (flags & 1)
        gui::release(page);
    return page;
}

const std::array<Method, 47> free_run_methods = {
    reinterpret_cast<Method>(::nfs::application::frontend::delete_free_run), // 0.
    reinterpret_cast<Method>(::nfs::application::frontend::free_run_name), // 1.
    reinterpret_cast<Method>(::nfs::application::frontend::free_run_is_type), // 2.
    reinterpret_cast<Method>(::nfs::application::frontend::free_run_is_name), // 3.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::load_general), // 4.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::save_general), // 5.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::layout), // 6.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_text), // 7.
    reinterpret_cast<Method>(::nfs::gui::widgets::text), // 8.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::hit_test), // 9.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::navigate), // 10.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::enter_modal), // 11.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::leave_modal), // 12.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::general_event), // 13.
    reinterpret_cast<Method>(::nfs::application::frontend::free_run_child_event), // 14.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_two_arguments), // 15.
    reinterpret_cast<Method>(::nfs::gui::widgets::apply_drawing_state), // 16.
    reinterpret_cast<Method>(::nfs::gui::widgets::prepare_draw), // 17.
    reinterpret_cast<Method>(::nfs::gui::widgets::push_draw_bounds), // 18.
    reinterpret_cast<Method>(::nfs::gui::widgets::draw_content), // 19.
    reinterpret_cast<Method>(::nfs::gui::widgets::pop_draw_bounds), // 20.
    reinterpret_cast<Method>(::nfs::application::frontend::initialize_free_run), // 21.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_no_arguments), // 22.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_render_argument), // 23.
    reinterpret_cast<Method>(::nfs::application::frontend::update_free_run), // 24.
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

static void *create_free_run(void *parent) {
    auto *page =
        static_cast<FreeRunPage *>(gui::allocate_flags(sizeof(FreeRunPage), 0, "FreeRunScreen"));
    if (!page)
        return nullptr;
    try {
        construct_widget(page, parent);
        page->players = nullptr;
        page->traffic = nullptr;
        page->base.base.methods = free_run_methods.data();
        return page;
    } catch (...) {
        gui::release_flags(page, 0, "FreeRunScreen");
        throw;
    }
}
static ClassRecord free_run_class;

static void unregister_free_run() {
    unregister_class(&free_run_class);
}

static void register_free_run() {
    register_class(&free_run_class, "FreeRunScreen", create_free_run, "GWidget");
    std::atexit(unregister_free_run);
}
namespace {
NFS_KERNEL_INITIALIZER(startup_register_free_run, ".CRT$XCU_NFS_0069008C",
                       register_free_run);
} // namespace
} // namespace nfs::application::frontend
