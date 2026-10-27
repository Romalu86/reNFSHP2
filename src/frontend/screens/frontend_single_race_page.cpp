#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "../../gui/widgets/gui_widget_shared_methods.hpp"
#include "frontend_menu_pages.hpp"
#include <cstdlib>
#include <cstring>
namespace nfs::application::frontend {
using namespace gui::widgets;
extern const std::array<Method, 47> single_race_methods;

const char *__cdecl single_race_class_key() {
    return "SingleRaceScreen";
}

static const char *__fastcall single_race_name(Widget *, void *) {
    return "SingleRaceScreen";
}

static Dword __fastcall single_race_is_type(Widget *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(single_race_class_key) ||
           type == reinterpret_cast<const void *>(general_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

static Dword __fastcall single_race_is_name(Widget *, void *, const char *name) {
    return !std::strncmp(name, "SingleRaceScreen", 17) || !std::strncmp(name, "GWidget", 8) ||
           !std::strncmp(name, "GUI", 4);
}

static void destroy_single_race(SingleRacePage *page) {
    page->base.base.methods = single_race_methods.data();
    destroy_general(&page->base);
}

static SingleRacePage *__fastcall delete_single_race(SingleRacePage *page, void *, Dword flags) {
    destroy_single_race(page);
    if (flags & 1)
        gui::release(page);
    return page;
}

const std::array<Method, 47> single_race_methods = {
    reinterpret_cast<Method>(::nfs::application::frontend::delete_single_race), // 0.
    reinterpret_cast<Method>(::nfs::application::frontend::single_race_name), // 1.
    reinterpret_cast<Method>(::nfs::application::frontend::single_race_is_type), // 2.
    reinterpret_cast<Method>(::nfs::application::frontend::single_race_is_name), // 3.
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
    reinterpret_cast<Method>(::nfs::application::frontend::single_race_child_event), // 14.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_two_arguments), // 15.
    reinterpret_cast<Method>(::nfs::gui::widgets::apply_drawing_state), // 16.
    reinterpret_cast<Method>(::nfs::gui::widgets::prepare_draw), // 17.
    reinterpret_cast<Method>(::nfs::gui::widgets::push_draw_bounds), // 18.
    reinterpret_cast<Method>(::nfs::gui::widgets::draw_content), // 19.
    reinterpret_cast<Method>(::nfs::gui::widgets::pop_draw_bounds), // 20.
    reinterpret_cast<Method>(::nfs::application::frontend::initialize_single_race), // 21.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_no_arguments), // 22.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_render_argument), // 23.
    reinterpret_cast<Method>(::nfs::application::frontend::update_single_race), // 24.
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

static void *create_single_race(void *parent) {
    auto *page = static_cast<SingleRacePage *>(
        gui::allocate_flags(sizeof(SingleRacePage), 0, "SingleRaceScreen"));
    if (!page)
        return nullptr;
    try {
        construct_widget(page, parent);
        page->players = nullptr;
        page->cops = nullptr;
        page->opponents = nullptr;
        page->opponent_cars = nullptr;
        page->skill = nullptr;
        page->traffic = nullptr;
        page->base.base.methods = single_race_methods.data();
        return page;
    } catch (...) {
        gui::release(page);
        throw;
    }
}
static ClassRecord single_race_class;

static void unregister_single_race() {
    unregister_class(&single_race_class);
}

static void register_single_race() {
    register_class(&single_race_class, "SingleRaceScreen", create_single_race, "GWidget");
    std::atexit(unregister_single_race);
}
namespace {
NFS_KERNEL_INITIALIZER(startup_register_single_race, ".CRT$XCU_NFS_00690138",
                       register_single_race);
} // namespace
} // namespace nfs::application::frontend
