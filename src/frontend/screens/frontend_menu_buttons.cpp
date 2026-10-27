#include "frontend_menu_controls.hpp"
#include "../../gui/lifecycle/gui_configuration_values.hpp"
#include "../../gui/lifecycle/gui_input.hpp"
namespace nfs::gui::widgets {

Dword widget_flag_matches(Widget *widget, Dword mask, Dword any) {
    return any ? (widget->flags & mask) != 0 : (widget->flags & mask) == mask;
}
} // namespace nfs::gui::widgets
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::invoke;
static Widget *focused_widget() {
    return static_cast<Widget *>(gui::events::find_widget(gui::context_state, active_widget_id));
}

void __fastcall activate_rollover(RolloverButton *widget, void *) {
    family::activate_button(&widget->base.base.base, nullptr);
    widget->latched = 1;
    if (auto *parent = widget->base.base.base.parent)
        if (widget_flag_matches(parent, 256, 1))
            activate(parent);
}

Dword __fastcall rollover_state(RolloverButton *widget, void *) {
    const Dword state = family::button_visual_state(&widget->base.base.base, nullptr);
    auto *parent = widget->base.base.base.parent;
    if (parent && widget_flag_matches(parent, 256, 1) && focused_widget() != parent)
        return state;
    if (!state && widget->latched)
        return 2;
    return state;
}

Dword __fastcall rollover_pointer_move(RolloverButton *widget, void *, Dword first, Dword second) {
    if (invoke<Dword>(widget, 9))
        widget->latched = 1;
    if (auto *parent = widget->base.base.base.parent)
        if (widget_flag_matches(parent, 256, 1)) {
            activate(parent);
            return 1;
        }
    return gui::widgets::pending::update_active_style(&widget->base.base.base, nullptr, first,
                                                      second);
}

void __fastcall update_rollover(RolloverButton *widget, void *, Dword) {
    if (!invoke<Dword>(widget, 9) ||
        gui::input::pointer_target(&widget->base.base.base) != widget->base.base.base.id)
        widget->latched = 0;
}

Dword __fastcall menu_spin_state(MenuSpinButton *widget, void *) {
    if (widget->highlight_ticks > 0)
        return 2;
    if (widget->base.base.base.parent && focused_widget())
        return 0;
    return 4;
}

void __fastcall update_menu_spin(MenuSpinButton *widget, void *, Dword ticks) {
    if (widget->highlight_ticks > 0)
        widget->highlight_ticks = int(Dword(widget->highlight_ticks) - ticks);
}

Dword __fastcall load_menu_spin(MenuSpinButton *widget, void *, void *configuration) {
    gui::engine::pending::read_int(configuration, "HP2SpinButton.m_HiLiteTime",
                                   &widget->highlight_duration, 0);
    return family::button_load(&widget->base.base.base, nullptr,
                               reinterpret_cast<Dword>(configuration));
}

Dword __fastcall save_menu_spin(MenuSpinButton *widget, void *, void *configuration) {
    gui::engine::pending::write_int(configuration, "HP2SpinButton.m_HiLiteTime",
                                    widget->highlight_duration, 0);
    return family::button_save(&widget->base.base.base, nullptr,
                               reinterpret_cast<Dword>(configuration));
}
void latch_menu_spin(Dword id) {
    if (auto *widget = static_cast<Widget *>(gui::events::find_widget(gui::context_state, id)))
        if (invoke<Dword>(widget, 2, menu_spin_class_key)) {
            auto *spin = reinterpret_cast<MenuSpinButton *>(widget);
            spin->highlight_ticks = spin->highlight_duration;
        }
}
} // namespace nfs::application::frontend
