#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "../../gui/widgets/gui_widget_shared_methods.hpp"
#include "frontend_menu_pages.hpp"
#include <cstdlib>
#include <cstring>
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::invoke;
using game::runtime::system;
struct ChallengeScreen {
    Widget base;
    TypeSpinControl *race_type;
};
static_assert(sizeof(ChallengeScreen) == 260);
extern const std::array<Method, 42> challenge_screen_methods;

static ChallengeScreen *construct_challenge_screen(ChallengeScreen *screen, void *parent) {
    gui::widgets::pending::construct_interface(screen, parent);
    screen->race_type = nullptr;
    screen->base.methods = challenge_screen_methods.data();
    return screen;
}

static void destroy_challenge_screen(ChallengeScreen *screen) {
    screen->base.methods = challenge_screen_methods.data();
    destroy_interface(&screen->base);
}

static ChallengeScreen *__fastcall delete_challenge_screen(ChallengeScreen *screen, void *,
                                                           Dword flags) {
    destroy_challenge_screen(screen);
    if (flags & 1)
        gui::release(screen);
    return screen;
}

static const char *__fastcall challenge_screen_name(Widget *, void *) {
    return "SingleChallengeScreen";
}

static Dword __fastcall challenge_screen_is_type(Widget *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(challenge_screen_class_key) ||
           type == reinterpret_cast<const void *>(interface_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

static Dword __fastcall challenge_screen_is_name(Widget *, void *, const char *name) {
    return !std::strncmp(name, "SingleChallengeScreen", 22) ||
           !std::strncmp(name, "GInterface", 11) || !std::strncmp(name, "GUI", 4);
}

static void __fastcall initialize_challenge_screen(ChallengeScreen *screen, void *) {
    gui::widgets::pending::request_interface_layout(&screen->base, nullptr);
    auto *selector = static_cast<Widget *>(pending::find_child(screen, "<RACETYPE>"));
    screen->race_type = selector && invoke<Dword>(selector, 2, type_spin_class_key)
                            ? reinterpret_cast<TypeSpinControl *>(selector)
                            : nullptr;
    network::packets::pending::set_game_rule(system(72), 1, 1);
}
// Body is RET4.
static void __fastcall update_challenge_screen(ChallengeScreen *, void *, Dword) {
}
 
static void __fastcall release_challenge_screen(ChallengeScreen *, void *) {
}

static Dword __fastcall challenge_screen_event(ChallengeScreen *screen, void *, Dword event,
                                              Dword first, Dword second) {
    return gui::widgets::pending::handle_event(&screen->base, nullptr, event, first, second);
}

static Dword __fastcall finish_challenge_screen(ChallengeScreen *screen, void *, Dword result,
                                               Widget *owner) {
    if (!owner || !invoke<Dword>(owner, 2, interface_class_name))
        return 0;
    if (invoke<Dword>(owner, 3, "SingleChallengeScreen") && result == 4) {
        auto *view = static_cast<Widget *>(pending::find_child(owner, "<VIEW>"));
        if (view && invoke<Dword>(view, 2, challenge_view_class_key) &&
            !can_leave_challenge_view(reinterpret_cast<ChallengeView *>(view)))
            return 0;
    }
    screen->base.modal_result = result;
    return 1;
}

const std::array<Method, 42> challenge_screen_methods = {
    reinterpret_cast<Method>(::nfs::application::frontend::delete_challenge_screen), // 0.
    reinterpret_cast<Method>(::nfs::application::frontend::challenge_screen_name), // 1.
    reinterpret_cast<Method>(::nfs::application::frontend::challenge_screen_is_type), // 2.
    reinterpret_cast<Method>(::nfs::application::frontend::challenge_screen_is_name), // 3.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::load_interface), // 4.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::save_interface), // 5.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::layout), // 6.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_text), // 7.
    reinterpret_cast<Method>(::nfs::gui::widgets::text), // 8.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::hit_test), // 9.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::navigate), // 10.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::enter_interface), // 11.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::leave_modal), // 12.
    reinterpret_cast<Method>(::nfs::application::frontend::challenge_screen_event), // 13.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_four_arguments), // 14.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_two_arguments), // 15.
    reinterpret_cast<Method>(::nfs::gui::widgets::apply_drawing_state), // 16.
    reinterpret_cast<Method>(::nfs::gui::widgets::prepare_draw), // 17.
    reinterpret_cast<Method>(::nfs::gui::widgets::push_draw_bounds), // 18.
    reinterpret_cast<Method>(::nfs::gui::widgets::draw_content), // 19.
    reinterpret_cast<Method>(::nfs::gui::widgets::pop_draw_bounds), // 20.
    reinterpret_cast<Method>(::nfs::application::frontend::initialize_challenge_screen), // 21.
    reinterpret_cast<Method>(::nfs::application::frontend::release_challenge_screen), // 22.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_render_argument), // 23.
    reinterpret_cast<Method>(::nfs::application::frontend::update_challenge_screen), // 24.
    reinterpret_cast<Method>(::nfs::gui::widgets::clear_layout_request), // 25.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_layout_argument), // 26.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_parent_enter), // 27.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_parent_leave), // 28.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_parent_argument), // 29.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_child_argument), // 30.
    reinterpret_cast<Method>(::nfs::application::frontend::finish_challenge_screen), // 31.
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

static void *create_challenge_screen(void *parent) {
    auto *screen = static_cast<ChallengeScreen *>(
        gui::allocate_flags(sizeof(ChallengeScreen), 0, "SingleChallengeScreen"));
    if (!screen)
        return nullptr;
    try {
        return construct_challenge_screen(screen, parent);
    } catch (...) {
        gui::release(screen);
        throw;
    }
}
static ClassRecord challenge_screen_class;

static void unregister_challenge_screen() {
    unregister_class(&challenge_screen_class);
}

static void register_challenge_screen() {
    register_class(&challenge_screen_class, "SingleChallengeScreen", create_challenge_screen,
                   "GInterface");
    std::atexit(unregister_challenge_screen);
}
static CallbackRecord challenge_heading_callback;

static void unregister_challenge_heading() {
    unregister_callback(&challenge_heading_callback);
}

static void register_challenge_heading() {
    register_callback(&challenge_heading_callback, general_class_name, "GEVENT_FOCUSSET", 14,
                      "SingleChallengeScreen::UpdateDisplayData", update_challenge_heading, 0);
    std::atexit(unregister_challenge_heading);
}
namespace {
NFS_KERNEL_INITIALIZER(startup_register_challenge_screen, ".CRT$XCU_NFS_00690130",
                       register_challenge_screen);
NFS_KERNEL_INITIALIZER(startup_register_challenge_heading, ".CRT$XCU_NFS_00690134",
                       register_challenge_heading);
} // namespace
} // namespace nfs::application::frontend
