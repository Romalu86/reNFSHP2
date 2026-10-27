#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "../../gui/widgets/gui_widget_shared_methods.hpp"
#include "frontend_application_widgets.hpp"
#include "nfs/core/application/application.hpp"
#include "nfs/audio/music/music_clients.hpp"
#include "nfs/network/session/network_rules.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include <algorithm>
#include <cstdlib>
#include <cstring>
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::at;
using game::runtime::invoke;
using game::runtime::system;
extern const std::array<Method, 49> title_screen_methods;

static const char *__cdecl title_screen_class_key() {
    return "TitleScreen";
}

static const char *__fastcall title_screen_name(Widget *, void *) {
    return "TitleScreen";
}

static Dword __fastcall title_screen_is_type(Widget *widget, void *, const void *type) {
    return type == reinterpret_cast<const void *>(title_screen_class_key) ||
           type == reinterpret_cast<const void *>(family::dialog_class_key) ||
           reinterpret_cast<Dword(__thiscall *)(Widget *, const void *)>(family::dialog_methods[2])(
               widget, type);
}

static Dword __fastcall title_screen_is_name(Widget *, void *, const char *name) {
    return !std::strncmp(name, "TitleScreen", 12) || !std::strncmp(name, "GDialog", 8) ||
           !std::strncmp(name, "GWidget", 8) || !std::strncmp(name, "GUI", 4);
}

static void destroy_title_screen(Widget *widget) {
    widget->methods = title_screen_methods.data();
    family::dialog_destroy(widget);
}

static Widget *__fastcall delete_title_screen(Widget *widget, void *, Dword flags) {
    destroy_title_screen(widget);
    if (flags & 1)
        gui::release(widget);
    return widget;
}

static Dword __fastcall title_screen_key_up(Widget *widget, void *, Dword key, Dword modifiers) {
    if (!at<std::uint8_t>(system(96), 12) && !std::uint8_t(modifiers) &&
        at<float>(widget, 284) > 0.25) {
        family::finish_modal(widget, 3, 0);
        game::audio::play_frontend_event(network::rules::random() & 3, 127);
        return 1;
    }
    return reinterpret_cast<Dword(__thiscall *)(Widget *, Dword, Dword)>(family::dialog_methods[33])(
        widget, key, modifiers);
}

static Dword __fastcall title_screen_pointer_press(Widget *widget, void *, Dword button,
                                                  Dword modifiers) {
    if (!at<std::uint8_t>(system(96), 12)) {
        family::finish_modal(widget, 3, 0);
        return 1;
    }
    return reinterpret_cast<Dword(__thiscall *)(Widget *, Dword, Dword)>(family::dialog_methods[40])(
        widget, button, modifiers);
}

const std::array<Method, 49> title_screen_methods = {
    reinterpret_cast<Method>(::nfs::application::frontend::delete_title_screen), // 0.
    reinterpret_cast<Method>(::nfs::application::frontend::title_screen_name), // 1.
    reinterpret_cast<Method>(::nfs::application::frontend::title_screen_is_type), // 2.
    reinterpret_cast<Method>(::nfs::application::frontend::title_screen_is_name), // 3.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::dialog_load), // 4.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::dialog_save), // 5.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::layout), // 6.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_text), // 7.
    reinterpret_cast<Method>(::nfs::gui::widgets::text), // 8.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::hit_test), // 9.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::navigate), // 10.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::dialog_enter_modal), // 11.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::dialog_leave_modal), // 12.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::dialog_event), // 13.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::dialog_configure_geometry), // 14.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_two_arguments), // 15.
    reinterpret_cast<Method>(::nfs::gui::widgets::apply_drawing_state), // 16.
    reinterpret_cast<Method>(::nfs::gui::widgets::prepare_draw), // 17.
    reinterpret_cast<Method>(::nfs::gui::widgets::push_draw_bounds), // 18.
    reinterpret_cast<Method>(::nfs::gui::widgets::draw_content), // 19.
    reinterpret_cast<Method>(::nfs::gui::widgets::pop_draw_bounds), // 20.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::dialog_initialize), // 21.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_no_arguments), // 22.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_render_argument), // 23.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::dialog_tick), // 24.
    reinterpret_cast<Method>(::nfs::gui::widgets::clear_layout_request), // 25.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_layout_argument), // 26.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_parent_enter), // 27.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::dialog_parent_leave), // 28.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_parent_argument), // 29.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_child_argument), // 30.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_modal_state), // 31.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::dialog_key_down), // 32.
    reinterpret_cast<Method>(::nfs::application::frontend::title_screen_key_up), // 33.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_34), // 34.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_35), // 35.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::dialog_button_down), // 36.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_37), // 37.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_38), // 38.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::update_active_style), // 39.
    reinterpret_cast<Method>(::nfs::application::frontend::title_screen_pointer_press), // 40.
    reinterpret_cast<Method>(::nfs::gui::widgets::capture_style), // 41.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::dialog_accept), // 42.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::dialog_cancel), // 43.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_scroll), // 44.
    reinterpret_cast<Method>(::nfs::gui::widgets::process_general_command), // 45.
    reinterpret_cast<Method>(::nfs::gui::widgets::process_general_notification), // 46.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::show_dialog), // 47.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::hide_dialog), // 48.
};

static void *create_title_screen(void *parent) {
    void *storage = gui::allocate_flags(288, 0, "TitleScreen");
    if (!storage)
        return nullptr;
    try {
        gui::widgets::pending::construct_dialog(storage, parent);
        static_cast<Widget *>(storage)->methods = title_screen_methods.data();
    } catch (...) {
        gui::release(storage);
        throw;
    }
    return storage;
}
static ClassRecord title_screen_class;

static void unregister_title_screen() {
    unregister_class(&title_screen_class);
}

static void register_title_screen() {
    register_class(&title_screen_class, "TitleScreen", create_title_screen, "GDialog");
    std::atexit(unregister_title_screen);
}
namespace {
NFS_KERNEL_INITIALIZER(startup_register_title_screen, ".CRT$XCU_NFS_00690140",
                       register_title_screen);
} // namespace
} // namespace nfs::application::frontend
