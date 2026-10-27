#include "frontend_menu_roots.hpp"
#include "nfs/resources/archives/archive_cache.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include "nfs/core/lifecycle/system_runtime.hpp"
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::at;
using game::runtime::bytes;
using game::runtime::invoke;
using game::runtime::system;

TreeScreen *construct_tree_screen(TreeScreen *screen, void *parent) {
    gui::widgets::pending::construct_interface(screen, parent);
    screen->mode = 0;
    screen->subtitle = nullptr;
    screen->tree = nullptr;
    screen->description = nullptr;
    screen->preferences = nullptr;
    screen->selection = 0;
    screen->purchasing = 0;
    screen->races = nullptr;
    screen->delay = 0;
    screen->base.methods = tree_screen_methods.data();
    try {
        pending::LocalFrontendString types("<RACE_TYPE>");
        read_tree_text_list(types.release(), screen->race_names, 14);
        pending::LocalFrontendString skills("<OPPONENT_SKILL>");
        read_tree_text_list(skills.release(), screen->skill_names, 3);
        auto *races = static_cast<TreeRace *>(
            files::archives::allocate_entry(20064, "tree node info", lifecycle::level_pool));
        if (races)
            for (unsigned i = 0; i < 33; ++i)
                initialize_tree_race(races + i);
        screen->races = races;
        network::packets::pending::set_game_rule(system(72), 13, 1);
    } catch (...) {
        destroy_interface(&screen->base);
        throw;
    }
    return screen;
}

void destroy_tree_screen(TreeScreen *screen) {
    screen->base.methods = tree_screen_methods.data();
    if (screen->races)
        kernel::memory_free(screen->races);
    if (void *camera = invoke<void *>(game::tick_services[0], 2)) {
        at<float>(camera, 56) = 10000;
        at<float>(camera, 52) = 1;
    }
    destroy_interface(&screen->base);
}

void __fastcall initialize_tree_screen(TreeScreen *screen, void *) {
    gui::widgets::pending::request_interface_layout(&screen->base, nullptr);
    void *rules = system(72);
    at<int>(rules, 102920) = screen->selection;
    screen->preferences = bytes(rules) + 2488;
    screen->tree =
        reinterpret_cast<TreeWidget *>(track_child(screen, "<TREE>", tree_widget_class_key));
    if (screen->tree) {
        screen->tree->selection = screen->selection;
        screen->tree->rotation = 0;
    }
    screen->description = track_child(screen, "<TREE_DATA>", family::text_class_key);
    if (void *camera = invoke<void *>(game::tick_services[0], 2)) {
        at<float>(camera, 56) = 38;
        at<float>(camera, 52) = 1;
    }
}

Dword __fastcall tree_screen_event(TreeScreen *screen, void *, Dword event, Dword first, Dword second) {
    return gui::widgets::pending::handle_event(&screen->base, nullptr, event, first, second);
}

Widget *tree_race_button(TreeScreen *screen, int index) {
    pending::LocalFrontendString name(&lifecycle::construction::unresolved::shared_string);
    gui::string_assign(&name.value, index < 10 ? "bt_0%d" : "bt_%d");
    gui::string_format(&name.value, &name.value, index);
    return track_child(screen, gui::string_ascii(&name.value), family::button_class_key);
}

void enable_tree_selection(TreeScreen *screen, std::uint8_t enabled) {
    if (auto *button = tree_race_button(screen, screen->selection)) {
        set_enabled(button, enabled);
        pending::set_widget_visible(button, enabled);
    }
}

void refresh_tree_selection(TreeScreen *screen) {
    enable_tree_selection(screen, 1);
    const int selection = screen->selection;
    at<int>(system(72), 102920) = selection;
    TreeRace &event = screen->races[selection];
    screen->mode = int(event.type);
    if (screen->description) {
        pending::LocalFrontendString text;
        gui::string_reference(&text.value, event.title == Dword(-1) ? screen->race_names[event.type]
                                                                   : event.title);
        invoke<void>(screen->description, 7, &text.value);
    }
    screen->tree->selection = selection;
    screen->tree->rotation = 0;
    at<int>(screen->preferences, at<Dword>(system(72), 148) == 2 ? 7540 : 7536) = selection;
    if (auto *button = tree_race_button(screen, selection))
        if (active_widget_id != button->id)
            activate(button);
}

void __fastcall tree_screen_child(TreeScreen *screen, void *, Widget *sender, Dword event, Dword,
                                  Dword modifiers) {
    if (std::uint8_t(modifiers))
        return;
    int selected = 0;
    for (; selected < 33; ++selected) {
        auto *button = tree_race_button(screen, selected);
        if (button && button == sender)
            break;
    }
    if (selected < 33) {
        if (event == 14) {
            screen->selection = selected;
            refresh_tree_selection(screen);
            screen->delay = 0;
        } else if (event == 43)
            open_tree_event(screen);
    }
    enable_tree_selection(screen, 1);
}

Dword navigate_tree_screen(TreeScreen *screen, Dword direction) {
    auto &scene = screen->tree->base.base;
    if (scene.drag_flags[0] || screen->delay || !scene.drag_flags[1])
        return 0;
    const Dword target = screen->races[screen->selection].navigation[direction];
    if (int(target) == screen->selection)
        return 0;
    screen->selection = int(target);
    refresh_tree_selection(screen);
    screen->delay = 0;
    return 1;
}

Dword __fastcall tree_screen_key(TreeScreen *screen, void *, Dword key, Dword modifiers) {
    Widget *button = tree_race_button(screen, screen->selection);
    auto finish = [&]() {
        return gui::widgets::pending::interface_key(&screen->base, nullptr, key, modifiers);
    };
    if (key == 28) {
        if (!std::uint8_t(modifiers))
            open_tree_event(screen);
        return finish();
    }
    if (key != 200 && key != 203 && key != 205 && key != 208)
        return finish();
    if (button && active_widget_id != button->id) {
        activate(button);
        return finish();
    }
    auto &scene = screen->tree->base.base;
    bool navigation = false;
    Dword target = Dword(screen->selection);
    if (!scene.drag_flags[0] && !screen->delay && scene.drag_flags[1]) {
        const unsigned direction = key == 200 ? 0 : key == 203 ? 3 : key == 205 ? 1 : 2;
        target = screen->races[screen->selection].navigation[direction];
        navigation = int(target) != screen->selection;
    }
    if (navigation) {
        screen->selection = int(target);
        refresh_tree_selection(screen);
        screen->delay = 0;
        game::audio::play_frontend_event(32, 127);
    } else if (key == 200)
        activate_last_child(&screen->base);
    else if (key == 208)
        activate_first_child(&screen->base);
    return finish();
}

Dword __fastcall tree_screen_key_up(TreeScreen *screen, void *, Dword key, Dword modifiers) {
    return style_result_34(&screen->base, nullptr, key, modifiers);
}

void __fastcall tick_tree_screen(TreeScreen *screen, void *, Dword elapsed) {
    if (screen->delay > 0) {
        screen->delay = int(Dword(screen->delay) - elapsed);
        if (screen->delay < 0)
            screen->delay = 0;
    }
}
} // namespace nfs::application::frontend
