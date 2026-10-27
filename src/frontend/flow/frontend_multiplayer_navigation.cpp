#include "frontend_multiplayer.hpp"
#include "nfs/gui/lifecycle/gui_modal.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
namespace nfs::application::pending {
void *create_widget(const gui::String *, const gui::String *, const gui::String *, void *);
}
namespace nfs::gui::widgets {

Dword contains_widget_id(Widget *parent, Dword id) {
    if (!id)
        return 0;
    Widget *widget = static_cast<Widget *>(events::find_widget(context_state, id));
    while (widget) {
        widget = widget->parent;
        if (widget == parent)
            return 1;
    }
    return 0;
}
} // namespace nfs::gui::widgets
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::invoke;
using game::runtime::system;

void open_server_setup(MultiplayerScreen *screen) {
    Widget *dialog;
    {
        pending::LocalFrontendString type("ServerSetup"), name("<SERVER>"),
            resource("system\\GDialogs.gui");
        dialog = static_cast<Widget *>(
            pending::create_widget(&resource.value, &name.value, &type.value, screen));
        if (!dialog || !invoke<Dword>(dialog, 2, family::dialog_class_key))
            dialog = nullptr;
    }
    if (dialog) {
        gui::run_modal(dialog, nullptr);
        pending::destroy_screen(dialog);
    }
}

void change_multiplayer_page(MultiplayerScreen *screen) {
    bool keep_focus = false;
    if (screen->page && gui::events::find_widget(gui::context_state, active_widget_id)) {
        auto *active =
            static_cast<Widget *>(gui::events::find_widget(gui::context_state, active_widget_id));
        keep_focus = contains_widget_id(screen->page, active->id) != 0;
    }
    screen->selected_mode =
        int(network::packets::rule_value(static_cast<Dword *>(system(72)), 62));
    if (screen->page) {
        pending::destroy_screen(screen->page);
        screen->page = nullptr;
    }
    static constexpr const char *page_names[]{"Browser", "Browser", "DirectIP"};
    static constexpr const char *types[]{"BrowserView", "BrowserView", "PrivateView"};
    static constexpr Dword titles[]{1715, 1717, 1716};
    {
        pending::LocalFrontendString type(types[screen->selected_mode]),
            name(page_names[screen->selected_mode]), resource("System\\GInterface.gui");
        screen->page = static_cast<Widget *>(
            pending::create_widget(&resource.value, &name.value, &type.value, screen));
    }
    auto *heading = static_cast<Ticker *>(pending::find_child(screen, "<FOCUS_STATE>"));
    if (heading && invoke<Dword>(heading, 2, ticker_class_key)) {
        {
            pending::LocalFrontendString text;
            gui::string_reference(&text.value, titles[screen->selected_mode]);
            invoke(heading, 7, &text.value);
        }
        restart_multiplayer_heading(screen);
    }
    restart_multiplayer_heading(screen);
    if (screen->mode && screen->page)
        pending::set_widget_tab_order(screen->page, Dword(screen->mode->base.base.tab_index) + 1);
    if (keep_focus)
        activate_first_child(pending::widget_root(&screen->base));
    if (screen->selected_mode >= 0 && screen->selected_mode < 3)
        network::lobby::set_mode(screen->selected_mode);
}

void select_multiplayer_mode(MultiplayerScreen *screen, int mode) {
    if (network::packets::pending::session_kind != mode)
        network::lobby::set_mode(mode);
    if (screen->mode)
        select_type_spin(screen->mode, mode);
}

void restart_multiplayer_heading(MultiplayerScreen *screen) {
    auto *heading = static_cast<Ticker *>(pending::find_child(screen, "<FOCUS_STATE>"));
    if (!heading || !invoke<Dword>(heading, 2, ticker_class_key))
        return;
    clear_ticker_messages(heading);
    for (int index = 0; index < 6; ++index)
        queue_ticker_message(heading, invoke<const gui::String *>(heading, 8), 1,
                             &heading->base.base.colour, 0, 100.f, 0);
}

void set_multiplayer_heading(MultiplayerScreen *screen, Dword reference) {
    auto *heading = static_cast<Ticker *>(pending::find_child(screen, "<FOCUS_STATE>"));
    if (heading && invoke<Dword>(heading, 2, ticker_class_key)) {
        {
            pending::LocalFrontendString text;
            gui::string_reference(&text.value, reference);
            invoke(heading, 7, &text.value);
        }
        restart_multiplayer_heading(screen);
    }
}

void finish_browser_view(BrowserView *page, Dword result) {
    if (page->owner) {
        pending::clear_screen_queue(system(96));
        family::finish_modal(&page->owner->base, result, 0);
    }
}

void finish_private_view(PrivateView *page, Dword result) {
    if (page->owner) {
        pending::clear_screen_queue(system(96));
        family::finish_modal(&page->owner->base, result, 0);
    }
}
} // namespace nfs::application::frontend
