#include "nfs/network/packets/network_packet_handlers.hpp"
#include <cstring>
namespace nfs::network::packets {
char local_player_name[80]{};

void set_local_name(const char *name) {
    std::strncpy(local_player_name, name, 80);
    local_player_name[79] = 0;
}

const char *get_local_name() {
    return local_player_name;
}

void player_update(void *packet) {
    ui::PlayerRecord player;
    ui::initialize_car_settings(&player.car);
    auto *key = bytes(packet) + 2;
    if (!copy_player(key, &player))
        return;
    switch (at<Byte>(packet, 0)) {
    case 15:
        at<Dword>(&player, 122) = at<Dword>(packet, 4);
        at<Dword>(&player, 126) = at<Dword>(packet, 8);
        break;
    case 34:
        at<Byte>(&player, 120) = at<Byte>(packet, 4);
        break;
    case 44:
        at<Dword>(&player, 132) = at<Dword>(packet, 4);
        break;
    }
    replace_player(key, &player);
}

void formatted_notice(void *packet) {
    ScopedString text;
    gui::string_copy(&text.value, &lifecycle::construction::unresolved::shared_string);
    const Byte flags = at<Byte>(packet, 16);
    if (!(flags & 1))
        gui::string_reference(&text.value, at<Dword>(packet, 4));
    else if (!(flags & 2)) {
        ScopedString pattern;
        gui::string_reference(&pattern.value, at<Dword>(packet, 4));
        gui::string_format(&text.value, &pattern.value, at<Dword>(packet, 8));
    } else {
        ui::PlayerRecord player;
        ui::initialize_car_settings(&player.car);
        if (!copy_player(bytes(packet) + 8, &player))
            return;
        ScopedString name, pattern;
        gui::string_assign(&name.value, player.name);
        gui::string_make_wide(&name.value);
        gui::string_reference(&pattern.value, at<Dword>(packet, 4));
        gui::string_format(&text.value, &pattern.value, gui::string_data(&name.value));
    }
    if (!gui::string_equal(&text.value,
                           &lifecycle::construction::unresolved::shared_string))
        notice(&text.value, at<std::int16_t>(packet, 12));
}

void rename_player(Dword *owner, void *packet) {
    ui::PlayerRecord player;
    ui::initialize_car_settings(&player.car);
    ScopedString text;
    auto *key = bytes(packet) + 2;
    const auto *new_name = reinterpret_cast<char *>(packet) + 4;
    if (!at<Dword>(server(), 44) && copy_player(key, &player)) {
        ScopedString before, after, pattern;
        gui::string_assign(&before.value, player.name);
        gui::string_assign(&after.value, new_name);
        gui::string_make_wide(&before.value);
        gui::string_make_wide(&after.value);
        gui::string_reference(&pattern.value, 1350);
        gui::string_format(&text.value, &pattern.value, gui::string_data(&before.value),
                           gui::string_data(&after.value));
        std::strncpy(player.name, new_name, 24);
        player.name[23] = 0;
        replace_player(key, &player);
        notice(&text.value);
    }
    if (at<std::uint16_t>(packet, 2) == at<std::uint16_t>(owner, 376)) {
        ScopedString changed, name;
        gui::string_assign(&changed.value, new_name);
        gui::string_copy(&name.value, &changed.value);
        if (!gui::string_equal(&name.value,
                               &lifecycle::construction::unresolved::shared_string)) {
            gui::string_copy(reinterpret_cast<gui::String *>(bytes(owner) + 4), &name.value);
            ScopedString narrow;
            gui::string_copy(&narrow.value, &name.value);
            gui::string_make_narrow(&narrow.value);
            set_local_name(gui::string_ascii(&narrow.value));
            ScopedString preference;
            gui::string_copy(&preference.value, &narrow.value);
            if (gui::string_make_narrow(&preference.value) &&
                string_character(&preference.value, 0)) {
                std::strncpy(reinterpret_cast<char *>(system(72)) + 2514,
                             gui::string_ascii(&preference.value), 25);
                at<Byte>(system(72), 2513) = 0; // Clear the preceding flag byte.
            } else
                at<Byte>(system(72), 2514) = 0;
        }
    }
}

void add_player(Dword *owner, std::uint16_t identifier, const char *name, std::uint16_t route) {
    ui::PlayerRecord player;
    ui::initialize_car_settings(&player.car);
    ScopedString text;
    std::memset(&player, 0, sizeof(player));
    if (!queue::copy_matching(owner, address_of(&identifier), address_of(&player), nullptr)) {
        player.identifier = identifier | (Dword(route) << 16);
        player.phase = 7;
        std::strncpy(player.name, name, 24);
        player.name[23] = 0;
        player.slot = -1;
        queue::append(owner, address_of(&player), address_of("listnode"));
        ScopedString wide_name, pattern;
        gui::string_assign(&wide_name.value, name);
        gui::string_make_wide(&wide_name.value);
        gui::string_reference(&pattern.value, 1626);
        gui::string_format(&text.value, &pattern.value, gui::string_data(&wide_name.value));
    } else {
        at<std::uint16_t>(&player, 2) = route;
        if (std::strcmp(player.name, name)) {
            ScopedString before, after, pattern;
            gui::string_assign(&before.value, player.name);
            gui::string_assign(&after.value, name);
            gui::string_make_wide(&before.value);
            gui::string_make_wide(&after.value);
            gui::string_reference(&pattern.value, 1350);
            gui::string_format(&text.value, &pattern.value, gui::string_data(&before.value),
                               gui::string_data(&after.value));
        }
        std::strncpy(player.name, name, 24);
        player.name[23] = 0;
        queue::replace(owner, address_of(&identifier), address_of(&player), nullptr);
    }
    gui::String owned;
    gui::string_construct_copy(&owned, &text.value);
    players::notify_game(network_game(), owned, -1, 0);
}

void broadcast_chat(Dword *owner, const char *text, void *target) {
    unsigned length = static_cast<unsigned>(std::strlen(text));
    if (static_cast<int>(length) > 128)
        length = 128;
    const int size = length + 12;
    auto *packet = static_cast<Byte *>(
        files::archives::allocate_entry(size, "ChatMsg", gui::application_pool));
    packet[0] = 13;
    at<Dword>(packet, 4) = 0;
    std::strncpy(reinterpret_cast<char *>(packet + 8), text, length);
    packet[length + 8] = 0;
    at<std::uint16_t>(packet, 2) = 65534;
    if (target)
        send(target, packet, size);
    else
        broadcast(owner, packet, size, 0, nullptr);
    gui::application_release(packet);
}
} // namespace nfs::network::packets
