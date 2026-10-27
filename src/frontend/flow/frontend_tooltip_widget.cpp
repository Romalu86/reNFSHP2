#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "../../gui/widgets/gui_widget_shared_methods.hpp"
#include "frontend_application_widgets.hpp"
#include "../../gui/widgets/gui_widget_resources.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include <cstdlib>
#include <cstring>
namespace nfs::gui::engine {
void inflate_rectangle(Rectangle *, int);
}
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::at;
using game::runtime::invoke;
extern const std::array<Method, 47> tooltip_methods;

static const char *__cdecl tooltip_class_key() {
    return "ToolTip";
}

static const char *__fastcall tooltip_name(Widget *, void *) {
    return "ToolTip";
}

static Dword __fastcall tooltip_is_type(Widget *widget, void *, const void *type) {
    return type == reinterpret_cast<const void *>(tooltip_class_key) ||
           reinterpret_cast<Dword(__thiscall *)(Widget *, const void *)>(
               family::tool_tip_methods[2])(widget, type);
}

static Dword __fastcall tooltip_is_name(Widget *widget, void *, const char *name) {
    return !std::strncmp(name, "ToolTip", 8) ||
           reinterpret_cast<Dword(__thiscall *)(Widget *, const char *)>(
               family::tool_tip_methods[3])(widget, name);
}

static void destroy_tooltip(Widget *widget) {
    widget->methods = tooltip_methods.data();
    family::tool_tip_destroy(widget);
}

static Widget *__fastcall delete_tooltip(Widget *widget, void *, Dword flags) {
    destroy_tooltip(widget);
    if (flags & 1)
        gui::release(widget);
    return widget;
}

static void __fastcall prepare_tooltip(Widget *widget, void *, Dword render) {
    family::tool_tip_prepare_draw(widget, nullptr, render);
    if (family::tooltip_owner != widget->id || at<Dword>(widget, 264) != 2)
        return;
    const int ticks = at<int>(widget, 268);
    const int width_fraction = int(Dword(ticks) * 100u) / 30;
    const int height_fraction = int(Dword(ticks) * 100u) / 15;
    const int width_percent = width_fraction < 0 ? 0 : width_fraction > 100 ? 100 : width_fraction;
    const int height_percent = height_fraction < 0     ? 0
                               : height_fraction > 100 ? 100
                                                       : height_fraction;
    auto *text = static_cast<Widget *>(application::pending::find_child(widget, "<TEXT>"));
    auto *back = static_cast<Widget *>(application::pending::find_child(widget, "<BACK>"));
    if (!text || !back)
        return;
    invoke(text, 6, Dword(64));
    gui::engine::Rectangle bounds{gui::engine::rectangle_methods, text->bounds.top,
                                  text->bounds.left, text->bounds.width, text->bounds.height};
    gui::engine::inflate_rectangle(&bounds, at<int>(widget, 288));
    widget->bounds.width = bounds.width;
    widget->bounds.height = bounds.height;
    invoke(text, 6, Dword(48));
    invoke(back, 6, Dword(128));
    const int height = int(Dword(bounds.height) * Dword(height_percent)) / 100;
    back->bounds.height = height;
    widget->bounds.width = int(Dword(bounds.width) * Dword(width_percent)) / 100;
    widget->bounds.height = height;
    gui::engine::destroy_rectangle(&bounds);
}

const std::array<Method, 47> tooltip_methods = {
    reinterpret_cast<Method>(::nfs::application::frontend::delete_tooltip), // 0.
    reinterpret_cast<Method>(::nfs::application::frontend::tooltip_name), // 1.
    reinterpret_cast<Method>(::nfs::application::frontend::tooltip_is_type), // 2.
    reinterpret_cast<Method>(::nfs::application::frontend::tooltip_is_name), // 3.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::tool_tip_load), // 4.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::tool_tip_save), // 5.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::layout), // 6.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_text), // 7.
    reinterpret_cast<Method>(::nfs::gui::widgets::text), // 8.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::hit_test), // 9.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::navigate), // 10.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::enter_modal), // 11.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::leave_modal), // 12.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::tool_tip_event), // 13.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_four_arguments), // 14.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_two_arguments), // 15.
    reinterpret_cast<Method>(::nfs::gui::widgets::apply_drawing_state), // 16.
    reinterpret_cast<Method>(::nfs::application::frontend::prepare_tooltip), // 17.
    reinterpret_cast<Method>(::nfs::gui::widgets::push_draw_bounds), // 18.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::tool_tip_draw), // 19.
    reinterpret_cast<Method>(::nfs::gui::widgets::pop_draw_bounds), // 20.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::tool_tip_initialize), // 21.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_no_arguments), // 22.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_render_argument), // 23.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::tool_tip_tick), // 24.
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

static void *create_tooltip(void *parent) {
    void *storage = gui::allocate_flags(292, 0, "ToolTip");
    if (!storage)
        return nullptr;
    try {
        gui::widgets::pending::construct_tool_tip(storage, parent);
        at<Dword>(storage, 288) = 5;
        static_cast<Widget *>(storage)->methods = tooltip_methods.data();
    } catch (...) {
        gui::release(storage);
        throw;
    }
    return storage;
}
static ClassRecord tooltip_class;

static void unregister_tooltip() {
    unregister_class(&tooltip_class);
}

static void register_tooltip() {
    register_class(&tooltip_class, "ToolTip", create_tooltip, "GToolTip");
    std::atexit(unregister_tooltip);
}
namespace {
NFS_KERNEL_INITIALIZER(startup_register_tooltip, ".CRT$XCU_NFS_0069014C",
                       register_tooltip);
} // namespace
} // namespace nfs::application::frontend
