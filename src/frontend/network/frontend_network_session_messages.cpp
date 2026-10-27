#include "frontend_network_session.hpp"
#include "nfs/physics/frame/simulation_runtime.hpp"
#include <mmsystem.h>
namespace nfs::application::frontend {
using game::runtime::at;
using game::runtime::bytes;
using game::runtime::invoke;
using game::runtime::system;
Dword menu_list_data(gui::widgets::family::ListItems *, Dword, Dword);
const gui::String *menu_list_text(gui::widgets::family::ListItems *, Dword, Dword);

void format_network_mode(gui::String *out) {
    gui::string_copy(out, &lifecycle::construction::unresolved::shared_string);
    if (at<Dword>(network::client(), 68) && at<int>(system(72), 204) != -1) {
        const int vehicle = at<int>(network::client(), 496);
        pending::LocalFrontendString name(network::players::vehicle_name(vehicle)), pattern;
        gui::string_make_wide(&name.value);
        gui::string_reference(&pattern.value, vehicle == 19 ? 1619 : 1620);
        gui::string_format(out, &pattern.value, gui::string_data(&name.value));
        return;
    }
    {
        pending::LocalFrontendString pattern;
        gui::string_reference(&pattern.value, 1621);
        gui::string_format(out, &pattern.value);
    }
    auto *car = game::runtime::physics::local_car(system(68), 0);
    if (car && at<Dword>(car, 1772) < 24) {
        auto *settings = bytes(system(72)) + 248 + at<Dword>(car, 1772) * 84;
        if (settings) {
            pending::LocalFrontendString name(reinterpret_cast<const char *>(settings + 24));
            gui::string_make_wide(&name.value);
            pending::LocalFrontendString pattern;
            gui::string_reference(&pattern.value, 1622);
            gui::string_format(out, &pattern.value, gui::string_data(&name.value));
        }
    }
}

void refresh_network_session_messages(NetworkSessionWidget *widget) {
    auto *empty = &lifecycle::construction::unresolved::shared_string;
    pending::LocalFrontendString text(empty);
    if (widget->mode && widget->mode_expiry < timeGetTime()) {
        if (at<Dword>(network::server(), 44) && at<Dword>(network::server(), 48)) {
            pending::LocalFrontendString pattern;
            gui::string_reference(&pattern.value, 1785);
            gui::string_format(&text.value, &pattern.value, 242);
        } else {
            const auto phase = at<std::int16_t>(network::network_game(), 8);
            if (phase == 5 && at<int>(system(72), 204) == -1)
                format_network_mode(&text.value);
            else {
                gui::string_copy(&text.value, empty);
                Dword reference;
                switch (phase) {
                case 1:
                    reference = 1615;
                    break;
                case 2:
                    reference = 1616;
                    break;
                case 3:
                case 4:
                    reference = 1617;
                    break;
                case 5:
                    reference = 464;
                    break;
                case 6:
                    reference = 472;
                    break;
                default:
                    reference = 1618;
                    break;
                }
                gui::string_reference(&text.value, reference);
            }
        }
        invoke(widget->mode, 7, &text.value);
        widget->mode_expiry = 0;
    }
    if (widget->system_message && widget->message_log) {
        int consumed = widget->consumed_messages;
        if (consumed < int(widget->message_log->items.grid.rows.count) &&
            widget->next_message < timeGetTime()) {
            widget->next_message = timeGetTime() + 200;
            bool found = true;
            do {
                widget->consumed_messages = ++consumed;
                if (consumed < 1)
                    widget->consumed_messages = 1;
                else if (int(widget->message_log->items.grid.rows.count) < consumed) {
                    widget->consumed_messages = int(widget->message_log->items.grid.rows.count);
                    found = false;
                    break;
                }
                consumed = widget->consumed_messages;
            } while (menu_list_data(&widget->message_log->items, Dword(consumed - 1), 0) != Dword(-2));
            if (found) {
                const Dword *methods = at<const Dword *>(widget->system_message, 0);
                const auto *message = menu_list_text(&widget->message_log->items,
                                                     Dword(widget->consumed_messages - 1), 0);
                using SetText = void(__thiscall *)(gui::widgets::Widget *, const gui::String *);
                reinterpret_cast<SetText>(methods[7])(widget->system_message, message);
            }
        } else if (widget->next_message + 5000 < timeGetTime())
            invoke(widget->system_message, 7, empty);
        if (at<Dword>(network::server(), 44) && at<Dword>(network::server(), 48) &&
            gui::string_equal(invoke<const gui::String *>(widget->system_message, 8), empty) &&
            !game::runtime::reset_complete && !game::runtime::reset_requested && !game::phase_reset)
            network_reference(widget->system_message, game::phase_pending ? 1783 : 1784);
    }
    if (auto *history = widget->chat_history) {
        const Dword now = timeGetTime();
        auto *items = &history->items;
        if (!items->grid.rows.count)
            widget->chat_expiry = now + 6000;
        else if (int(widget->chat_expiry - now) < 0) {
            widget->chat_expiry = now + (int(items->grid.rows.count) < 5 ? 6000 : 2000);
            invoke(items, 5, Dword(0));
        }
    }
}
} // namespace nfs::application::frontend
