#include "nfs/network/session/network_gui.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
namespace nfs::network::ui {
namespace {
void delete_first(void *widget) {
    auto *list = static_cast<std::uint8_t *>(widget) + 264;
    using Remove = void(__thiscall *)(void *, Dword);
    reinterpret_cast<const Remove *>(*reinterpret_cast<Dword *>(list))[5](list, 0);
}
void append(void *widget, const Message *message) {
    auto *list = static_cast<std::uint8_t *>(widget) + 264;
    using Append = void(__thiscall *)(void *, const Message *);
    reinterpret_cast<const Append *>(*reinterpret_cast<Dword *>(list))[8](list, message);
}
} // namespace

void __fastcall notice(Dword *owner, void *, const gui::String *text, std::int16_t sender,
                       int client_prefix, int mute_log, int mute_chat) {
    if (!text->length)
        return;
    if ((console_available() & 255) && server()[44 / 4]) {
        gui::String temporary;
        gui::string_construct_copy(&temporary, text);
        try {
            if (gui::string_make_narrow(&temporary))
                console_print(gui::string_ascii(&temporary));
        } catch (...) {
            gui::string_destruct(&temporary);
            throw;
        }
        gui::string_destruct(&temporary);
    }
    if (sender == -3 && field<Dword>(owner, 264)) {
        auto *target = field<Dword *>(owner, 264);
        using Print = void(__thiscall *)(Dword *, const gui::String *);
        reinterpret_cast<const Print *>(target[0])[7](target, text);
        field<Dword>(owner, 312) = transport_time() + 5000;
        return;
    }
    gui::String line;
    gui::string_construct_copy(&line, &lifecycle::construction::unresolved::shared_string);
    try {
        if (sender >= 0) {
            PlayerRecord player;
            initialize_car_settings(&player.car);
            Dword *game = network_game();
            if (lifecycle::construction::network::queue::copy_matching(
                    server_queue(game), reinterpret_cast<Dword>(&sender),
                    reinterpret_cast<Dword>(&player), nullptr)) {
                gui::string_append_ascii(&line, player.name);
                if (client_prefix && client()[68 / 4]) {
                    gui::string_append_reference(&line, 733);
                    gui::string_append_ascii(&line, "[");
                    gui::String name;
                    gui::copy_member_string(client(), &name);
                    try {
                        gui::string_append(&line, &name);
                    } catch (...) {
                        gui::string_destruct(&name);
                        throw;
                    }
                    gui::string_destruct(&name);
                    gui::string_append_ascii(&line, "]");
                }
                gui::string_append_reference(&line, 733);
                gui::string_append_reference(&line, 722);
                gui::string_append_reference(&line, 733);
            }
        }
        gui::string_append(&line, text);
        Message message;
        construct_message(&message, &line);
        try {
            message.sender = sender;
            void *log = field<void *>(owner, 288);
            if (log && !mute_log) {
                if (field<int>(log, 280) > 99) {
                    delete_first(log);
                    --field<int>(owner, 300);
                }
                append(log, &message);
                if (field<int>(owner, 300) < 0)
                    field<Dword>(owner, 300) = 0;
            }
            void *chat = field<void *>(owner, 284);
            if (chat && !mute_chat && sender > -2) {
                if (field<int>(chat, 280) > 99)
                    delete_first(chat);
                append(chat, &message);
            }
        } catch (...) {
            destroy_message(&message);
            throw;
        }
        destroy_message(&message);
    } catch (...) {
        gui::string_destruct(&line);
        throw;
    }
    gui::string_destruct(&line);
}
} // namespace nfs::network::ui
