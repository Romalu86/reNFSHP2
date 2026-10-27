#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "frontend_race_loading.hpp"
#include "../../gui/widgets/gui_widget_shared_methods.hpp"
#include "frontend_transition_math.hpp"
#include <cstdlib>
#include <cstring>
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::at;
using game::runtime::invoke;

static void __fastcall initialize_race_loading(RaceLoadingScreen *screen, void *) {
    gui::widgets::pending::request_general_layout(&screen->base, nullptr);
    screen->progress = static_cast<Widget *>(pending::find_child(screen, "<BAR>"));
    auto *full = static_cast<Widget *>(pending::find_child(screen, "<BACK>"));
    screen->completed = 0;
    if (full)
        screen->full_width = full->bounds.width;
}

static Dword __fastcall race_loading_event(RaceLoadingScreen *screen, void *, Dword event,
                                          Dword first, Dword second) {
    if (event == 108 && screen->progress)
        transition_math::advance_loading_screen(screen);
    return gui::widgets::pending::general_event(&screen->base, nullptr, event, first, second);
}

static void destroy_race_loading(RaceLoadingScreen *screen) {
    screen->base.base.methods = race_loading_methods.data();
    destroy_general(&screen->base);
}

static RaceLoadingScreen *__fastcall delete_race_loading(RaceLoadingScreen *screen, void *, Dword flags) {
    destroy_race_loading(screen);
    if (flags & 1)
        gui::release(screen);
    return screen;
}

static const char *__fastcall race_loading_name(Widget *, void *) {
    return "QuickRaceLoadingScreen";
}

static Dword __fastcall race_loading_is_type(Widget *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(game::audio::vehicle::pending::loading_widget_type) ||
           type == reinterpret_cast<const void *>(general_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

static Dword __fastcall race_loading_is_name(Widget *, void *, const char *name) {
    return !std::strncmp(name, "QuickRaceLoadingScreen", 23) ||
           !std::strncmp(name, "GWidget", 8) || !std::strncmp(name, "GUI", 4);
}

const std::array<Method, 47> race_loading_methods = {
    reinterpret_cast<Method>(::nfs::application::frontend::delete_race_loading), // 0.
    reinterpret_cast<Method>(::nfs::application::frontend::race_loading_name), // 1.
    reinterpret_cast<Method>(::nfs::application::frontend::race_loading_is_type), // 2.
    reinterpret_cast<Method>(::nfs::application::frontend::race_loading_is_name), // 3.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::load_general), // 4.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::save_general), // 5.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::layout), // 6.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_text), // 7.
    reinterpret_cast<Method>(::nfs::gui::widgets::text), // 8.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::hit_test), // 9.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::navigate), // 10.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::enter_modal), // 11.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::leave_modal), // 12.
    reinterpret_cast<Method>(::nfs::application::frontend::race_loading_event), // 13.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_four_arguments), // 14.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_two_arguments), // 15.
    reinterpret_cast<Method>(::nfs::gui::widgets::apply_drawing_state), // 16.
    reinterpret_cast<Method>(::nfs::gui::widgets::prepare_draw), // 17.
    reinterpret_cast<Method>(::nfs::gui::widgets::push_draw_bounds), // 18.
    reinterpret_cast<Method>(::nfs::gui::widgets::draw_content), // 19.
    reinterpret_cast<Method>(::nfs::gui::widgets::pop_draw_bounds), // 20.
    reinterpret_cast<Method>(::nfs::application::frontend::initialize_race_loading), // 21.
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

void *create_race_loading_screen(void *parent) {
    auto *screen = static_cast<RaceLoadingScreen *>(
        gui::allocate_flags(sizeof(RaceLoadingScreen), 0, "QuickRaceLoadingScreen"));
    if (!screen)
        return nullptr;
    try {
        construct_widget(screen, parent);
        screen->progress = nullptr;
        screen->full_width = 0;
        screen->completed = 0;
        screen->base.base.methods = race_loading_methods.data();
        return screen;
    } catch (...) {
        gui::release(screen);
        throw;
    }
}
static ClassRecord loading_class;

static void unregister_race_loading() {
    unregister_class(&loading_class);
}

static void register_race_loading() {
    register_class(&loading_class, "QuickRaceLoadingScreen", create_race_loading_screen, "GWidget");
    std::atexit(unregister_race_loading);
}
namespace {
NFS_KERNEL_INITIALIZER(startup_register_race_loading, ".CRT$XCU_NFS_006900C4",
                       register_race_loading);
} // namespace
} // namespace nfs::application::frontend
