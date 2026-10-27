#include "frontend_network_pages.hpp"
#include "../../network/session/network_bandwidth.hpp"
#include "../../network/session/server_runtime.hpp"
#include "nfs/network/packets/network_packet_handlers.hpp"
#include <cstring>
namespace nfs::network::packets::pending {
extern int session_kind;
}
namespace nfs::services::pending {
void activate_server(void *, Dword);
}
namespace nfs::application::pending {
void send_race_text(void *, Dword, Dword, Dword, Dword, Dword, Dword);
}
namespace nfs::game::runtime::ai::cars {
gui::String *profile_display_name(void *, gui::String *);
}
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::at;
using game::runtime::bytes;
using game::runtime::invoke;
using game::runtime::system;
namespace {
Widget *widget(ProfileToggle *value) {
    return reinterpret_cast<Widget *>(value);
}
void add_choice(ProfileToggle *toggle, Dword reference, Dword value) {
    pending::LocalFrontendString text;
    gui::string_reference(&text.value, reference);
    NetworkListItem item(&text.value, value);
    invoke(&toggle->items, 8, &item.value);
}
template <std::size_t Size> void add_choices(ProfileToggle *toggle, const Dword (&values)[Size][2]) {
    for (const auto &row : values)
        add_choice(toggle, row[0], row[1]);
}
const Dword track_choices[][2] = {{40, 12}, {256, 0},  {257, 1}, {258, 2}, {270, 3},
                                 {271, 4}, {272, 5},  {266, 6}, {267, 7}, {268, 8},
                                 {263, 9}, {264, 10}, {265, 11}};
const Dword class_choices[][2] = {{38, 6},    {249, 38}, {250, 39},
                                 {251, 40}, {252, 41}, {253, 42}};
const Dword race_choices[][2] = {{19, 0}, {22, 1}};
const Dword direction_choices[][2] = {{692, 0}, {693, 1}, {694, 2}, {695, 3}, {40, 4}};
} // namespace

void __fastcall initialize_server_setup(ServerSetup *page, void *) {
    family::dialog_initialize(&page->base, nullptr);
    page->accept = network_page_child(page, "<ACCEPT>", family::button_class_key);
    page->cancel = network_page_child(page, "<CANCEL>", family::button_class_key);
    page->dedicated = network_page_child(page, "<DEDICATED>", family::button_class_key);
    const auto toggle = [&](const char *name) {
        return reinterpret_cast<ProfileToggle *>(
            network_page_child(page, name, profiles::profile_list_class_key));
    };
    page->track = toggle("<TRACK>");
    page->direction = toggle("<DIRECTION>");
    page->laps = toggle("<TLAPS>");
    page->maximum_clients = toggle("<TMAXCLIENTS>");
    page->collisions = toggle("<COLLISIONS>");
    page->race_type = toggle("<RACETYPE>");
    page->car_class = toggle("<CLASS>");
    page->name = network_page_child(page, "<SERVERNAME>", family::edit_text_class_key);
    page->password = network_page_child(page, "<PASSWORD>", family::edit_text_class_key);
    auto *settings = services::pending::server_settings;
    services::pending::load_server_settings(settings);
    pending::LocalFrontendString text(&lifecycle::construction::unresolved::shared_string);
    if (page->track) {
        add_choices(page->track, track_choices);
        select_toggle_value(page->track, at<Dword>(settings, 524));
    }
    if (page->car_class) {
        add_choices(page->car_class, class_choices);
        profiles::select_profile_row(page->car_class, 0);
        select_toggle_value(page->car_class, at<Dword>(settings, 688));
    }
    if (page->race_type) {
        add_choices(page->race_type, race_choices);
        profiles::select_profile_row(page->race_type, 0);
        if (at<Dword>(settings, 680) == 1)
            profiles::select_profile_row(page->race_type, 1);
    }
    if (page->direction) {
        add_choices(page->direction, direction_choices);
        profiles::select_profile_row(page->direction, 0);
        select_toggle_value(page->direction, at<Dword>(settings, 684));
    }
    if (page->collisions) {
        add_choice(page->collisions, 445, 0);
        add_choice(page->collisions, 444, 1);
        profiles::select_profile_row(page->collisions, at<Dword>(system(108), 136) != 0);
    }
    if (page->laps) {
        for (int lap = 1; lap < 10; ++lap) {
            gui::string_format_ascii(&text.value, "%d", lap);
            NetworkListItem item(&text.value, Dword(lap));
            invoke(&page->laps->items, 8, &item.value);
        }
        profiles::select_profile_row(page->laps, 0);
        if (at<Dword>(settings, 672))
            select_toggle_value(page->laps, at<Dword>(settings, 672));
    }
    Dword maximum = 8;
    if (network::packets::pending::session_kind == 1) {
        Dword rate = pending::server_password_enabled;
        if (!rate)
            rate = network::bandwidth::limit();
        if (!rate || rate < 38)
            maximum = 4;
        else if (rate < 60)
            maximum = 5;
        else if (rate < 94)
            maximum = 6;
        else
            maximum = (rate > 174) + 7;
    }
    if (page->maximum_clients) {
        for (Dword count = 2; int(count) <= int(maximum); ++count) {
            gui::string_format_ascii(&text.value, "%d", int(count));
            NetworkListItem item(&text.value, count);
            invoke(&page->maximum_clients->items, 8, &item.value);
        }
        select_toggle_value(page->maximum_clients, maximum);
    }
    if (page->name) {
        if (!*reinterpret_cast<const char *>(settings)) {
            gui::String name;
            game::runtime::ai::cars::profile_display_name(
                pending::player_preferences(system(72), 0), &name);
            try {
                gui::string_copy(&text.value, &name);
            } catch (...) {
                gui::string_destruct(&name);
                throw;
            }
            gui::string_destruct(&name);
            gui::string_append_ascii(&text.value, "'s Game");
        } else
            gui::string_format_ascii(&text.value, "%s", reinterpret_cast<const char *>(settings));
        invoke(page->name, 7, &text.value);
    }
    if (page->password) {
        gui::string_copy(&text.value, &lifecycle::construction::unresolved::shared_string);
        if (at<Dword>(system(108), 44))
            gui::string_format_ascii(&text.value, "%s",
                                     reinterpret_cast<const char *>(settings + 632));
        invoke(page->password, 7, &text.value);
    }
    if (page->dedicated)
        invoke(page->dedicated, 47, at<Dword>(settings, 692));
    if (page->accept && at<Dword>(system(108), 44) && at<std::int16_t>(system(104), 8) != 2)
        network_reference(page->accept, 1795);
}

