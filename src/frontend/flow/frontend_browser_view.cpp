#include "frontend_multiplayer.hpp"
#include "../../gui/widgets/gui_widget_shared_methods.hpp"
#include "../../network/session/server_runtime.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include <cstring>
namespace nfs::gui::widgets::family {
int __cdecl compare_list_text(const void *, const void *, Dword);
}
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::at;
using game::runtime::invoke;
using game::runtime::system;
namespace {
Dword host_clock{}, host_clock_guard{};
}

BrowserView *construct_browser_view(BrowserView *page, void *parent) {
    construct_widget(page, parent);
    page->base.methods = browser_view_methods.data();
    page->maximum_servers = 1000;
    std::fill_n(page->columns, 6, -1);
    page->owner = nullptr;
    page->host = page->join = page->refresh = nullptr;
    page->servers = nullptr;
    page->track = page->car_class = page->dedicated = page->bandwidth = nullptr;
    std::fill_n(page->sort_buttons, 6, nullptr);
    return page;
}

void destroy_browser_view(BrowserView *page) {
    page->base.methods = browser_view_methods.data();
    destroy_general(reinterpret_cast<GeneralWidget *>(page));
}

void __fastcall initialize_browser_view(BrowserView *page, void *) {
    page->host = network_page_child(page, "<HOST>", family::button_class_key);
    page->join = network_page_child(page, "<JOIN>", family::button_class_key);
    page->refresh = network_page_child(page, "<REFRESH>", family::button_class_key);
    static constexpr const char *sort_names[]{"<STEXT1>", "<STEXT2>", "<STEXT3>",
                                              "<STEXT4>", "<STEXT5>", "<STEXT6>"};
    for (int index = 0; index < 6; ++index)
        page->sort_buttons[index] =
            network_page_child(page, sort_names[index], family::button_class_key);
    page->servers = reinterpret_cast<family::ListBox *>(
        network_page_child(page, "<SERVERLIST>", family::list_box_class_key));
    page->maximum_servers = 1000;
    for (int index = 0; index < 6; ++index)
        page->columns[index] = index;
    page->track = network_page_child(page, "<TRACK>", family::text_class_key);
    page->car_class = network_page_child(page, "<CLASS>", family::text_class_key);
    page->dedicated = network_page_child(page, "<DEDICATED>", family::text_class_key);
    clear_browser_details(page);
    page->selection = nullptr;
    Widget *parent = page->base.parent;
    page->owner = parent && invoke<Dword>(parent, 3, "MultiplayerScreen") &&
                          invoke<Dword>(parent, 2, multiplayer_class_key)
                      ? reinterpret_cast<MultiplayerScreen *>(parent)
                      : nullptr;
    if (auto *refresh = page->refresh) {
        set_enabled(refresh, 0);
        pending::set_widget_visible(refresh, 0);
    }
    gui::widgets::pending::request_general_layout(reinterpret_cast<GeneralWidget *>(page), nullptr);
}

void __fastcall tick_browser_view(BrowserView *page, void *, Dword) {
    if (auto *refresh = page->refresh) {
        const Dword internet = network::packets::pending::session_kind == 1;
        set_enabled(refresh, internet);
        pending::set_widget_visible(refresh, internet);
        const bool busy = network::lobby::query_phase || network::lobby::queued_master ||
                          network::lobby::queued_lan;
        {
            pending::LocalFrontendString text;
            gui::string_reference(&text.value,
                                  network::packets::pending::session_kind == 1 && busy ? 1781
                                                                                      : 1192);
            invoke(page->refresh, 7, &text.value);
        }
        set_enabled(page->refresh, network::lobby::query_phase != 3);
    }
    if (page->join) {
        set_enabled(page->join, page->selection != nullptr);
        pending::LocalFrontendString text;
        gui::string_reference(&text.value, 1181);
        invoke(page->join, 7, &text.value);
    }
    if (page->host) {
        bool enabled = true;
        if (network::packets::pending::session_kind == 1 && !pending::server_password_enabled) {
            if (!(host_clock_guard & 1)) {
                host_clock_guard |= 1;
                host_clock = GetTickCount();
            }
            if (network::bandwidth::probe_phase == 2)
                host_clock = GetTickCount();
            else {
                const Dword delta = GetTickCount() - host_clock;
                const Dword sign = Dword(std::int32_t(delta) >> 31);
                if (std::int32_t((delta ^ sign) - sign) < 5000)
                    enabled = false;
            }
        }
        set_enabled(page->host, enabled);
    }
    if (!page->selection)
        clear_browser_details(page);
    network::lobby::tick();
}

void __fastcall leave_browser_view(BrowserView *page, void *) {
    clear_browser_servers(page);
    clear_layout_request(&page->base, nullptr);
}

void __fastcall browser_view_child(BrowserView *page, void *, Widget *sender, Dword event, Dword,
                                   Dword) {
    if (event == 40) {
        Widget *list =
            sender && invoke<Dword>(sender, 2, family::list_box_class_key) ? sender : nullptr;
        if (list == reinterpret_cast<Widget *>(page->servers))
            connect_selected_server(page);
    } else if (event == 43) {
        Widget *button =
            sender && invoke<Dword>(sender, 2, family::button_class_key) ? sender : nullptr;
        if (button == page->host) {
            if (page->owner)
                open_server_setup(page->owner);
            const Dword active = at<Dword>(system(108), 44);
            if (!active)
                return;
            if (!active || !at<Dword>(system(108), 48))
                services::pending::connect_client(system(112), nullptr, 0, 0, 0);
            finish_browser_view(page, 2);
        } else if (button == page->join)
            connect_selected_server(page);
        else if (button == page->refresh) {
            if (!network::lobby::query_phase && !network::lobby::queued_master &&
                !network::lobby::queued_lan) {
                network::lobby::clear();
                network::lobby::queued_lan = network::lobby::queued_master = 1;
            } else {
                network::lobby::queued_lan = network::lobby::queued_master = 0;
                network::lobby::cancel();
            }
        } else {
            static constexpr int order[]{0, 3, 4, 2, 5, 1};
            for (int index = 0; index < 6; ++index)
                if (button == page->sort_buttons[index]) {
                    if (page->servers)
                        family::sort_list_items(&page->servers->items, page->columns[order[index]],
                                                index == 3 ? compare_server_ping
                                                           : family::compare_list_text);
                    return;
                }
        }
    } else if (event == 68) {
        Widget *list =
            sender && invoke<Dword>(sender, 2, family::list_box_class_key) ? sender : nullptr;
        if (list != reinterpret_cast<Widget *>(page->servers))
            return;
        auto *item = invoke<family::ListItem *>(&page->servers->items, 9,
                                                family::selected_list_row(&page->servers->items),
                                                page->columns[0]);
        if (!item)
            return;
        page->selection = reinterpret_cast<void *>(item->data);
        if (!page->selection)
            return;
        pending::LocalFrontendString unused(
            &lifecycle::construction::unresolved::shared_string);
        if (network::packets::pending::session_kind == 1)
            network::lobby::request_server(page->selection);
        else
            refresh_browser_details(page, page->selection);
    }
}
} // namespace nfs::application::frontend
