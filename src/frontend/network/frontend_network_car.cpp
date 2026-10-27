#include "frontend_network_pages.hpp"
#include "../../vehicles/rendering/vehicle_renderable_resources.hpp"
#include "../../network/session/network_lifecycle_runtime.hpp"
#include <cstring>
namespace nfs::network::session {

void send_client_selection(void *owner) {
    if (at<Dword>(owner, 60) && at<std::int16_t>(system(104), 8) == 2) {
        alignas(4) Byte packet[88]; // Byte1 is not written.
        packet[0] = 22;
        at<std::uint16_t>(packet, 2) = at<std::uint16_t>(owner, 376);
        std::memcpy(packet + 4, bytes(owner) + 436, 84);
        at<std::uint16_t>(owner, 380) = 9;
        at<Dword>(owner, 2092) = 0;
        send_packet(reinterpret_cast<Dword>(bytes(owner) + 72), reinterpret_cast<Dword>(packet),
                    sizeof(packet), 0);
    }
}

void request_spectator_mode(void *owner) {
    if (at<std::int16_t>(system(104), 8) == 5) {
        alignas(4) Byte packet[24]; // Bytes1..3/7/12..23 are not written.
        packet[0] = 43;
        at<std::uint16_t>(packet, 4) = at<std::uint16_t>(owner, 376);
        packet[6] = 1;
        at<Dword>(packet, 8) = 0;
        send_packet(reinterpret_cast<Dword>(bytes(owner) + 72), reinterpret_cast<Dword>(packet),
                    sizeof(packet), 0);
    }
}
} // namespace nfs::network::session
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::at;
using game::runtime::bytes;
using game::runtime::invoke;
using game::runtime::system;
namespace {
const Dword selectable_cars[24] = {7,  9,  21, 17, 18, 3, 22, 14, 23, 13, 5,  6,
                                  20, 15, 0,  19, 1,  8, 2,  16, 4,  12, 10, 11};
family::ListItem *selected_car_item(family::ListBox *cars) {
    auto *items = &cars->items;
    using Fetch = family::ListItem *(__thiscall *)(void *, Dword, Dword);
    const auto fetch = (*reinterpret_cast<const Fetch **>(items))[9];
    const Dword row = family::selected_list_row(items);
    return fetch(items, row, 0);
}
} // namespace

void __fastcall initialize_client_car_selection(ClientCarSelection *page, void *) {
    family::dialog_initialize(&page->base, nullptr);
    page->select = network_page_child(page, "<SELECT>", family::button_class_key);
    page->cancel = network_page_child(page, "<CANCEL>", family::button_class_key);
    page->observe = network_page_child(page, "<OBSERVE>", family::button_class_key);
    page->transmission = reinterpret_cast<ProfileToggle *>(
        network_page_child(page, "<TRANNY>", profiles::profile_list_class_key));
    page->cars = reinterpret_cast<family::ListBox *>(
        network_page_child(page, "<CARLIST>", family::list_box_class_key));
    page->colours = reinterpret_cast<ColourWheel *>(
        network_page_child(page, "<COLOUR>", colour_wheel_class_key));
    pending::LocalFrontendString text(&lifecycle::construction::unresolved::shared_string);
    auto *selected_cars = page->cars;
    if (selected_cars) {
        for (Dword car : selectable_cars) {
            const Dword allowed = network::packets::rule_value(system(72), 15);
            if ((allowed == Dword(network::packets::vehicle_class(
                                reinterpret_cast<Dword *>(bytes(system(52)) + 4), int(car))) ||
                 allowed == 6) &&
                car != 7) {
                family::ListItem item;
                construct_ascii_item(&item, network::players::vehicle_name(int(car)), car);
                try {
                    invoke(&page->cars->items, 8, &item);
                } catch (...) {
                    gui::string_destruct(&item.tooltip);
                    gui::string_destruct(&item.text);
                    throw;
                }
                gui::string_destruct(&item.tooltip);
                gui::string_destruct(&item.text);
            }
        }
        selected_cars = page->cars;
        invoke(&selected_cars->items, 10, Dword(0), Dword(1));
        gui::events::dispatch(&selected_cars->base.base, 68, 0, 0);
    }
    if (page->colours) {
        family::move_to_first_draw_position(&page->colours->base.base);
        set_enabled(&page->colours->base.base, 1);
        pending::set_widget_visible(page->colours, 1);
        if (auto *item = selected_car_item(selected_cars))
            refresh_client_car_colours(page, item->data);
    }
    if (page->transmission) {
        pending::LocalFrontendString label;
        gui::string_reference(&label.value, 690);
        {
            NetworkListItem item(&label.value, 1);
            invoke(&page->transmission->items, 8, &item.value);
        }
        gui::string_reference(&label.value, 1);
        {
            NetworkListItem item(&label.value, 0);
            invoke(&page->transmission->items, 8, &item.value);
        }
        profiles::select_profile_row(page->transmission, 0);
    }
}

