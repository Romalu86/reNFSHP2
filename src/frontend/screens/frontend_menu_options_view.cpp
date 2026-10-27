#include "frontend_menu_controls.hpp"
#include "nfs/network/packets/network_packet_handlers.hpp"
namespace nfs::application::pending {
void *create_widget(const gui::String *, const gui::String *, const gui::String *, void *);
}
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::invoke;
using game::runtime::system;

static void reload_options_view(OptionsView *widget) {
    bool refocus = false;
    if (gui::events::find_widget(gui::context_state, active_widget_id)) {
        auto *active =
            static_cast<Widget *>(gui::events::find_widget(gui::context_state, active_widget_id));
        refocus = descendant(&widget->base.base, active->id) != nullptr;
    }
    widget->selected_tab = int(network::packets::rule_value(system(72), 28));
    if (widget->content) {
        pending::destroy_screen(widget->content);
        widget->content = nullptr;
    }
    {
        const auto &page = option_pages[widget->selected_tab];
        pending::LocalFrontendString type(page.type), name(page.name),
            file("System\\GInterface.gui");
        widget->content = static_cast<Widget *>(
            pending::create_widget(&file.value, &name.value, &type.value, widget));
    }
    auto *parent = widget->base.base.parent;
    if (invoke<Dword>(parent, 3, "OptionsScreen")) {
        if (invoke<Dword>(parent, 2, options_screen_class_key)) {
            set_options_heading(parent, option_pages[widget->selected_tab].title);
            update_options_heading(widget, 0, 0, 0);
        }
        auto *view = static_cast<Widget *>(pending::find_child(parent, "<VIEW>"));
        if (!view || !invoke<Dword>(view, 2, options_view_class_key))
            view = nullptr;
        if (view) {
            if (widget->selected_tab == 6)
                view->flags &= ~Dword(4);
            else
                view->flags |= 4;
        }
        const Dword profile = invoke<Dword>(widget->content, 3, "DriverProfileScreen") != 0;
        if (auto *button = static_cast<Widget *>(pending::find_child(parent, "<btna>"))) {
            set_enabled(button, profile);
            pending::set_widget_visible(button, profile);
        }
        if (auto *text = static_cast<Widget *>(pending::find_child(parent, "<ta>"))) {
            set_enabled(text, profile);
            pending::set_widget_visible(text, profile);
        }
    }
    if (refocus)
        activate_first_child(pending::widget_root(&widget->base.base));
}

void __fastcall initialize_options_view(OptionsView *widget, void *) {
    gui::widgets::pending::request_general_layout(&widget->base, nullptr);
    reload_options_view(widget);
}

void __fastcall update_options_view(OptionsView *widget, void *, Dword) {
    if (widget->selected_tab != int(network::packets::rule_value(system(72), 28)))
        reload_options_view(widget);
}
} // namespace nfs::application::frontend
