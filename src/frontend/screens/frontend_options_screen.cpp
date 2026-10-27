#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "../../gui/widgets/gui_widget_shared_methods.hpp"
#include "frontend_menu_pages.hpp"
#include <cstdlib>
#include <cstring>
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::at;
using game::runtime::invoke;
using game::runtime::system;
struct OptionsScreen {
    Widget base;
    Dword preferences[43];
    Dword controller_mode, active_controller, language, audio_mode;
};
static_assert(sizeof(OptionsScreen) == 444 && offsetof(OptionsScreen, language) == 436);
static OptionsScreen *active_options_screen;
extern const std::array<Method, 42> options_screen_methods;

static void initialize_option_switches(Dword *values) {
    for (unsigned index = 0; index < 6; ++index)
        values[index] = 1;
    values[6] = values[7] = 0;
}

static void initialize_option_levels(Dword *values) {
    for (unsigned index = 0; index < 5; ++index)
        values[index] = 100;
}

static void initialize_option_bindings(Dword *values) {
    for (unsigned index = 0; index < 8; ++index)
        values[index] = 0;
}

static OptionsScreen *construct_options_screen(OptionsScreen *screen, void *parent) {
    gui::widgets::pending::construct_interface(screen, parent);
    screen->preferences[0] = 0;
    for (unsigned index = 4; index < 12; ++index)
        screen->preferences[index] = 100;
    screen->preferences[12] = 1;
    screen->preferences[13] = 0;
    for (unsigned index = 14; index < 22; ++index)
        screen->preferences[index] = 1;
    initialize_option_switches(screen->preferences + 22);
    initialize_option_levels(screen->preferences + 30);
    initialize_option_bindings(screen->preferences + 35);
    screen->base.methods = options_screen_methods.data();
    try {
        screen->language = network::packets::rule_value(system(72), 17);
        screen->audio_mode = network::packets::rule_value(system(72), 18);
        if (auto *heading = static_cast<Widget *>(pending::find_child(screen, "<FOCUS_STATE>")))
            invoke<Dword>(heading, 2, ticker_class_key);
        active_options_screen = screen;
    } catch (...) {
        destroy_interface(&screen->base);
        throw;
    }
    return screen;
}

static void destroy_options_screen(OptionsScreen *screen) {
    screen->base.methods = options_screen_methods.data();
    active_options_screen = nullptr;
    destroy_interface(&screen->base);
}

static OptionsScreen *__fastcall delete_options_screen(OptionsScreen *screen, void *, Dword flags) {
    destroy_options_screen(screen);
    if (flags & 1)
        gui::release(screen);
    return screen;
}

static const char *__fastcall options_screen_name(Widget *, void *) {
    return "OptionsScreen";
}

