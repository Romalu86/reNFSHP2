#include "../screens/frontend_menu_roots.hpp"
#include "nfs/gui/lifecycle/gui_modal.hpp"
namespace nfs::network::session {
Dword pump_connection_dialog(void *);
}
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::invoke;

static void __cdecl tree_zoom_in_modal(void *object) {
    auto *widget = static_cast<TreeWidget *>(object);
    if (widget && invoke<Dword>(widget, 2, tree_widget_class_key)) {
        widget->base.base.drag_flags[0] = 1;
        widget->base.base.drag_flags[1] = 0;
        do {
            if (widget->base.base.drag_flags[1])
                break;
            network::session::pump_connection_dialog(widget);
        } while (widget->base.base.drag_flags[0]);
        family::finish_modal(reinterpret_cast<Widget *>(widget), 2, 0);
        return;
    }
    family::finish_modal(reinterpret_cast<Widget *>(widget), 7, 0);
}

void zoom_tree_selection_in(TreeWidget *widget) {
    auto *active =
        static_cast<Widget *>(gui::events::find_widget(gui::context_state, active_widget_id));
    gui::run_modal(widget, tree_zoom_in_modal);
    if (active)
        activate(active);
}

static void __cdecl tree_zoom_out_modal(void *object) {
    auto *widget = static_cast<TreeWidget *>(object);
    if (widget && invoke<Dword>(widget, 2, tree_widget_class_key)) {
        widget->base.base.drag_flags[0] = 0;
        widget->base.base.drag_flags[1] = 0;
        do {
            if (widget->base.base.drag_flags[1])
                break;
            network::session::pump_connection_dialog(widget);
        } while (!widget->base.base.drag_flags[0]);
        family::finish_modal(reinterpret_cast<Widget *>(widget), 2, 0);
        return;
    }
    family::finish_modal(reinterpret_cast<Widget *>(widget), 7, 0);
}

void zoom_tree_selection_out(TreeWidget *widget) {
    auto *active =
        static_cast<Widget *>(gui::events::find_widget(gui::context_state, active_widget_id));
    gui::run_modal(widget, tree_zoom_out_modal);
    if (active)
        activate(active);
}

void __cdecl wait_tree_exit(void *object) {
    auto *widget = static_cast<TreeWidget *>(object);
    while (widget->base.frame > 2)
        network::session::pump_connection_dialog(widget);
    family::finish_modal(reinterpret_cast<Widget *>(widget), 2, 0);
}
} // namespace nfs::application::frontend
