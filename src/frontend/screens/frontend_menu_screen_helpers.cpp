#include "frontend_menu_controls.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::invoke;

static Widget *search_menu_ancestors(Widget *widget, const gui::String *type,
                                     const gui::String *name, Dword flags) {
    for (; widget; widget = widget->parent) {
        if (name->length && !gui::string_equal(&widget->name, name))
            continue;
        if (!type->length)
            return widget;
        if (flags & 1) {
            if (toggle_string_equals(type, invoke<const char *>(widget, 1)))
                return widget;
        } else if (invoke<Dword>(widget, 3, gui::string_ascii(type)))
            return widget;
    }
    return nullptr;
}

Widget *find_menu_ancestor(Widget *widget, const gui::String *type, const gui::String *name,
                           Dword flags) {
    if (!gui::string_data(type) && !gui::string_data(name))
        return nullptr;
    return search_menu_ancestors(widget->parent, type, name, flags);
}

const char *__cdecl options_screen_class_key() {
    return "OptionsScreen";
}

const char *__cdecl challenge_screen_class_key() {
    return "SingleChallengeScreen";
}
static Ticker *heading_ticker(Widget *screen) {
    auto *widget = static_cast<Widget *>(pending::find_child(screen, "<FOCUS_STATE>"));
    return widget && invoke<Dword>(widget, 2, ticker_class_key) ? reinterpret_cast<Ticker *>(widget)
                                                               : nullptr;
}
static void repeat_heading(Ticker *ticker, unsigned count) {
    clear_ticker_messages(ticker);
    for (unsigned index = 0; index < count; ++index)
        queue_ticker_message(ticker, invoke<const gui::String *>(ticker, 8), 1,
                             &ticker->base.base.colour, 0, 100.0f, 0);
}

static void repeat_options_heading(Widget *screen) {
    if (auto *ticker = heading_ticker(screen))
        repeat_heading(ticker, 4);
}

void set_options_heading(Widget *screen, Dword reference) {
    if (auto *ticker = heading_ticker(screen)) {
        {
            pending::LocalFrontendString title;
            gui::string_reference(&title.value, reference);
            invoke(ticker, 7, &title.value);
        }
        repeat_options_heading(screen);
    }
}

void set_challenge_heading(Widget *screen, Dword reference) {
    if (auto *ticker = heading_ticker(screen)) {
        pending::LocalFrontendString title;
        gui::string_reference(&title.value, reference);
        invoke(ticker, 7, &title.value);
    }
}

Dword __cdecl update_options_heading(void *sender, Dword, Dword, Dword) {
    Widget *screen;
    {
        pending::LocalFrontendString type("OptionsScreen");
        screen =
            find_menu_ancestor(static_cast<Widget *>(sender), &type.value,
                               &lifecycle::construction::unresolved::shared_string, 0);
        if (screen && !invoke<Dword>(screen, 2, options_screen_class_key))
            screen = nullptr;
    }
    if (screen)
        repeat_options_heading(screen);
    return 0;
}

Dword __cdecl update_challenge_heading(void *sender, Dword, Dword, Dword) {
    Widget *screen;
    {
        pending::LocalFrontendString type("SingleChallengeScreen");
        screen =
            find_menu_ancestor(static_cast<Widget *>(sender), &type.value,
                               &lifecycle::construction::unresolved::shared_string, 0);
        if (screen && !invoke<Dword>(screen, 2, challenge_screen_class_key))
            screen = nullptr;
    }
    if (screen) {
        pending::LocalFrontendString unused(
            &lifecycle::construction::unresolved::shared_string);
        if (auto *ticker = heading_ticker(screen))
            repeat_heading(ticker, 5);
    }
    return 0;
}
} // namespace nfs::application::frontend
