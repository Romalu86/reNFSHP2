#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "../../gui/widgets/gui_widget_shared_methods.hpp"
#include "frontend_multiplayer.hpp"
#include <cstdlib>
#include <cstring>
namespace nfs::application::frontend {
using namespace gui::widgets;

const char *__cdecl multiplayer_class_key() {
    return "MultiplayerScreen";
}

static const char *__fastcall multiplayer_name(MultiplayerScreen *, void *) {
    return "MultiplayerScreen";
}

static Dword __fastcall multiplayer_is_type(MultiplayerScreen *, void *, const void *key) {
    return key == reinterpret_cast<const void *>(multiplayer_class_key) ||
           key == reinterpret_cast<const void *>(interface_class_name) ||
           key == reinterpret_cast<const void *>(base_class_name);
}

static Dword __fastcall multiplayer_is_name(MultiplayerScreen *, void *, const char *name) {
    return !std::strncmp(name, "MultiplayerScreen", 18) || !std::strncmp(name, "GInterface", 11) ||
           !std::strncmp(name, "GUI", 4);
}

static MultiplayerScreen *__fastcall delete_multiplayer(MultiplayerScreen *page, void *,
                                                        Dword flags) {
    destroy_multiplayer(page);
    if (flags & 1)
        gui::release(page);
    return page;
}

const std::array<Method, 42> multiplayer_methods = {
    reinterpret_cast<Method>(::nfs::application::frontend::delete_multiplayer), // 0.
    reinterpret_cast<Method>(::nfs::application::frontend::multiplayer_name), // 1.
    reinterpret_cast<Method>(::nfs::application::frontend::multiplayer_is_type), // 2.
    reinterpret_cast<Method>(::nfs::application::frontend::multiplayer_is_name), // 3.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::load_interface), // 4.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::save_interface), // 5.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::layout), // 6.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_text), // 7.
    reinterpret_cast<Method>(::nfs::gui::widgets::text), // 8.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::hit_test), // 9.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::navigate), // 10.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::enter_interface), // 11.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::leave_modal), // 12.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::handle_event), // 13.
    reinterpret_cast<Method>(::nfs::application::frontend::multiplayer_child), // 14.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_two_arguments), // 15.
    reinterpret_cast<Method>(::nfs::gui::widgets::apply_drawing_state), // 16.
    reinterpret_cast<Method>(::nfs::gui::widgets::prepare_draw), // 17.
    reinterpret_cast<Method>(::nfs::gui::widgets::push_draw_bounds), // 18.
    reinterpret_cast<Method>(::nfs::gui::widgets::draw_content), // 19.
    reinterpret_cast<Method>(::nfs::gui::widgets::pop_draw_bounds), // 20.
    reinterpret_cast<Method>(::nfs::application::frontend::initialize_multiplayer), // 21.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_no_arguments), // 22.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_render_argument), // 23.
    reinterpret_cast<Method>(::nfs::application::frontend::tick_multiplayer), // 24.
    reinterpret_cast<Method>(::nfs::application::frontend::leave_multiplayer), // 25.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_layout_argument), // 26.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_parent_enter), // 27.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_parent_leave), // 28.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_parent_argument), // 29.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_child_argument), // 30.
    reinterpret_cast<Method>(::nfs::application::frontend::close_multiplayer), // 31.
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

static void *create_multiplayer(void *parent) {
    auto *page = static_cast<MultiplayerScreen *>(
        gui::allocate_flags(sizeof(MultiplayerScreen), 0, "MultiplayerScreen"));
    if (!page)
        return nullptr;
    try {
        return construct_multiplayer(page, parent);
    } catch (...) {
        gui::release_flags(page, 0, "MultiplayerScreen");
        throw;
    }
}

const char *__cdecl browser_view_class_key() {
    return "BrowserView";
}

static const char *__fastcall browser_view_name(BrowserView *, void *) {
    return "BrowserView";
}

static Dword __fastcall browser_view_is_type(BrowserView *, void *, const void *key) {
    return key == reinterpret_cast<const void *>(browser_view_class_key) ||
           key == reinterpret_cast<const void *>(general_class_name) ||
           key == reinterpret_cast<const void *>(base_class_name);
}

static Dword __fastcall browser_view_is_name(BrowserView *, void *, const char *name) {
    return !std::strncmp(name, "BrowserView", 12) || !std::strncmp(name, "GWidget", 8) ||
           !std::strncmp(name, "GUI", 4);
}

static BrowserView *__fastcall delete_browser_view(BrowserView *page, void *, Dword flags) {
    destroy_browser_view(page);
    if (flags & 1)
        gui::release(page);
    return page;
}

const std::array<Method, 47> browser_view_methods = {
    reinterpret_cast<Method>(::nfs::application::frontend::delete_browser_view), // 0.
    reinterpret_cast<Method>(::nfs::application::frontend::browser_view_name), // 1.
    reinterpret_cast<Method>(::nfs::application::frontend::browser_view_is_type), // 2.
    reinterpret_cast<Method>(::nfs::application::frontend::browser_view_is_name), // 3.
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
    reinterpret_cast<Method>(::nfs::application::frontend::browser_view_child), // 14.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_two_arguments), // 15.
    reinterpret_cast<Method>(::nfs::gui::widgets::apply_drawing_state), // 16.
    reinterpret_cast<Method>(::nfs::gui::widgets::prepare_draw), // 17.
    reinterpret_cast<Method>(::nfs::gui::widgets::push_draw_bounds), // 18.
    reinterpret_cast<Method>(::nfs::gui::widgets::draw_content), // 19.
    reinterpret_cast<Method>(::nfs::gui::widgets::pop_draw_bounds), // 20.
    reinterpret_cast<Method>(::nfs::application::frontend::initialize_browser_view), // 21.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_no_arguments), // 22.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_render_argument), // 23.
    reinterpret_cast<Method>(::nfs::application::frontend::tick_browser_view), // 24.
    reinterpret_cast<Method>(::nfs::application::frontend::leave_browser_view), // 25.
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

static void *create_browser_view(void *parent) {
    auto *page =
        static_cast<BrowserView *>(gui::allocate_flags(sizeof(BrowserView), 0, "BrowserView"));
    if (!page)
        return nullptr;
    try {
        return construct_browser_view(page, parent);
    } catch (...) {
        gui::release_flags(page, 0, "BrowserView");
        throw;
    }
}

const char *__cdecl private_view_class_key() {
    return "PrivateView";
}

static const char *__fastcall private_view_name(PrivateView *, void *) {
    return "PrivateView";
}

static Dword __fastcall private_view_is_type(PrivateView *, void *, const void *key) {
    return key == reinterpret_cast<const void *>(private_view_class_key) ||
           key == reinterpret_cast<const void *>(general_class_name) ||
           key == reinterpret_cast<const void *>(base_class_name);
}

static Dword __fastcall private_view_is_name(PrivateView *, void *, const char *name) {
    return !std::strncmp(name, "PrivateView", 12) || !std::strncmp(name, "GWidget", 8) ||
           !std::strncmp(name, "GUI", 4);
}

static PrivateView *__fastcall delete_private_view(PrivateView *page, void *, Dword flags) {
    destroy_private_view(page);
    if (flags & 1)
        gui::release(page);
    return page;
}

const std::array<Method, 47> private_view_methods = {
    reinterpret_cast<Method>(::nfs::application::frontend::delete_private_view), // 0.
    reinterpret_cast<Method>(::nfs::application::frontend::private_view_name), // 1.
    reinterpret_cast<Method>(::nfs::application::frontend::private_view_is_type), // 2.
    reinterpret_cast<Method>(::nfs::application::frontend::private_view_is_name), // 3.
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
    reinterpret_cast<Method>(::nfs::application::frontend::private_view_child), // 14.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_two_arguments), // 15.
    reinterpret_cast<Method>(::nfs::gui::widgets::apply_drawing_state), // 16.
    reinterpret_cast<Method>(::nfs::gui::widgets::prepare_draw), // 17.
    reinterpret_cast<Method>(::nfs::gui::widgets::push_draw_bounds), // 18.
    reinterpret_cast<Method>(::nfs::gui::widgets::draw_content), // 19.
    reinterpret_cast<Method>(::nfs::gui::widgets::pop_draw_bounds), // 20.
    reinterpret_cast<Method>(::nfs::application::frontend::initialize_private_view), // 21.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_no_arguments), // 22.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_render_argument), // 23.
    reinterpret_cast<Method>(::nfs::application::frontend::tick_private_view), // 24.
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

static void *create_private_view(void *parent) {
    auto *page =
        static_cast<PrivateView *>(gui::allocate_flags(sizeof(PrivateView), 0, "PrivateView"));
    if (!page)
        return nullptr;
    try {
        return construct_private_view(page, parent);
    } catch (...) {
        gui::release_flags(page, 0, "PrivateView");
        throw;
    }
}
static ClassRecord multiplayer_classes[3];

static void unregister_multiplayer_pages() {
    for (int index = 2; index >= 0; --index)
        unregister_class(multiplayer_classes + index);
}

static void register_multiplayer_pages() {
    register_class(multiplayer_classes, "MultiplayerScreen", create_multiplayer, "GInterface");
    register_class(multiplayer_classes + 1, "BrowserView", create_browser_view, "GWidget");
    register_class(multiplayer_classes + 2, "PrivateView", create_private_view, "GWidget");
    std::atexit(unregister_multiplayer_pages);
}
namespace {
NFS_KERNEL_INITIALIZER(startup_register_multiplayer_pages, ".CRT$XCU_NFS_006900EC",
                       register_multiplayer_pages);
} // namespace
} // namespace nfs::application::frontend
