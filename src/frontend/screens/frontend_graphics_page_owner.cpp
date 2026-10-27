#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "../../gui/widgets/gui_widget_shared_methods.hpp"
#include "frontend_menu_pages.hpp"
#include <cstdlib>
#include "nfs/graphics/configuration/render_settings.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include <cstring>
namespace nfs::application::frontend {
using namespace gui::widgets;
extern const std::array<Method, 47> graphics_page_methods;

const char *__cdecl graphics_page_class_key() {
    return "GraphicsScreen";
}

static const char *__fastcall graphics_page_name(Widget *, void *) {
    return "GraphicsScreen";
}

static Dword __fastcall graphics_page_is_type(Widget *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(graphics_page_class_key) ||
           type == reinterpret_cast<const void *>(general_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

static Dword __fastcall graphics_page_is_name(Widget *, void *, const char *name) {
    return !std::strncmp(name, "GraphicsScreen", 15) || !std::strncmp(name, "GWidget", 8) ||
           !std::strncmp(name, "GUI", 4);
}

static void destroy_graphics_page(GraphicsPage *page) {
    page->base.base.methods = graphics_page_methods.data();
    destroy_general(&page->base);
}

static GraphicsPage *__fastcall delete_graphics_page(GraphicsPage *page, void *, Dword flags) {
    destroy_graphics_page(page);
    if (flags & 1)
        gui::release(page);
    return page;
}

const std::array<Method, 47> graphics_page_methods = {
    reinterpret_cast<Method>(::nfs::application::frontend::delete_graphics_page), // 0.
    reinterpret_cast<Method>(::nfs::application::frontend::graphics_page_name), // 1.
    reinterpret_cast<Method>(::nfs::application::frontend::graphics_page_is_type), // 2.
    reinterpret_cast<Method>(::nfs::application::frontend::graphics_page_is_name), // 3.
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
    reinterpret_cast<Method>(::nfs::application::frontend::graphics_page_child_event), // 14.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_two_arguments), // 15.
    reinterpret_cast<Method>(::nfs::gui::widgets::apply_drawing_state), // 16.
    reinterpret_cast<Method>(::nfs::gui::widgets::prepare_draw), // 17.
    reinterpret_cast<Method>(::nfs::gui::widgets::push_draw_bounds), // 18.
    reinterpret_cast<Method>(::nfs::gui::widgets::draw_content), // 19.
    reinterpret_cast<Method>(::nfs::gui::widgets::pop_draw_bounds), // 20.
    reinterpret_cast<Method>(::nfs::application::frontend::initialize_graphics_page), // 21.
    reinterpret_cast<Method>(::nfs::application::frontend::refresh_graphics_page), // 22.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_render_argument), // 23.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_style_argument), // 24.
    reinterpret_cast<Method>(::nfs::gui::widgets::clear_layout_request), // 25.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_layout_argument), // 26.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_parent_enter), // 27.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_parent_leave), // 28.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_parent_argument), // 29.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_child_argument), // 30.
    reinterpret_cast<Method>(::nfs::application::frontend::finish_graphics_page), // 31.
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

static void *create_graphics_page(void *parent) {
    auto *page =
        static_cast<GraphicsPage *>(gui::allocate_flags(sizeof(GraphicsPage), 0, "GraphicsScreen"));
    if (!page)
        return nullptr;
    try {
        construct_widget(page, parent);
        const Dword resolution = Dword(graphics::startup::settings.race.index);
        const Dword colour_depth = Dword(graphics::startup::settings.race.colour_depth);
        page->resolution = page->colour_depth = nullptr;
        page->gamma = nullptr;
        page->base.base.methods = graphics_page_methods.data();
        lifecycle::construction::unresolved::graphics_screen_resolution_setting = resolution;
        lifecycle::construction::unresolved::graphics_colour_depth_setting = colour_depth;
        return page;
    } catch (...) {
        gui::release_flags(page, 0, "GraphicsScreen");
        throw;
    }
}
static ClassRecord graphics_page_class;

static void unregister_graphics_page() {
    unregister_class(&graphics_page_class);
}

static void register_graphics_page() {
    register_class(&graphics_page_class, "GraphicsScreen", create_graphics_page, "GWidget");
    std::atexit(unregister_graphics_page);
}
namespace {
NFS_KERNEL_INITIALIZER(startup_register_graphics_page, ".CRT$XCU_NFS_006900A0",
                       register_graphics_page);
} // namespace
} // namespace nfs::application::frontend
