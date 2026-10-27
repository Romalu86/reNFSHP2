#include "../../gui/widgets/gui_widget_shared_methods.hpp"
#include "frontend_scene_widgets.hpp"
#include <cstdlib>
#include <cstring>
namespace nfs::application::frontend {
using namespace gui::widgets;

const char *__cdecl track_selection_class_key() {
    return "TrackSelectScreen";
}

static const char *__fastcall track_selection_name(Widget *, void *) {
    return "TrackSelectScreen";
}

static Dword __fastcall track_selection_is_type(Widget *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(track_selection_class_key) ||
           type == reinterpret_cast<const void *>(interface_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

static Dword __fastcall track_selection_is_name(Widget *, void *, const char *name) {
    return !std::strncmp(name, "TrackSelectScreen", 18) || !std::strncmp(name, "GInterface", 11) ||
           !std::strncmp(name, "GUI", 4);
}

static TrackSelection *__fastcall delete_track_selection(TrackSelection *page, void *, Dword flags) {
    destroy_track_selection(page);
    if (flags & 1)
        gui::release(page);
    return page;
}

const std::array<Method, 42> track_selection_methods = {
    reinterpret_cast<Method>(::nfs::application::frontend::delete_track_selection), // 0.
    reinterpret_cast<Method>(::nfs::application::frontend::track_selection_name), // 1.
    reinterpret_cast<Method>(::nfs::application::frontend::track_selection_is_type), // 2.
    reinterpret_cast<Method>(::nfs::application::frontend::track_selection_is_name), // 3.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::load_interface), // 4.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::save_interface), // 5.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::layout), // 6.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_text), // 7.
    reinterpret_cast<Method>(::nfs::gui::widgets::text), // 8.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::hit_test), // 9.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::navigate), // 10.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::enter_interface), // 11.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::leave_modal), // 12.
    reinterpret_cast<Method>(::nfs::application::frontend::track_selection_event), // 13.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_four_arguments), // 14.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_two_arguments), // 15.
    reinterpret_cast<Method>(::nfs::gui::widgets::apply_drawing_state), // 16.
    reinterpret_cast<Method>(::nfs::gui::widgets::prepare_draw), // 17.
    reinterpret_cast<Method>(::nfs::gui::widgets::push_draw_bounds), // 18.
    reinterpret_cast<Method>(::nfs::gui::widgets::draw_content), // 19.
    reinterpret_cast<Method>(::nfs::gui::widgets::pop_draw_bounds), // 20.
    reinterpret_cast<Method>(::nfs::application::frontend::initialize_track_selection), // 21.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_no_arguments), // 22.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_render_argument), // 23.
    reinterpret_cast<Method>(::nfs::application::frontend::update_track_selection), // 24.
    reinterpret_cast<Method>(::nfs::gui::widgets::clear_layout_request), // 25.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_layout_argument), // 26.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_parent_enter), // 27.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_parent_leave), // 28.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_parent_argument), // 29.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_child_argument), // 30.
    reinterpret_cast<Method>(::nfs::application::frontend::finish_track_selection), // 31.
    reinterpret_cast<Method>(::nfs::application::frontend::track_selection_key), // 32.
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

void *create_track_selection(void *parent) {
    auto *page = static_cast<TrackSelection *>(
        gui::allocate_flags(sizeof(TrackSelection), 0, "TrackSelectScreen"));
    if (!page)
        return nullptr;
    try {
        construct_track_selection(page, parent);
        reinterpret_cast<Widget *>(page)->methods = track_selection_methods.data();
        return page;
    } catch (...) {
        gui::release_flags(page, 0, "TrackSelectScreen");
        throw;
    }
}
} // namespace nfs::application::frontend
