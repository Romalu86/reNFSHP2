#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "../../gui/widgets/gui_widget_shared_methods.hpp"
#include "../screens/frontend_menu_roots.hpp"
#include <cstdlib>
#include <cstring>
namespace nfs::application::frontend {
using namespace gui::widgets;

const char *__cdecl tournament_status_class_key() {
    return "TourneyStatus";
}

static const char *__fastcall tournament_status_name(Widget *, void *) {
    return "TourneyStatus";
}

static Dword __fastcall tournament_status_is_type(Widget *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(tournament_status_class_key) ||
           type == reinterpret_cast<const void *>(interface_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

static Dword __fastcall tournament_status_is_name(Widget *, void *, const char *name) {
    return !std::strncmp(name, "TourneyStatus", 14) || !std::strncmp(name, "GInterface", 11) ||
           !std::strncmp(name, "GUI", 4);
}

static TournamentStatus *__fastcall delete_tournament_status(TournamentStatus *page, void *,
                                                             Dword flags) {
    destroy_tournament_status(page);
    if (flags & 1)
        gui::release(page);
    return page;
}

const std::array<Method, 42> tournament_status_methods = {
    reinterpret_cast<Method>(::nfs::application::frontend::delete_tournament_status), // 0.
    reinterpret_cast<Method>(::nfs::application::frontend::tournament_status_name), // 1.
    reinterpret_cast<Method>(::nfs::application::frontend::tournament_status_is_type), // 2.
    reinterpret_cast<Method>(::nfs::application::frontend::tournament_status_is_name), // 3.
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
    reinterpret_cast<Method>(::nfs::application::frontend::tournament_status_child), // 14.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_two_arguments), // 15.
    reinterpret_cast<Method>(::nfs::gui::widgets::apply_drawing_state), // 16.
    reinterpret_cast<Method>(::nfs::gui::widgets::prepare_draw), // 17.
    reinterpret_cast<Method>(::nfs::gui::widgets::push_draw_bounds), // 18.
    reinterpret_cast<Method>(::nfs::gui::widgets::draw_content), // 19.
    reinterpret_cast<Method>(::nfs::gui::widgets::pop_draw_bounds), // 20.
    reinterpret_cast<Method>(::nfs::application::frontend::initialize_tournament_status), // 21.
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
    reinterpret_cast<Method>(::nfs::application::frontend::tournament_status_key), // 32.
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

void *create_tournament_status(void *parent) {
    auto *page = static_cast<TournamentStatus *>(
        gui::allocate_flags(sizeof(TournamentStatus), 0, "TourneyStatus"));
    if (!page)
        return nullptr;
    try {
        construct_tournament_status(page, parent);
        reinterpret_cast<Widget *>(page)->methods = tournament_status_methods.data();
        return page;
    } catch (...) {
        gui::release(page);
        throw;
    }
}
static ClassRecord tournament_status_class;

static void unregister_tournament_status() {
    unregister_class(&tournament_status_class);
}

static void register_tournament_status() {
    register_class(&tournament_status_class, "TourneyStatus", create_tournament_status,
                   "GInterface");
    std::atexit(unregister_tournament_status);
}
namespace {
NFS_KERNEL_INITIALIZER(startup_register_tournament_status, ".CRT$XCU_NFS_00690154",
                       register_tournament_status);
} // namespace
} // namespace nfs::application::frontend