void __fastcall tick_server_setup(ServerSetup *page, void *, Dword elapsed) {
    pending::LocalFrontendString text(&lifecycle::construction::unresolved::shared_string);
    if (page->track && page->laps && page->race_type && page->maximum_clients) {
        const Dword track = selected_toggle_value(page->track);
        if (track == 5 || track == 11 || track == 8 || track == 2) {
            select_toggle_value(page->laps, 1);
            set_enabled(widget(page->laps), 0);
            select_toggle_value(page->race_type, 0);
            set_enabled(widget(page->race_type), 0);
            at<Dword>(services::pending::server_settings, 672) = 1;
        } else {
            set_enabled(widget(page->race_type), 1);
            if (selected_toggle_value(page->race_type) == 1) {
                int laps = int(selected_toggle_value(page->maximum_clients) - 1);
                if (laps < 1)
                    laps = 1;
                select_toggle_value(page->laps, Dword(laps));
                set_enabled(widget(page->laps), 0);
            } else
                set_enabled(widget(page->laps), 1);
        }
    }
    family::dialog_tick(&page->base, nullptr, elapsed);
}

void __fastcall server_setup_child(ServerSetup *page, void *, Widget *sender, Dword event,
                                   Dword first, Dword second) {
    if (event == 43) {
        auto *button =
            sender && invoke<Dword>(sender, 2, family::button_class_key) ? sender : nullptr;
        if (button == page->accept)
            family::finish_modal(&page->base, 2, 0);
        else if (button == page->cancel)
            family::finish_modal(&page->base, 3, 0);
        else {
            auto *toggle = sender && invoke<Dword>(sender, 2, family::toggle_button_class_key)
                               ? sender
                               : nullptr;
            if (toggle == widget(page->race_type))
                select_toggle_value(page->laps,
                                    at<Dword>(services::pending::server_settings, 672));
            else if (toggle == widget(page->laps))
                at<Dword>(services::pending::server_settings, 672) =
                    selected_toggle_value(page->laps);
        }
    }
    family::dialog_configure_geometry(&page->base, nullptr, reinterpret_cast<Dword>(sender), event,
                                      first, second);
}

Dword __fastcall close_server_setup(ServerSetup *page, void *, Dword result, Dword) {
    if (result == 2) {
        auto *server = system(108);
        if (page->race_type)
            at<Dword>(server, 56) = selected_toggle_value(page->race_type);
        if (page->car_class)
            at<Dword>(server, 60) = selected_toggle_value(page->car_class);
        if (page->direction)
            at<Dword>(server, 68) = selected_toggle_value(page->direction);
        if (page->laps)
            at<Dword>(server, 72) = selected_toggle_value(page->laps);
        if (page->maximum_clients)
            at<std::uint8_t>(server, 96) =
                std::uint8_t(selected_toggle_value(page->maximum_clients));
        if (page->collisions)
            at<Dword>(server, 136) = selected_toggle_value(page->collisions);
        if (page->name) {
            pending::LocalFrontendString text(invoke<const gui::String *>(page->name, 8));
            const char *name = "Unnamed NFS HP2 Server";
            if (!gui::string_equal(&text.value,
                                   &lifecycle::construction::unresolved::shared_string) &&
                text.value.length)
                name = gui::string_ascii(invoke<const gui::String *>(page->name, 8));
            std::strncpy(reinterpret_cast<char *>(bytes(server) + 172), name, 24);
        }
        network::packets::set_server_name(system(104),
                                          reinterpret_cast<const char *>(bytes(server) + 172));
        if (auto *track = page->track) {
            at<Dword>(services::pending::server_settings, 524) =
                selected_toggle_value(track);
            at<Dword>(server, 64) = selected_toggle_value(track);
        }
        auto *password = page->password;
        if (password &&
            !gui::string_equal(invoke<const gui::String *>(password, 8),
                               &lifecycle::construction::unresolved::shared_string)) {
            if (!invoke<const gui::String *>(password, 8)->wide)
                std::strncpy(reinterpret_cast<char *>(bytes(server) + 196),
                             gui::string_ascii(invoke<const gui::String *>(password, 8)), 24);
        }
        auto *current_server = system(108);
        if (!at<Dword>(current_server, 44))
            services::pending::activate_server(
                current_server, page->dedicated ? at<Dword>(page->dedicated, 300) : 0);
        else {
            pending::send_race_text(current_server, 1637, Dword(-2), 0, 0, 0, 0);
            pending::rotate_server_race(system(108));
        }
    }
    page->base.modal_result = result;
    return 1;
}
} // namespace nfs::application::frontend