static Dword __fastcall options_screen_is_type(Widget *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(options_screen_class_key) ||
           type == reinterpret_cast<const void *>(interface_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

static Dword __fastcall options_screen_is_name(Widget *, void *, const char *name) {
    return !std::strncmp(name, "OptionsScreen", 14) || !std::strncmp(name, "GInterface", 11) ||
           !std::strncmp(name, "GUI", 4);
}

static Dword __fastcall finish_options_screen(OptionsScreen *screen, void *, Dword result, Widget *) {
    screen->base.modal_result = result;
    return 1;
}

static Dword __fastcall options_screen_key(OptionsScreen *screen, void *, Dword key, Dword modifiers) {
    return gui::widgets::pending::interface_key(&screen->base, nullptr, key, modifiers);
}
// No incoming native call or pointer reference is yet qualified.
Byte __fastcall options_settings_changed(OptionsScreen *screen, void *) {
    const auto *rules = system(72);
    if (screen->preferences[12] != at<Dword>(rules, 2360)) return 1;
    if (screen->preferences[7] != at<Dword>(rules, 2340)) return 1;
    if (screen->preferences[10] != at<Dword>(rules, 2352)) return 1;
    if (screen->preferences[4] != at<Dword>(rules, 2328)) return 1;
    if (screen->preferences[8] != at<Dword>(rules, 2344)) return 1;
    if (screen->preferences[9] != at<Dword>(rules, 2348)) return 1;
    if (screen->preferences[6] != at<Dword>(rules, 2336)) return 1;
    if (screen->preferences[5] != at<Dword>(rules, 2332)) return 1;
    if (screen->preferences[11] != at<Dword>(rules, 2356)) return 1;
    if (screen->preferences[0] != at<Dword>(rules, 2312)) return 1;
    if (screen->preferences[1] != at<Dword>(rules, 2316)) return 1;
    if (screen->preferences[2] != at<Dword>(rules, 2320)) return 1;
    if (screen->preferences[3] != at<Dword>(rules, 2324)) return 1;
    if (screen->preferences[26] != at<Dword>(rules, 2416)) return 1;
    if (screen->preferences[27] != at<Dword>(rules, 2420)) return 1;
    if (screen->preferences[25] != at<Dword>(rules, 2412)) return 1;
    if (screen->preferences[23] != at<Dword>(rules, 2404)) return 1;
    if (screen->preferences[24] != at<Dword>(rules, 2408)) return 1;
    if (screen->preferences[22] != at<Dword>(rules, 2400)) return 1;
    if (screen->preferences[14] != at<Dword>(rules, 2368)) return 1;
    if (screen->preferences[15] != at<Dword>(rules, 2372)) return 1;
    if (screen->preferences[13] != at<Dword>(rules, 2364)) return 1;
    if (screen->preferences[16] != at<Dword>(rules, 2376)) return 1;
    if (at<Dword>(rules, 2648) != screen->controller_mode) return 1;
    const Dword active_controller = at<Dword>(rules, 52736);
    return active_controller != screen->active_controller;
}

static void __fastcall initialize_options_screen(OptionsScreen *screen, void *) {
    if (auto *build = static_cast<Widget *>(pending::find_child(screen, "<build>")))
        if (invoke<Dword>(build, 2, family::text_class_key)) {
            set_enabled(build, 0);
            pending::set_widget_visible(build, 0);
        }
    gui::widgets::pending::request_interface_layout(&screen->base, nullptr);
    auto *rules = system(72);
    const Dword controller_mode = at<Dword>(rules, 2648),
               active_controller = at<Dword>(rules, 52736);
    for (unsigned index = 0; index < 43; ++index)
        screen->preferences[index] = at<Dword>(rules, 2312 + 4 * index);
    screen->controller_mode = controller_mode;
    screen->active_controller = active_controller;
}

static Dword __fastcall options_screen_event(OptionsScreen *screen, void *, Dword event, Dword first,
                                            Dword second) {
    if (event == 6) {
        auto *view = static_cast<Widget *>(pending::find_child(screen, "<VIEW>"));
        if (view && invoke<Dword>(view, 2, options_view_class_key))
            if (auto *credits = static_cast<Widget *>(pending::find_child(view, "credits")))
                dispatch_target(credits->id, 6, first, second);
    }
    return gui::widgets::pending::handle_event(&screen->base, nullptr, event, first, second);
}

void queue_options_notice(const gui::String *text) {
    if (active_options_screen)
        if (auto *heading =
                static_cast<Widget *>(pending::find_child(active_options_screen, "<FOCUS_STATE>")))
            if (invoke<Dword>(heading, 2, ticker_class_key))
                queue_ticker_message(reinterpret_cast<Ticker *>(heading), text, 1, &heading->colour,
                                     0, 100.0f, 1);
}

const std::array<Method, 42> options_screen_methods = {
    reinterpret_cast<Method>(::nfs::application::frontend::delete_options_screen), // 0.
    reinterpret_cast<Method>(::nfs::application::frontend::options_screen_name), // 1.
    reinterpret_cast<Method>(::nfs::application::frontend::options_screen_is_type), // 2.
    reinterpret_cast<Method>(::nfs::application::frontend::options_screen_is_name), // 3.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::load_interface), // 4.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::save_interface), // 5.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::layout), // 6.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_text), // 7.
    reinterpret_cast<Method>(::nfs::gui::widgets::text), // 8.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::hit_test), // 9.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::navigate), // 10.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::enter_interface), // 11.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::leave_modal), // 12.
    reinterpret_cast<Method>(::nfs::application::frontend::options_screen_event), // 13.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_four_arguments), // 14.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_two_arguments), // 15.
    reinterpret_cast<Method>(::nfs::gui::widgets::apply_drawing_state), // 16.
    reinterpret_cast<Method>(::nfs::gui::widgets::prepare_draw), // 17.
    reinterpret_cast<Method>(::nfs::gui::widgets::push_draw_bounds), // 18.
    reinterpret_cast<Method>(::nfs::gui::widgets::draw_content), // 19.
    reinterpret_cast<Method>(::nfs::gui::widgets::pop_draw_bounds), // 20.
    reinterpret_cast<Method>(::nfs::application::frontend::initialize_options_screen), // 21.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_no_arguments), // 22.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_render_argument), // 23.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_style_argument), // 24.
    reinterpret_cast<Method>(::nfs::gui::widgets::clear_layout_request), // 25.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_layout_argument), // 26.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_parent_enter), // 27.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_parent_leave), // 28.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_parent_argument), // 29.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_child_argument), // 30.
    reinterpret_cast<Method>(::nfs::application::frontend::finish_options_screen), // 31.
    reinterpret_cast<Method>(::nfs::application::frontend::options_screen_key), // 32.
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

static void *create_options_screen(void *parent) {
    auto *screen = static_cast<OptionsScreen *>(
        gui::allocate_flags(sizeof(OptionsScreen), 0, "OptionsScreen"));
    if (!screen)
        return nullptr;
    try {
        return construct_options_screen(screen, parent);
    } catch (...) {
        gui::release_flags(screen, 0, "OptionsScreen");
        throw;
    }
}
static ClassRecord options_screen_class;

static void unregister_options_screen() {
    unregister_class(&options_screen_class);
}

static void register_options_screen() {
    register_class(&options_screen_class, "OptionsScreen", create_options_screen, "GInterface");
    std::atexit(unregister_options_screen);
}
namespace {
NFS_KERNEL_INITIALIZER(startup_register_options_screen, ".CRT$XCU_NFS_006900F4",
                       register_options_screen);
} // namespace
} // namespace nfs::application::frontend
