#include "../screens/frontend_menu_roots.hpp"
#include "nfs/world/resources/runtime_resources.hpp"
#include "../../vehicles/rendering/vehicle_renderable_resources.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include <cstring>
namespace audio_layout = nfs::game::audio::state::layout;
namespace nfs::gui::widgets::family {
Dword select_named_sequence_frame(ImageSequence *, const String *);
}
namespace nfs::gui::engine {
int seconds_to_ticks(float);
}
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::at;
using game::runtime::bytes;
using game::runtime::invoke;
using game::runtime::system;
static void car_element(Widget *widget, Dword enabled) {
    if (widget) {
        set_enabled(widget, enabled);
        pending::set_widget_visible(widget, enabled);
    }
}
static void car_reference(Widget *widget, Dword reference) {
    pending::LocalFrontendString text;
    gui::string_reference(&text.value, reference);
    invoke<void>(widget, 7, &text.value);
}

void refresh_car_colours(CarSelection *screen) {
    if (!screen->colours)
        return;
    clear_colour_wheel(screen->colours);
    if (screen->police) {
        screen->skin = 15;
        return;
    }
    void *record = bytes(screen->vehicles) + Dword(screen->car) * 1228 + 8;
    for (int i = 0; i < at<int>(record, 60); ++i) {
        void *colour = game::world::vehicle_render::catalogue_skin(bytes(screen->vehicles) + 4,
                                                                   Dword(screen->car), Dword(i));
        pending::LocalFrontendString name;
        gui::string_reference(&name.value, at<Dword>(colour, 4));
        pending::LocalFrontendString owned(&name.value);
        append_colour_wheel(screen->colours, owned.release(), at<Dword>(colour, 8));
    }
    screen->colours->selected = at<short>(car_preferences(screen), 7414 + Dword(screen->car) * 2);
    refresh_colour_wheel(screen->colours);
    screen->skin = screen->colours->selected;
}

void update_car_preview(CarSelection *screen) {
    if (!screen->preview)
        return;
    if (screen->previous_car != screen->car) {
        screen->previous_car = screen->car;
        refresh_car_colours(screen);
        refresh_selected_skin(screen);
    }
    if (screen->preview_car != screen->selection || screen->preview_skin != screen->skin) {
        if (screen->preview_car >= 0)
            reset_scene_camera(&screen->preview->base);
        screen->preview_car = screen->selection;
        screen->preview_skin = screen->skin;
        if (screen->selection < 24)
            at<short>(car_preferences(screen), 7414 + Dword(screen->car) * 2) =
                short(screen->skin);
        clear_car_preview(screen->preview);
        pending::LocalFrontendString resource(&screen->descriptions[screen->car]);
        gui::string_copy(&screen->preview->resource, &resource.value);
        load_car_preview(screen->preview, Dword(screen->skin));
        const bool unlocked = car_unlocked(screen);
        car_element(screen->lock, !unlocked);
        if (screen->cost) {
            pending::LocalFrontendString format, text;
            gui::string_reference(&format.value, 1362);
            gui::string_format(&text.value, &format.value, car_cost(screen));
            invoke<void>(screen->cost, 7, &text.value);
            car_element(screen->cost, !unlocked);
        }
        if (screen->preview_hint) {
            car_reference(screen->preview_hint, unlocked ? 736 : 1131);
            car_element(screen->preview_hint, 0);
        }
        car_element(screen->showcase, unlocked);
    }
    auto *preview = reinterpret_cast<Widget *>(screen->preview);
    car_element(preview, 1);
    const Dword unlocked = car_unlocked(screen);
    set_enabled(preview, unlocked);
    screen->preview->base.rotates = std::uint8_t(unlocked);
    refresh_car_selection(screen);
}

