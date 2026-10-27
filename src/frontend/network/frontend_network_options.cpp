#include "frontend_network_pages.hpp"
#include "../../network/session/network_lifecycle_runtime.hpp"
#include "nfs/core/configuration/launch_options.hpp"
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::at;
using game::runtime::invoke;
using game::runtime::system;
std::uint8_t &network_statistics_requested = game::phase_pending;  
NetworkListItem::NetworkListItem(const gui::String *text) {
    gui::widgets::family::construct_text_list_item(&value, text);
}
NetworkListItem::NetworkListItem(const gui::String *text, Dword data) {
    construct_toggle_item(&value, text, data);
}
NetworkListItem::~NetworkListItem() {
    gui::string_destruct(&value.tooltip);
    gui::string_destruct(&value.text);
    value.methods = reinterpret_cast<const Method *>(gui::base_methods);
}
Widget *network_page_child(void *page, const char *name, TypeName key) {
    auto *widget = static_cast<Widget *>(pending::find_child(page, name));
    return widget && invoke<Dword>(widget, 2, key) ? widget : nullptr;
}
static bool dedicated_server() {
    auto *server = system(108);
    return at<Dword>(server, 44) && at<Dword>(server, 48);
}

void __fastcall initialize_network_options(NetworkOptions *page, void *) {
    family::dialog_initialize(&page->base, nullptr);
    page->server = network_page_child(page, "<SERVER>", family::button_class_key);
    if (page->server)
        set_enabled(page->server, enabled_in_ancestors(page->server));
    page->car = network_page_child(page, "<CAR>", family::button_class_key);
    page->audio = network_page_child(page, "<AUDIO>", family::button_class_key);
    page->disconnect = network_page_child(page, "<DISCONNECT>", family::button_class_key);
    page->close = network_page_child(page, "<OK>", family::button_class_key);
    page->hud = network_page_child(page, "<HUD>", family::button_class_key);
    if (page->audio)
        set_enabled(page->audio, !dedicated_server());
    if (page->hud)
        set_enabled(page->hud, !dedicated_server());
    if (page->server)
        set_enabled(page->server, at<Dword>(system(108), 44));
    if (page->close && dedicated_server())
        network_reference(page->close, 1782);
}

void __fastcall tick_network_options(NetworkOptions *page, void *, Dword elapsed) {
    if (page->car) {
        if (!at<Dword>(system(112), 68))
            set_enabled(page->car, 0);
        else {
            set_enabled(page->car, at<std::int16_t>(system(104), 8) > 1);
            network_reference(page->car, at<std::int16_t>(system(104), 8) < 3 ? 1601 : 1670);
        }
    }
    family::dialog_tick(&page->base, nullptr, elapsed);
}

void __fastcall network_options_child(NetworkOptions *page, void *, Widget *sender, Dword event,
                                      Dword first, Dword second) {
    if (event == 43) {
        auto *button =
            sender && invoke<Dword>(sender, 2, family::button_class_key) ? sender : nullptr;
        Dword next = 0;
        if (button == page->close) {
            family::finish_modal(&page->base, 2, 0);
            if (dedicated_server()) {
                network_statistics_requested = 1;
                next = 112;
            }
        } else if (button == page->car) {
            if (at<std::int16_t>(system(104), 8) != 2)
                network::session::request_spectator_mode(system(112));
            else {
                family::finish_modal(&page->base, 2, 0);
                next = 111;
            }
        } else if (button == page->server) {
            family::finish_modal(&page->base, 2, 0);
            if (at<Dword>(system(108), 44))
                next = 114;
        } else if (button == page->disconnect) {
            if (sender->parent) {
                int result = 2;
                if (!launch::field<Dword>(8627776)) {
                    pending::MessageBox box;
                    {
                        pending::LocalFrontendString title, text;
                        gui::string_reference(&title.value, 558);
                        gui::string_reference(&text.value, 1719);
                        pending::construct_message_box(&box, &text.value, &title.value, 81986);
                    }
                    try {
                        result = pending::run_message_box(&box, nullptr, 0);
                    } catch (...) {
                        pending::destroy_message_box(&box);
                        throw;
                    }
                    pending::destroy_message_box(&box);
                }
                if (result == 2) {
                    const Dword client_active = at<Dword>(system(112), 68);
                    if (client_active)
                        network::disconnect_client(system(112), 16);
                    const Dword server_active = at<Dword>(system(108), 44);
                    if (server_active)
                        network::leave_game(system(108));
                    family::finish_modal(&page->base, 2, 0);
                    at<Dword>(system(96), 36) = 19;
                }
            }
        } else if (button == page->audio) {
            family::finish_modal(&page->base, 2, 0);
            next = 116;
        } else if (button == page->hud) {
            family::finish_modal(&page->base, 2, 0);
            next = 115;
        }
        if (next)
            gui::events::propagate(at<void *>(system(96), 40), next, 0, 0, 2);
    }
    family::dialog_configure_geometry(&page->base, nullptr, reinterpret_cast<Dword>(sender), event,
                                      first, second);
}
 
void __fastcall leave_network_options(NetworkOptions *page, void *) {
    gui::widgets::clear_layout_request(&page->base,nullptr);
}

Dword __fastcall close_network_options(NetworkOptions *page, void *, Dword result, Dword) {
    if (result == 3 && dedicated_server()) {
        network_statistics_requested = 1;
        gui::events::propagate(at<void *>(system(96), 40), 112, 0, 0, 2);
    }
    page->base.modal_result = result;
    return 1;
}
} // namespace nfs::application::frontend
