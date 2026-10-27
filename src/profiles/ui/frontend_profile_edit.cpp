#include "../../gui/widgets/gui_widget_shared_methods.hpp"
#include "../../frontend/flow/frontend_application_widgets.hpp"
#include "../../gui/widgets/gui_widget_edit.hpp"
#include <cstdlib>
#include <cstring>
namespace nfs::gui::engine {
void destroy_coordinate(Point *);
Point *top_left(Point *, const Rectangle *);
}
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::invoke;
extern const std::array<Method, 55> profile_edit_methods;

static const char *__cdecl profile_edit_class_key() {
    return "HP2EditText";
}

static const char *__fastcall profile_edit_name(Widget *, void *) {
    return "HP2EditText";
}

static Dword __fastcall profile_edit_is_type(Widget *widget, void *, const void *type) {
    return type == reinterpret_cast<const void *>(profile_edit_class_key) ||
           reinterpret_cast<Dword(__thiscall *)(Widget *, const void *)>(
               family::edit_text_methods[2])(widget, type);
}

static Dword __fastcall profile_edit_is_name(Widget *widget, void *, const char *name) {
    return !std::strncmp(name, "HP2EditText", 12) ||
           reinterpret_cast<Dword(__thiscall *)(Widget *, const char *)>(
               family::edit_text_methods[3])(widget, name);
}

static void destroy_profile_edit(Widget *widget) {
    widget->methods = profile_edit_methods.data();
    family::edit_text_destroy(widget);
}

static Widget *__fastcall delete_profile_edit(Widget *widget, void *, Dword flags) {
    destroy_profile_edit(widget);
    if (flags & 1)
        gui::release(widget);
    return widget;
}
// Point is passed by value after the renderer pointer.
static void __fastcall draw_profile_caret(Widget *widget, void *, gui::engine::Engine *render,
                                          Dword point_methods, Dword x, Dword y) {
    struct CoordinateOwner {
        gui::engine::Point *point;
        ~CoordinateOwner() { gui::engine::destroy_coordinate(point); }
    };
    gui::engine::Point start{reinterpret_cast<const gui::Destroy *>(point_methods), int(x), int(y)};
    CoordinateOwner start_owner{&start};
    if (invoke<Dword>(render, 59))
        return;
    const Dword height = invoke<Dword>(render, 17);
    if (!height)
        return;
    {
        gui::engine::Point origin;
        gui::engine::top_left(&origin, gui::engine::current_viewport_bounds(&render->base));
        CoordinateOwner origin_owner{&origin};
        start.x = int(Dword(start.x) + Dword(origin.x));
        start.y = int(Dword(start.y) + Dword(origin.y));
    }
    gui::engine::Point end;
    gui::engine::construct_point(&end);
    CoordinateOwner end_owner{&end};
    end.x = start.x;
    end.y = int(Dword(start.y) + height);
    const Dword colour = widget->colour;
    invoke(render, 46, &start, &end, colour, colour);
    start.x = int(Dword(start.x) + 1);
    end.x = int(Dword(end.x) + 1);
    invoke(render, 46, &start, &end, colour, colour);
}

const std::array<Method, 55> profile_edit_methods = {
    reinterpret_cast<Method>(::nfs::application::frontend::delete_profile_edit), // 0.
    reinterpret_cast<Method>(::nfs::application::frontend::profile_edit_name), // 1.
    reinterpret_cast<Method>(::nfs::application::frontend::profile_edit_is_type), // 2.
    reinterpret_cast<Method>(::nfs::application::frontend::profile_edit_is_name), // 3.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::edit_text_load), // 4.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::edit_text_save), // 5.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::text_layout), // 6.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_text), // 7.
    reinterpret_cast<Method>(::nfs::gui::widgets::text), // 8.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::hit_test), // 9.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::navigate), // 10.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::enter_modal), // 11.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::leave_modal), // 12.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::edit_text_event), // 13.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_four_arguments), // 14.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_two_arguments), // 15.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::edit_text_drawing_state), // 16.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::edit_text_prepare_draw), // 17.
    reinterpret_cast<Method>(::nfs::gui::widgets::push_draw_bounds), // 18.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::edit_text_draw), // 19.
    reinterpret_cast<Method>(::nfs::gui::widgets::pop_draw_bounds), // 20.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::request_general_layout), // 21.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_no_arguments), // 22.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_render_argument), // 23.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_style_argument), // 24.
    reinterpret_cast<Method>(::nfs::gui::widgets::clear_layout_request), // 25.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_layout_argument), // 26.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::edit_text_parent_enter), // 27.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::edit_text_parent_leave), // 28.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_parent_argument), // 29.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_child_argument), // 30.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_modal_state), // 31.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::edit_text_key_down), // 32.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_result), // 33.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_34), // 34.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::edit_text_character), // 35.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::edit_text_button_down), // 36.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_37), // 37.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_38), // 38.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::edit_text_pointer_move), // 39.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::edit_text_pointer_press), // 40.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::edit_text_pointer_release), // 41.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::edit_text_accept), // 42.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::edit_text_cancel), // 43.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_scroll), // 44.
    reinterpret_cast<Method>(::nfs::gui::widgets::process_general_command), // 45.
    reinterpret_cast<Method>(::nfs::gui::widgets::process_general_notification), // 46.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::draw_edit_line), // 47.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::reject_edit_character), // 48.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::begin_edit_session), // 49.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::edit_text_item_operation_50), // 50.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::edit_text_item_operation_51), // 51.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::validate_edit_text), // 52.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::edit_text_item_operation_53), // 53.
    reinterpret_cast<Method>(::nfs::application::frontend::draw_profile_caret), // 54.
};

void *create_profile_edit(void *parent) {
    auto *storage = static_cast<Widget *>(gui::allocate_flags(328, 0, "HP2EditText"));
    if (!storage)
        return nullptr;
    try {
        gui::widgets::pending::construct_edit_text(storage, parent);
        storage->methods = profile_edit_methods.data();
        return storage;
    } catch (...) {
        gui::release_flags(storage, 0, "HP2EditText");
        throw;
    }
}
} // namespace nfs::application::frontend
