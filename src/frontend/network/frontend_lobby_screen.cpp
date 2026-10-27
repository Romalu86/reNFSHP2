#include "../screens/frontend_auxiliary_screens.hpp"
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::invoke;

void __fastcall initialize_lobby_screen(LobbyScreen *screen, void *) {
    gui::widgets::pending::request_interface_layout(&screen->base, nullptr);
    screen->players = reinterpret_cast<family::ListBox *>(
        network_page_child(screen, "<PLAYER_LIST>", family::list_box_class_key));
    screen->messages = reinterpret_cast<family::ListBox *>(
        network_page_child(screen, "<CHAT_LIST>", family::list_box_class_key));
    screen->chat = network_page_child(screen, "<CHAT>", family::edit_text_class_key);
    if (screen->chat) {
        invoke(screen->chat, 7, &lifecycle::construction::unresolved::shared_string);
        activate(screen->chat);
    }
}

void __fastcall lobby_screen_child(LobbyScreen *screen, void *, Widget *sender, Dword event, Dword,
                                   Dword) {
    if (event != 64)
        return;
    Widget *edit =
        sender && invoke<Dword>(sender, 2, family::edit_text_class_key) ? sender : nullptr;
    if (edit != screen->chat || !screen->messages)
        return;
    pending::LocalFrontendString text(invoke<const gui::String *>(screen->chat, 8));
    if (!text.value.length) {
        activate(screen->chat);
        return;
    }
    gui::String line;
    gui::prepend_ascii_value(&line, "Me: ", &text.value);
    try {
        auto *items = &screen->messages->items;
        invoke(items, 4, Dword(0));
        family::ListItem item;
        family::construct_text_list_item(&item, &line);
        try {
            invoke(items, 7, &item, Dword(0), Dword(0));
        } catch (...) {
            gui::string_destruct(&item.tooltip);
            gui::string_destruct(&item.text);
            throw;
        }
        gui::string_destruct(&item.tooltip);
        gui::string_destruct(&item.text);
        invoke(screen->chat, 7, &lifecycle::construction::unresolved::shared_string);
        activate(screen->chat);
    } catch (...) {
        gui::string_destruct(&line);
        throw;
    }
    gui::string_destruct(&line);
}
} // namespace nfs::application::frontend
