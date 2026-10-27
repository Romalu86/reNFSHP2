#include "../widgets/gui_widget_controls.hpp"
#include "nfs/gui/events/gui_events.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
namespace nfs::gui::widgets::family {
extern Dword tooltips_enabled;
}
namespace nfs::application::pending {
void set_widget_visible(void *, kernel::Dword);
}
namespace nfs::gui::engine::pending {

void draw_overlay(void *render) {
    using namespace widgets;
    using game::runtime::at;
    using game::runtime::invoke;
    if (!family::tooltips_enabled || !family::tooltip_owner)
        return;
    auto *tooltip =
        static_cast<Widget *>(events::find_widget(context_state, family::tooltip_owner));
    if (tooltip && invoke<Dword>(tooltip, 2, family::tool_tip_class_key) &&
        at<Dword>(tooltip, 280) && at<Dword>(tooltip, 264) == 2) {
        application::pending::set_widget_visible(tooltip, 1);
        draw_root(tooltip, render, 1);
        application::pending::set_widget_visible(tooltip, 0);
    }
}
} // namespace nfs::gui::engine::pending
