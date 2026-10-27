#include "frontend_network_pages.hpp"
#include "../../network/session/network_lifecycle_runtime.hpp"
#include "../../race/session/race_runtime.hpp"
namespace nfs::game::world::frame {
Dword read_keyboard_key(int);
}
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::at;
using game::runtime::bytes;
using game::runtime::invoke;
using game::runtime::system;
namespace {
const int lobby_columns[] = {1, 2, 7, 9};
const int racing_columns[] = {0, 1, 2, 4, 5, 6, 9};
const int results_columns[] = {0, 3, 4, 5, 6, 7, 9};
Dword phase_reference(void *game) {
    switch (at<std::uint16_t>(game, 8)) {
    case 1:
        return 1615;
    case 2:
        return 1616;
    case 3:
    case 4:
        return 1617;
    case 5:
        return 464;
    case 6:
        return 472;
    default:
        return 1618;
    }
}
void refresh_phase_and_car(NetworkStatistics *page, gui::String *text) {
    if (auto *phase = page->phase) {
        void *game = system(104);
        gui::string_copy(text, &lifecycle::construction::unresolved::shared_string);
        gui::string_reference(text, phase_reference(game));
        invoke(phase, 7, text);
    }
    if (auto *status = page->status) {
        gui::string_reference(text, 38);
        if (at<Dword>(system(112), 68) && at<int>(system(72), 204) != -1)
            gui::string_assign(text, network::players::vehicle_name(at<int>(system(112), 496)));
        invoke(status, 7, text);
    }
}
bool dedicated_server() {
    return at<Dword>(system(108), 44) && at<Dword>(system(108), 48);
}
} // namespace
void refresh_network_statistics_rows(NetworkStatistics *page) {
    lifecycle::construction::network::queue::visit(
        network::server_queue(system(104)), 0, reinterpret_cast<Dword>(page),
        reinterpret_cast<Dword *>(&statistics_visitor), 0, nullptr);
    family::sort_list_items(&page->players->items, 0, compare_network_positions);
}

void __fastcall initialize_network_statistics(NetworkStatistics *page, void *) {
    family::dialog_initialize(&page->base, nullptr);
    page->phase = network_page_child(page, "<PHASE>", family::text_class_key);
    page->status = network_page_child(page, "<STATUS>", family::text_class_key);
    page->rotation = network_page_child(page, "<ROTATE>", family::text_class_key);
    page->race_type = network_page_child(page, "<RACETYPE>", family::text_class_key);
    page->server_name = network_page_child(page, "<SERVERNAME>", family::text_class_key);
    page->car_class = network_page_child(page, "<CLASS>", family::text_class_key);
    page->track = network_page_child(page, "<TRACK>", family::text_class_key);
    page->direction = network_page_child(page, "<DIRECTION>", family::text_class_key);
    page->players = reinterpret_cast<family::ListBox *>(
        network_page_child(page, "<LIST>", family::list_box_class_key));
    page->next_column = page->column = 0;
    page->columns = nullptr;
    page->rotation_deadline = network::transport_time() + 1750;
    pending::LocalFrontendString text(&lifecycle::construction::unresolved::shared_string);
    refresh_phase_and_car(page, &text.value);
    if (auto *race_type = page->race_type) {
        gui::string_reference(&text.value, at<Dword>(system(72), 152) == 1 ? 22 : 19);
        invoke(race_type, 7, &text.value);
    }
    if (auto *server_name = page->server_name) {
        gui::string_assign(&text.value, reinterpret_cast<const char *>(bytes(system(104)) + 52));
        invoke(server_name, 7, &text.value);
    }
    if (auto *track_widget = page->track) {
        const Dword track = network::packets::rule_value(system(72), 5);
        gui::string_reference(&text.value, at<Dword>(at<void *>(system(40), 12 + track * 4), 12));
        invoke(track_widget, 7, &text.value);
    }
    if (auto *direction_widget = page->direction) {
        const Dword direction = network::packets::rule_value(system(72), 6);
        gui::string_copy(&text.value, &lifecycle::construction::unresolved::shared_string);
        gui::string_reference(&text.value, direction == 1   ? 693
                                           : direction == 2 ? 694
                                           : direction == 3 ? 695
                                                            : 692);
        invoke(direction_widget, 7, &text.value);
    }
    if (auto *car_class = page->car_class) {
        Dword reference;
        switch (network::packets::rule_value(system(72), 15)) {
        case 38:
            reference = 249;
            break;
        case 39:
            reference = 250;
            break;
        case 40:
            reference = 251;
            break;
        case 41:
            reference = 252;
            break;
        case 42:
            reference = 253;
            break;
        case 43:
            reference = 40;
            break;
        default:
            reference = 38;
            break;
        }
        gui::string_reference(&text.value, reference);
        invoke(car_class, 7, &text.value);
    }
    if (page->players) {
        rotate_network_statistic(page);
        refresh_network_statistics_rows(page);
    }
    page->base.flags |= 512;
}

