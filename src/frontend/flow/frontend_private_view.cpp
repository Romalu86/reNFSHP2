#include "frontend_multiplayer.hpp"
#include "../../gui/widgets/gui_widget_shared_methods.hpp"
#include "../../network/session/server_runtime.hpp"
namespace nfs::gui::widgets::family {
ListItem *construct_ascii_list_item(ListItem *, const char *);
}
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::at;
using game::runtime::invoke;
using game::runtime::system;

PrivateView *construct_private_view(PrivateView *page, void *parent) {
    construct_widget(page, parent);
    page->base.methods = private_view_methods.data();
    page->owner = nullptr;
    page->host = page->join = page->address = nullptr;
    page->recent = nullptr;
    return page;
}

void destroy_private_view(PrivateView *page) {
    page->base.methods = private_view_methods.data();
    destroy_general(reinterpret_cast<GeneralWidget *>(page));
}

void __fastcall initialize_private_view(PrivateView *page, void *) {
    Widget *parent = page->base.parent;
    page->owner = parent && invoke<Dword>(parent, 3, "MultiplayerScreen") &&
                          invoke<Dword>(parent, 2, multiplayer_class_key)
                      ? reinterpret_cast<MultiplayerScreen *>(parent)
                      : nullptr;
    page->address = network_page_child(page, "<IP>", family::edit_text_class_key);
    if (page->address) {
        pending::LocalFrontendString text(network::lobby::recent_address(0));
        invoke(page->address, 7, &text.value);
    }
    page->host = network_page_child(page, "<HOST>", family::button_class_key);
    page->join = network_page_child(page, "<JOIN>", family::button_class_key);
    page->recent = reinterpret_cast<family::ListBox *>(
        network_page_child(page, "<IPS>", family::list_box_class_key));
    if (page->recent)
        for (int row = 0; row < 5; ++row) {
            family::ListItem item;
            family::construct_ascii_list_item(&item, network::lobby::recent_address(row));
            try {
                invoke(&page->recent->items, 7, &item, row, 0);
            } catch (...) {
                gui::string_destruct(&item.tooltip);
                gui::string_destruct(&item.text);
                throw;
            }
            gui::string_destruct(&item.tooltip);
            gui::string_destruct(&item.text);
        }
    gui::widgets::pending::request_general_layout(reinterpret_cast<GeneralWidget *>(page), nullptr);
}

void __fastcall tick_private_view(PrivateView *, void *, Dword) {
    if (!pending::server_password_enabled)
        network::bandwidth::poll();
}

void __fastcall private_view_child(PrivateView *page, void *, Widget *sender, Dword event, Dword,
                                   Dword) {
    if (event == 40) {
        Widget *list =
            sender && invoke<Dword>(sender, 2, family::list_box_class_key) ? sender : nullptr;
        Widget *edit =
            sender && invoke<Dword>(sender, 2, family::edit_text_class_key) ? sender : nullptr;
        if (page->address &&
            (list == reinterpret_cast<Widget *>(page->recent) || edit == page->address))
            connect_private_server(page);
    } else if (event == 43) {
        Widget *button =
            sender && invoke<Dword>(sender, 2, family::button_class_key) ? sender : nullptr;
        if (button == page->host) {
            MultiplayerScreen *owner = page->owner;
            if (owner)
                open_server_setup(owner);
            const Dword active = at<Dword>(system(108), 44);
            if (active) {
                if (!active || !at<Dword>(system(108), 48))
                    services::pending::connect_client(system(112), nullptr, 0, 0, 0);
                if (owner) {
                    pending::clear_screen_queue(system(96));
                    family::finish_modal(&owner->base, 2, 0);
                }
            }
        } else if (button == page->join && page->address)
            connect_private_server(page);
    } else if (event == 68) {
        Widget *list =
            sender && invoke<Dword>(sender, 2, family::list_box_class_key) ? sender : nullptr;
        if (list == reinterpret_cast<Widget *>(page->recent)) {
            auto *item = invoke<family::ListItem *>(
                &page->recent->items, 9, family::selected_list_row(&page->recent->items), 0);
            if (item && page->address)
                invoke(page->address, 7, &item->text);
        }
    }
}
} // namespace nfs::application::frontend
