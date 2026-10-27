#include "../screens/frontend_menu_roots.hpp"
namespace audio_layout = nfs::game::audio::state::layout;
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::at;
using game::runtime::invoke;
using game::runtime::system;

void car_purchase_error() {
    game::audio::play_frontend_event(66, 63);
    pending::LocalFrontendString text;
    gui::string_reference(&text.value, 1424);
    pending::ScopedMessageBox message(&text.value, 8449);
    pending::run_message_box(&message.value, nullptr, 0);
}

void enable_selected_car_image(CarSelection *screen, Dword car) {
    if (!screen->car_list)
        return;
    auto *widget = reinterpret_cast<Widget *>(screen->car_list);
    for (int i = 0; i < int(screen->car_list->base.items.count); ++i) {
        const Dword id = family::image_cell_id(widget, i);
        if (id && menu_image_value(id) == car) {
            set_menu_image_enabled(id, 1);
            return;
        }
    }
}

Dword purchase_selected_car(CarSelection *screen) {
    pending::LocalFrontendString format, text;
    gui::string_reference(&format.value, 1426);
    const Dword cost = car_cost(screen);
    gui::string_format(&text.value, &format.value, at<Dword>(screen->preferences, 7240), cost);
    pending::ScopedMessageBox message(&text.value, 18);
    if (pending::run_message_box(&message.value, nullptr, 0) != 2)
        return 0;
    at<Dword>(screen->preferences, 7240) -= cost;
    if (Dword(screen->selection) < 86)
        at<std::uint8_t>(screen->preferences, 7328 + Dword(screen->selection)) = 1;
    ++at<Dword>(screen->preferences, 7324);
    if (screen->preview) {
        set_enabled(reinterpret_cast<Widget *>(screen->preview), 1);
        screen->preview->base.rotates = 1;
    }
    refresh_car_selection(screen);
    enable_selected_car_image(screen, Dword(screen->selection));
    for (auto *widget : {screen->lock, screen->cost})
        if (widget) {
            set_enabled(widget, 0);
            pending::set_widget_visible(widget, 0);
        }
    if (screen->preview_hint) {
        pending::LocalFrontendString next;
        gui::string_reference(&next.value, 736);
        invoke<void>(screen->preview_hint, 7, &next.value);
    }
    return 1;
}

Dword can_select_car(CarSelection *screen, std::uint8_t prompt) {
    if (car_unlocked(screen))
        return 1;
    if (at<int>(screen->preferences, 7240) < int(car_cost(screen))) {
        if (prompt)
            car_purchase_error();
    } else if (prompt)
        purchase_selected_car(screen);
    return 0;
}

Dword __cdecl car_next_pressed(void *sender, Dword event, Dword, Dword) {
    if (event == 43)
        if (auto *root = pending::widget_root(static_cast<Widget *>(sender)))
            if (invoke<Dword>(root, 2, interface_class_name))
                family::finish_modal(root, 4, 0);
    return 0;
}

Dword __cdecl toggle_car_description(void *sender, Dword event, Dword, Dword modifiers) {
    if (event == 43 && !std::uint8_t(modifiers))
        if (auto *root = pending::widget_root(static_cast<Widget *>(sender)))
            if (invoke<Dword>(root, 2, car_selection_class_key)) {
                auto *screen = reinterpret_cast<CarSelection *>(root);
                if (car_unlocked(screen)) {
                    flush_car_selection(screen);
                    if (game::audio::state::option<Dword>(audio_layout::speech_stream_active)) {
                        stop_track_description(system(84));
                        show_car_statistics(screen, 0);
                    } else {
                        play_car_description(system(84), Dword(screen->selection));
                        show_car_statistics(screen, 1);
                    }
                }
            }
    return 0;
}
} // namespace nfs::application::frontend
