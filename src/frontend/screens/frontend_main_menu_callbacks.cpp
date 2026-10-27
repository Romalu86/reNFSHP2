#include "../flow/frontend_application_widgets.hpp"
#include "../flow/frontend_runtime.hpp"
#include "../../gui/widgets/gui_widget_hooks.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include <cstdlib>
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::invoke;
using game::runtime::system;

static Dword __cdecl main_menu_quick_race(void *source, Dword event, Dword, Dword) {
    auto *widget = static_cast<Widget *>(source);
    if (widget && invoke<Dword>(widget, 2, family::button_class_key) && event == 43) {
        Widget *root = application::pending::widget_root(widget);
        if (root && invoke<Dword>(root, 2, interface_class_name)) {
            network::packets::pending::set_rule_variant(static_cast<Dword *>(system(72)),
                                                                 1, 0);
            family::finish_modal(root, 2, 0);
            application::pending::clear_screen_queue(system(96));
        }
    }
    return 0;
}

static Dword __cdecl main_menu_single_challenge(void *, Dword, Dword, Dword) {
    network::packets::pending::set_game_rule(static_cast<Dword *>(system(72)), 1, 1);
    if (!network::packets::rule_value(static_cast<Dword *>(system(72)), 14) &&
        network::packets::rule_value(static_cast<Dword *>(system(72)), 13) == 1)
        network::packets::pending::set_game_rule(static_cast<Dword *>(system(72)), 14, 1);
    return 0;
}

static Dword __cdecl main_menu_hot_pursuit(void *, Dword, Dword, Dword) {
    network::packets::pending::set_game_rule(static_cast<Dword *>(system(72)), 1, 2);
    return 0;
}

static Dword __cdecl main_menu_top_cop(void *, Dword, Dword, Dword) {
    network::packets::pending::set_game_rule(static_cast<Dword *>(system(72)), 1, 3);
    return 0;
}

static Dword __cdecl main_menu_championship(void *, Dword, Dword, Dword) {
    network::packets::pending::set_game_rule(static_cast<Dword *>(system(72)), 1, 4);
    return 0;
}

static Dword __cdecl main_menu_multiplayer(void *, Dword, Dword, Dword) {
    network::packets::pending::set_game_rule(static_cast<Dword *>(system(72)), 1, 5);
    return 0;
}

static Dword __cdecl main_menu_title_text(void *source, Dword, Dword, Dword) {
    auto *widget = static_cast<Widget *>(source);
    Widget *button = widget && invoke<Dword>(widget, 2, family::button_class_key) ? widget : nullptr;
    application::pending::LocalFrontendString text(
        &lifecycle::construction::unresolved::shared_string);
    if (button)
        gui::string_copy(&text.value, &button->tooltip);
    Ticker *ticker;
    {
        application::pending::LocalFrontendString name("<TITLEDATA>");
        auto *sibling = family::named_sibling(widget, &name.value);
        ticker = sibling && invoke<Dword>(sibling, 2, ticker_class_key)
                     ? reinterpret_cast<Ticker *>(sibling)
                     : nullptr;
    }
    if (ticker) {
        invoke(ticker, 7, &text.value);
        clear_ticker_messages(ticker);
        for (unsigned index = 0; index != 9; ++index) {
            const auto *message = invoke<const gui::String *>(ticker, 8);
            queue_ticker_message(ticker, message, 1, &ticker->base.base.colour, 0, 100.0f, 0);
        }
    }
    return 0;
}
 
static Dword __cdecl main_menu_set_focus(void *, Dword, Dword, Dword) {
    return 0;
}
static CallbackRecord main_menu_callbacks[9];

static void unregister_main_menu_callbacks() {
    for (unsigned index = 9; index; --index)
        unregister_callback(main_menu_callbacks + index - 1);
}

void register_main_menu_callbacks() {
    const struct Definition {
        Dword event;
        const char *event_name, *callback_name;
        Callback callback;
    } definitions[] = {
        {43, "BEVENT_PRESSED", "MainMenu_QuickRace", main_menu_quick_race},
        {6, "GEVENT_KEYDOWN", "MainMenu_QuickRace", main_menu_quick_race},
        {43, "BEVENT_PRESSED", "MainMenu_SingleChallenge", main_menu_single_challenge},
        {43, "BEVENT_PRESSED", "MainMenu_HotPursuit", main_menu_hot_pursuit},
        {43, "BEVENT_PRESSED", "MainMenu_TopCop", main_menu_top_cop},
        {43, "BEVENT_PRESSED", "MainMenu_Championship", main_menu_championship},
        {43, "BEVENT_PRESSED", "MainMenu_Multiplayer", main_menu_multiplayer},
        {14, "GEVENT_FOCUSSET", "MainMenu_SetTitleText", main_menu_title_text},
        {19, "GEVENT_INITIALUPDATE", "MainMenu_SetFocus", main_menu_set_focus}};
    for (unsigned index = 0; index != 9; ++index) {
        const auto &definition = definitions[index];
        register_callback(main_menu_callbacks + index, family::button_class_key,
                          definition.event_name, definition.event, definition.callback_name,
                          definition.callback, 0);
    }
    std::atexit(unregister_main_menu_callbacks);
}
} // namespace nfs::application::frontend
