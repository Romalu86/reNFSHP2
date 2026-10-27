#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "../../gui/widgets/gui_widget_shared_methods.hpp"
#include "frontend_scene_widgets.hpp"
#include <cstdlib>
#include <cstring>
namespace nfs::application::frontend {
using namespace gui::widgets;

const char *__cdecl colour_wheel_class_key() {
    return "ColorWheel";
}

static const char *__fastcall colour_wheel_name(Widget *, void *) {
    return "ColorWheel";
}

static Dword __fastcall colour_wheel_is_type(Widget *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(colour_wheel_class_key) ||
           type == reinterpret_cast<const void *>(general_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

static Dword __fastcall colour_wheel_is_name(Widget *, void *, const char *name) {
    return !std::strncmp(name, "ColorWheel", 11) || !std::strncmp(name, "GWidget", 8) ||
           !std::strncmp(name, "GUI", 4);
}

static void destroy_colour_wheel(ColourWheel *page) {
    reinterpret_cast<Widget *>(page)->methods = colour_wheel_methods.data();
    for (int i = 24; i >= 0; --i)
        gui::string_destruct(&page->names[i]);
    destroy_general(&page->base);
}

static ColourWheel *__fastcall delete_colour_wheel(ColourWheel *page, void *, Dword flags) {
    destroy_colour_wheel(page);
    if (flags & 1)
        gui::release(page);
    return page;
}

const std::array<Method, 47> colour_wheel_methods = {
    reinterpret_cast<Method>(::nfs::application::frontend::delete_colour_wheel), // 0.
    reinterpret_cast<Method>(::nfs::application::frontend::colour_wheel_name), // 1.
    reinterpret_cast<Method>(::nfs::application::frontend::colour_wheel_is_type), // 2.
    reinterpret_cast<Method>(::nfs::application::frontend::colour_wheel_is_name), // 3.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::load_general), // 4.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::save_general), // 5.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::layout), // 6.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_text), // 7.
    reinterpret_cast<Method>(::nfs::gui::widgets::text), // 8.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::hit_test), // 9.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::navigate), // 10.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::enter_modal), // 11.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::leave_modal), // 12.
    reinterpret_cast<Method>(::nfs::application::frontend::colour_wheel_event), // 13.
    reinterpret_cast<Method>(::nfs::application::frontend::colour_wheel_child), // 14.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_two_arguments), // 15.
    reinterpret_cast<Method>(::nfs::gui::widgets::apply_drawing_state), // 16.
    reinterpret_cast<Method>(::nfs::gui::widgets::prepare_draw), // 17.
    reinterpret_cast<Method>(::nfs::gui::widgets::push_draw_bounds), // 18.
    reinterpret_cast<Method>(::nfs::gui::widgets::draw_content), // 19.
    reinterpret_cast<Method>(::nfs::gui::widgets::pop_draw_bounds), // 20.
    reinterpret_cast<Method>(::nfs::application::frontend::initialize_colour_wheel), // 21.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_no_arguments), // 22.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_render_argument), // 23.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_style_argument), // 24.
    reinterpret_cast<Method>(::nfs::gui::widgets::clear_layout_request), // 25.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_layout_argument), // 26.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_parent_enter), // 27.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_parent_leave), // 28.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_parent_argument), // 29.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_child_argument), // 30.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_modal_state), // 31.
    reinterpret_cast<Method>(::nfs::application::frontend::colour_wheel_key), // 32.
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

void *create_colour_wheel(void *parent) {
    auto *page =
        static_cast<ColourWheel *>(gui::allocate_flags(sizeof(ColourWheel), 0, "ColorWheel"));
    if (!page)
        return nullptr;
    try {
        construct_widget(page, parent);
        page->up = page->down = 0;
        page->selected = page->count = 0;
        for (auto &colour : page->colours)
            colour = 0;
        unsigned constructed = 0;
        try {
            for (; constructed < 25; ++constructed)
                gui::string_construct(&page->names[constructed]);
        } catch (...) {
            while (constructed)
                gui::string_destruct(&page->names[--constructed]);
            destroy_general(&page->base);
            throw;
        }
        page->swatches[0] = page->swatches[1] = page->swatches[2] = 0;
        for (auto &colour : page->colours)
            colour = 0;
        reinterpret_cast<Widget *>(page)->methods = colour_wheel_methods.data();
        return page;
    } catch (...) {
        gui::release(page);
        throw;
    }
}
static ClassRecord colour_wheel_class;

static void unregister_colour_wheel() {
    unregister_class(&colour_wheel_class);
}

static void register_colour_wheel() {
    register_class(&colour_wheel_class, "ColorWheel", create_colour_wheel, "GWidget");
    std::atexit(unregister_colour_wheel);
}
namespace {
NFS_KERNEL_INITIALIZER(startup_register_colour_wheel, ".CRT$XCU_NFS_00690064",
                       register_colour_wheel);
} // namespace
} // namespace nfs::application::frontend
