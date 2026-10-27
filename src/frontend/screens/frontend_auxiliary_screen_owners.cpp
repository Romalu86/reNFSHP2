#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "../../gui/widgets/gui_widget_shared_methods.hpp"
#include "frontend_auxiliary_screens.hpp"
#include "../../gui/lifecycle/gui_named_identifiers.hpp"
#include <algorithm>
#include <cstdlib>
#include <cstring>
namespace nfs::application::frontend {
using namespace gui::widgets;

static void destroy_lobby_screen(LobbyScreen *screen) {
    screen->base.methods = lobby_screen_methods.data();
    destroy_interface(&screen->base);
}

static LobbyScreen *__fastcall delete_lobby_screen(LobbyScreen *screen, void *, Dword flags) {
    destroy_lobby_screen(screen);
    if (flags & 1)
        gui::release(screen);
    return screen;
}

const char *__cdecl lobby_screen_class_key() {
    return "LobbyScreen";
}

static const char *__fastcall lobby_screen_name(LobbyScreen *, void *) {
    return "LobbyScreen";
}

static Dword __fastcall lobby_screen_is_type(LobbyScreen *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(lobby_screen_class_key) ||
           type == reinterpret_cast<const void *>(interface_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

static Dword __fastcall lobby_screen_is_name(LobbyScreen *, void *, const char *name) {
    return !std::strncmp(name, "LobbyScreen", 12) || !std::strncmp(name, "GInterface", 11) ||
           !std::strncmp(name, "GUI", 4);
}

const std::array<Method, 42> lobby_screen_methods = {
    reinterpret_cast<Method>(::nfs::application::frontend::delete_lobby_screen), // 0.
    reinterpret_cast<Method>(::nfs::application::frontend::lobby_screen_name), // 1.
    reinterpret_cast<Method>(::nfs::application::frontend::lobby_screen_is_type), // 2.
    reinterpret_cast<Method>(::nfs::application::frontend::lobby_screen_is_name), // 3.
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
    reinterpret_cast<Method>(::nfs::application::frontend::lobby_screen_child), // 14.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_two_arguments), // 15.
    reinterpret_cast<Method>(::nfs::gui::widgets::apply_drawing_state), // 16.
    reinterpret_cast<Method>(::nfs::gui::widgets::prepare_draw), // 17.
    reinterpret_cast<Method>(::nfs::gui::widgets::push_draw_bounds), // 18.
    reinterpret_cast<Method>(::nfs::gui::widgets::draw_content), // 19.
    reinterpret_cast<Method>(::nfs::gui::widgets::pop_draw_bounds), // 20.
    reinterpret_cast<Method>(::nfs::application::frontend::initialize_lobby_screen), // 21.
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

static void *create_lobby_screen(void *parent) {
    auto *screen =
        static_cast<LobbyScreen *>(gui::allocate_flags(sizeof(LobbyScreen), 0, "LobbyScreen"));
    if (!screen)
        return nullptr;
    try {
        gui::widgets::pending::construct_interface(screen, parent);
    } catch (...) {
        gui::release(screen);
        throw;
    }
    screen->players = screen->messages = nullptr;
    screen->chat = nullptr;
    screen->base.methods = lobby_screen_methods.data();
    return screen;
}
static ClassRecord lobby_screen_class;

static void unregister_lobby_screen() {
    unregister_class(&lobby_screen_class);
}

static void register_lobby_screen() {
    register_class(&lobby_screen_class, "LobbyScreen", create_lobby_screen, "GInterface");
    std::atexit(unregister_lobby_screen);
}

static void destroy_replay_screen(ReplayScreen *screen) {
    screen->base.methods = replay_screen_methods.data();
    destroy_interface(&screen->base);
}

static ReplayScreen *__fastcall delete_replay_screen(ReplayScreen *screen, void *, Dword flags) {
    destroy_replay_screen(screen);
    if (flags & 1)
        gui::release(screen);
    return screen;
}

const char *__cdecl replay_screen_class_key() {
    return "ReplayScreen";
}

static const char *__fastcall replay_screen_name(ReplayScreen *, void *) {
    return "ReplayScreen";
}

static Dword __fastcall replay_screen_is_type(ReplayScreen *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(replay_screen_class_key) ||
           type == reinterpret_cast<const void *>(interface_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

static Dword __fastcall replay_screen_is_name(ReplayScreen *, void *, const char *name) {
    return !std::strncmp(name, "ReplayScreen", 13) || !std::strncmp(name, "GInterface", 11) ||
           !std::strncmp(name, "GUI", 4);
}

const std::array<Method, 42> replay_screen_methods = {
    reinterpret_cast<Method>(::nfs::application::frontend::delete_replay_screen), // 0.
    reinterpret_cast<Method>(::nfs::application::frontend::replay_screen_name), // 1.
    reinterpret_cast<Method>(::nfs::application::frontend::replay_screen_is_type), // 2.
    reinterpret_cast<Method>(::nfs::application::frontend::replay_screen_is_name), // 3.
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
    reinterpret_cast<Method>(::nfs::application::frontend::replay_screen_child), // 14.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_two_arguments), // 15.
    reinterpret_cast<Method>(::nfs::gui::widgets::apply_drawing_state), // 16.
    reinterpret_cast<Method>(::nfs::gui::widgets::prepare_draw), // 17.
    reinterpret_cast<Method>(::nfs::gui::widgets::push_draw_bounds), // 18.
    reinterpret_cast<Method>(::nfs::gui::widgets::draw_content), // 19.
    reinterpret_cast<Method>(::nfs::gui::widgets::pop_draw_bounds), // 20.
    reinterpret_cast<Method>(::nfs::application::frontend::initialize_replay_screen), // 21.
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

static void *create_replay_screen(void *parent) {
    auto *screen =
        static_cast<ReplayScreen *>(gui::allocate_flags(sizeof(ReplayScreen), 0, "ReplayScreen"));
    if (!screen)
        return nullptr;
    try {
        gui::widgets::pending::construct_interface(screen, parent);
    } catch (...) {
        gui::release(screen);
        throw;
    }
    screen->view = screen->remove = screen->replays = nullptr;
    screen->base.methods = replay_screen_methods.data();
    return screen;
}
static ClassRecord replay_screen_class;

static void unregister_replay_screen() {
    unregister_class(&replay_screen_class);
}

static void register_replay_screen() {
    register_class(&replay_screen_class, "ReplayScreen", create_replay_screen, "GInterface");
    std::atexit(unregister_replay_screen);
}
} // namespace nfs::application::frontend Saved startup order is established from the CRT pointer
// array.
NFS_KERNEL_INITIALIZER(startup_register_lobby_screen, ".CRT$XCU_NFS_006900CC",
                       nfs::application::frontend::register_lobby_screen);
NFS_KERNEL_INITIALIZER(startup_initialize_replay_tests, ".CRT$XCU_NFS_0069011C",
                       nfs::gui::widgets::identifiers::initialize_replay_tests);
NFS_KERNEL_INITIALIZER(startup_register_replay_screen, ".CRT$XCU_NFS_00690124",
                       nfs::application::frontend::register_replay_screen);