void refresh_client_car_colours(ClientCarSelection *page, Dword car) {
    if (!page->colours)
        return;
    clear_colour_wheel(page->colours);
    auto *catalogue = system(52);
    void *record = bytes(catalogue) + 8 + car * 1228;
    for (int skin = 0; skin < at<int>(record, 60); ++skin) {
        auto *entry = game::world::vehicle_render::catalogue_skin(system(52), car, Dword(skin));
        pending::LocalFrontendString text;
        gui::string_reference(&text.value, at<Dword>(entry, 4));
        if (gui::string_equal(&text.value,
                              &lifecycle::construction::unresolved::shared_string))
            gui::string_assign(&text.value, "Not Described");
        if (at<int>(entry, 0) != -1) {
            pending::LocalFrontendString copy(&text.value);
            append_colour_wheel(page->colours, copy.release(), at<Dword>(entry, 8));
        }
    }
}

void __fastcall tick_client_car_selection(ClientCarSelection *page, void *, Dword elapsed) {
    pending::LocalFrontendString text(&lifecycle::construction::unresolved::shared_string);
    if (at<std::int16_t>(system(104), 8) != 2)
        family::finish_modal(&page->base, 12, 0);
    family::dialog_tick(&page->base, nullptr, elapsed);
}

void __fastcall client_car_selection_child(ClientCarSelection *page, void *, Widget *sender,
                                           Dword event, Dword first, Dword second) {
    if (event == 40 || event == 68) {
        auto *list =
            sender && invoke<Dword>(sender, 2, family::list_box_class_key) ? sender : nullptr;
        if (list == reinterpret_cast<Widget *>(page->cars))
            if (auto *item = selected_car_item(page->cars)) {
                if (event == 40)
                    family::finish_modal(&page->base, 2, 0);
                else
                    refresh_client_car_colours(page, item->data);
            }
    } else if (event == 43) {
        auto *button =
            sender && invoke<Dword>(sender, 2, family::button_class_key) ? sender : nullptr;
        if (button == page->select)
            family::finish_modal(&page->base, 2, 0);
        else if (button == page->observe) {
            auto *client = system(112);
            if (at<std::int16_t>(client, 380) == 9 && at<std::int16_t>(client, 378) >= 0) {
                Dword packet = reinterpret_cast<Dword>(sender);
                at<std::uint16_t>(&packet, 2) = at<std::uint16_t>(client, 376);
                at<std::uint8_t>(&packet, 0) = 23;
                network::send_packet(reinterpret_cast<Dword>(bytes(client) + 72),
                                     reinterpret_cast<Dword>(&packet), 4, 0);
            }
            family::finish_modal(&page->base, 15, 0);
        } else if (button == page->cancel)
            family::finish_modal(&page->base, 3, 0);
        else if (sender)
            invoke<Dword>(sender, 2, family::toggle_button_class_key);
    }
    family::dialog_configure_geometry(&page->base, nullptr, reinterpret_cast<Dword>(sender), event,
                                      first, second);
}

Dword __fastcall close_client_car_selection(ClientCarSelection *page, void *, Dword result, Dword) {
    if (result == 2) {
        network::ui::CarSettings settings{}; // Clear all 21 DWORDs.
        at<Dword>(&settings, 0) = 1;
        at<Dword>(&settings, 68) = 1;
        if (page->cars) {
            auto *item = selected_car_item(page->cars);
            at<Dword>(&settings, 60) = item ? item->data : Dword(-1);
        }
        if (page->colours)
            at<Dword>(&settings, 64) = at<Dword>(page->colours, 292);
        if (page->transmission)
            at<Dword>(&settings, 68) = selected_toggle_value(page->transmission) != 0;
        pending::LocalFrontendString message;
        auto *client = system(112);
        if (!at<Dword>(client, 60)) {
            std::memcpy(bytes(client) + 436, &settings, sizeof(settings));
            {
                pending::LocalFrontendString name(&at<gui::String>(client, 4));
                gui::string_make_narrow(&name.value);
                std::strncpy(reinterpret_cast<char *>(bytes(client) + 460),
                             gui::string_ascii(&name.value), 26);
                at<std::uint8_t>(client, 485) = 0;
                at<Dword>(client, 60) = 1;
                network::session::send_client_selection(client);
            }
            gui::string_reference(&message.value, 1612);
        } else
            gui::string_reference(&message.value, 1614);
        pending::LocalFrontendString copy(&message.value);
        auto *game = network::network_game();
        if (auto *frontend = at<Dword *>(game, 92))
            network::ui::notice(frontend, nullptr, &copy.value, -1, 0, 0, 0);
    }
    page->base.modal_result = result;
    return 1;
}
} // namespace nfs::application::frontend