void refresh_car_selection(CarSelection *screen) {
    const Dword unlocked = car_unlocked(screen);
    if (auto *next = static_cast<Widget *>(pending::find_child(screen, "Next"))) {
        car_reference(next, unlocked ? 567 : 1131);
        if (!unlocked)
            gui::string_copy(&next->tooltip,
                             &lifecycle::construction::unresolved::shared_string);
        else
            gui::string_reference(&next->tooltip, 1802);
    }
    if (screen->cost) {
        pending::LocalFrontendString format, text;
        gui::string_reference(&format.value, 1362);
        gui::string_format(&text.value, &format.value, car_cost(screen));
        invoke<void>(screen->cost, 7, &text.value);
        car_element(screen->cost, !unlocked);
    }
    car_element(screen->lock, !unlocked);
    car_element(reinterpret_cast<Widget *>(screen->transmission), unlocked);
    car_element(screen->preview_hint, 0);
    if (pending::find_child(screen, "<btna>"))
        car_element(static_cast<Widget *>(pending::find_child(screen, "<btna>")), 1);
    if (screen->preview_delay <= 0) {
        if (pending::find_child(screen, "<btny>"))
            car_element(static_cast<Widget *>(pending::find_child(screen, "<btny>")), unlocked);
        if (pending::find_child(screen, "<t3>"))
            car_element(static_cast<Widget *>(pending::find_child(screen, "<t3>")), unlocked);
        car_element(screen->showcase, unlocked);
    }
    car_element(screen->showcase, unlocked);
    auto *preview = screen->preview;
    if (preview->actor) {
        game::runtime::scale_vectors(1, &preview->base.angular_velocity.x, 0.00277777784503996372222900390625f,
                                     &preview->base.angular_step.x);
        assign_vector_value(&preview->base.initial_position, &preview->base.position.x);
        preview->base.drag_flags[0] = 0;
        std::memcpy(preview->base.camera_origin, &preview->base.initial_camera.x, 12);
        preview->base.requested_zoom = preview->base.zoom;
        preview->base.drag_flags[0] = 0;
    }
    car_element(reinterpret_cast<Widget *>(screen->colours), screen->colours->count > 1);
    if (screen->selection > 37)
        car_element(reinterpret_cast<Widget *>(screen->colours), 0);
    if (screen->preview_delay > 0)
        car_element(reinterpret_cast<Widget *>(screen->colours), 0);
    if (game::audio::state::option<Dword>(audio_layout::speech_stream_active))
        stop_track_description(system(84));
    gui::String frame;
    car_frame_name(&frame, Dword(screen->car));
    try {
        family::select_named_sequence_frame(
            reinterpret_cast<family::ImageSequence *>(bytes(screen->logo) + 328), &frame);
    } catch (...) {
        gui::string_destruct(&frame);
        throw;
    }
    gui::string_destruct(&frame);
    if (screen->balance) {
        pending::LocalFrontendString text, number;
        gui::string_reference(&text.value, 722);
        gui::string_append_reference(&text.value, 733);
        gui::string_append_reference(&text.value, 733);
        gui::string_format_ascii(&number.value, "%d", at<int>(screen->preferences, 7240));
        gui::string_append(&text.value, &number.value);
        invoke<void>(screen->balance, 7, &text.value);
    }
    if (auto *footer = track_child(screen, "<ft>", flash_text_class_key))
        car_reference(footer, game::runtime::physics::split_screen()
                                  ? screen->player ? 454 : 453
                                  : 739);
    show_car_statistics(screen, 0);
}

void flush_car_selection(CarSelection *screen) {
    if (!screen->preview_delay)
        return;
    if (screen->car_list && !screen->preview_dirty) {
        auto *widget = reinterpret_cast<Widget *>(screen->car_list);
        widget->scroll.x = widget->scroll.y = 0;
        (void)menu_image_index(widget, selected_menu_image(widget));
        align_menu_image_list(widget);
        screen->preview_dirty = 1;
    }
    screen->preview_delay = 0;
    update_car_preview(screen);
}
} // namespace nfs::application::frontend