int __cdecl compare_network_positions(const void *first, const void *second, Dword) {
    if (!first)
        return second != nullptr;
    if (!second)
        return -1;
    const Dword a = at<Dword>(const_cast<void *>(first), 24),
               b = at<Dword>(const_cast<void *>(second), 24);
    if (int(b) < int(a))
        return 1;
    return int(b) <= int(a) ? 0 : -1;
}

void rotate_network_statistic(NetworkStatistics *page) {
    const auto phase = at<std::int16_t>(system(104), 8);
    const int *columns = phase == 5 ? racing_columns : phase == 6 ? results_columns : lobby_columns;
    if (columns != page->columns) {
        page->next_column = 0;
        page->columns = columns;
    }
    page->column = page->columns[page->next_column++];
    if (page->column > 8) {
        page->next_column = 0;
        page->column = page->columns[0];
        page->next_column = 1;
    }
    if (page->rotation) {
        pending::LocalFrontendString text(
            &lifecycle::construction::unresolved::shared_string);
        Dword reference;
        switch (page->column) {
        case 0:
            reference = 353;
            break;
        case 2:
            reference = 1191;
            break;
        case 3:
            reference = 639;
            break;
        case 4:
            reference = 350;
            break;
        case 5:
            reference = 1391;
            break;
        case 6:
            reference = 1390;
            break;
        case 7:
            reference = 647;
            break;
        default:
            reference = 303;
            break;
        }
        gui::string_reference(&text.value, reference);
        invoke(page->rotation, 7, &text.value);
    }
}

void __fastcall tick_network_statistics(NetworkStatistics *page, void *, Dword elapsed) {
    pending::LocalFrontendString text(&lifecycle::construction::unresolved::shared_string);
    refresh_phase_and_car(page, &text.value);
    const Dword now = network::transport_time();
    if (page->rotation_deadline < now) {
        page->rotation_deadline = now + 1750;
        if (page->players) {
            rotate_network_statistic(page);
            invoke(&page->players->items, 1);
            refresh_network_statistics_rows(page);
        }
    }
    if (!dedicated_server() && !game::world::frame::read_keyboard_key(15) &&
        at<std::int16_t>(system(104), 8) != 6) {
        network_statistics_requested = 0;
        family::finish_modal(&page->base, 3, 0);
    }
    family::dialog_tick(&page->base, nullptr, elapsed);
}
// Store the value before forwarding the event.
void __fastcall clear_network_statistics_request(NetworkStatistics *page, void *) {
    network_statistics_requested = 0;
    gui::widgets::clear_layout_request(&page->base, nullptr);
}

Dword __fastcall network_statistics_key(NetworkStatistics *page, void *, Dword key, Dword modifiers) {
    return key == 15 ? Dword(0) : family::dialog_key_down(&page->base, nullptr, key, modifiers);
}

void __fastcall network_statistics_child(NetworkStatistics *page, void *, Widget *sender,
                                         Dword event, Dword first, Dword second) {
    family::dialog_configure_geometry(&page->base, nullptr, reinterpret_cast<Dword>(sender), event,
                                      first, second);
}

Dword __fastcall visit_network_statistics(void *, void *, network::ui::PlayerRecord *player, Dword,
                                         Dword, NetworkStatistics *page) {
    auto *items = &page->players->items;
    pending::LocalFrontendString text(&lifecycle::construction::unresolved::shared_string);
    family::ListItem item;
    profiles::construct_profile_list_item(&item);
    const Dword row = items->grid.rows.count;
    try {
        gui::string_assign(&text.value, player->name);
        {
            NetworkListItem source(&text.value);
            family::copy_list_item(&item, &source.value);
        }
        invoke(items, 7, &item, row, Dword(1));
        int position = 99;
        if (!(player->phase == 9 && player->slot >= 0 && at<std::int16_t>(system(104), 8) > 4 &&
              format_network_position(player->slot, &text.value, &position)))
            gui::string_assign(&text.value, "-");
        {
            NetworkListItem source(&text.value);
            family::copy_list_item(&item, &source.value);
        }
        item.data = Dword(position);
        invoke(items, 7, &item, row, Dword(0));
        format_network_statistic(player, page->column, &text.value);
        {
            NetworkListItem source(&text.value);
            family::copy_list_item(&item, &source.value);
        }
        invoke(items, 7, &item, row, Dword(2));
    } catch (...) {
        gui::string_destruct(&item.tooltip);
        gui::string_destruct(&item.text);
        throw;
    }
    gui::string_destruct(&item.tooltip);
    gui::string_destruct(&item.text);
    return 1;
}
} // namespace nfs::application::frontend
