#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "hud_display.hpp"
#include "../../graphics/cameras/camera_director_runtime.hpp"
#include "../lifecycle/gui_images.hpp"
#include "../../race/session/race_runtime.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include <cstdlib>

namespace nfs::application::frontend::hud {
using game::runtime::at;
using game::runtime::invoke;
using game::runtime::system;
using kernel::numeric::Extended;
using pending::LocalFrontendString;


static Byte wrong_way_camera_visible(void *camera) {
    const Byte hidden = at<Byte>(camera, 172);
    const Dword mode = at<Dword>(camera, 312 + at<Dword>(camera, 356) * 4);
    return !hidden && (mode == 1 || mode == 0 || mode == 2 || mode == 3);
}


static Byte entity_driving_wrong_way(void *entity) {
    if (at<Byte>(entity, 409))
        return 0;
    return game::world::camera::route_heading_alignment(entity) <
           Extended(0.f, kernel::numeric::current_format());
}


static Dword __cdecl wrong_way_event(void *object, Dword event, Dword, Dword) {
    auto *widget = static_cast<Widget *>(object);
    const bool first_player = gui::string_data(&widget->name) &&
                              gui::string_compare_ascii(&widget->name, "<WW1>") == 0;
    const int player = first_player ? 0 : 1;
    const Dword now = game::race_ticks;
    if (event == 18) {
        pending::set_widget_callback(widget, 21, wrong_way_event);
        pending::set_widget_visible(widget, 0);
        if (game::runtime::race_started) {
            void *car = game::runtime::physics::local_car(static_cast<Dword *>(system(68)), player);
            if (car) {
                void *entity = at<void *>(car, 7988);
                if (!at<Byte>(entity, 409) &&
                    game::world::camera::route_heading_alignment(entity) <
                        Extended(0.f, kernel::numeric::current_format()))
                    widget->value_84 = now - 180;
            }
        }
        return 0;
    }
    if (event != 21)
        return 0;

    if (!player || game::runtime::physics::split_screen()) {
        void *car = game::runtime::physics::local_car(static_cast<Dword *>(system(68)), player);
        if (car &&
            !pending::participant_blocked(at<void *>(system(92), 4), at<Dword>(car, 1772)) &&
            at<Dword>(system(72), 152) != 6 &&
            (!game::tick_services[player] || wrong_way_camera_visible(game::tick_services[player]))) {
            car = game::runtime::race_started
                      ? game::runtime::physics::local_car(static_cast<Dword *>(system(68)), player)
                      : nullptr;
            if (car && entity_driving_wrong_way(at<void *>(car, 7988))) {
                if (!widget->value_84)
                    widget->value_84 = now + 180;
            } else
                widget->value_84 = 0;

            if (int(widget->value_84) <= int(now) && widget->value_84) {
                const auto format = kernel::numeric::current_format();
                auto fade = Extended::integer(int(now - widget->value_84), format) *
                            0.00277777784503996372222900390625f;
                if (!(fade < Extended(.75f, format)))
                    fade = Extended(.75f, format);
                const Byte alpha = Byte(((Extended(1.f, format) - fade) * 255.f).truncate_to_integer());
                widget->colour = (Dword(alpha) << 24) | 16777215;
                pending::set_widget_visible(widget, 1);
                return 0;
            }
        }
    }
    pending::set_widget_visible(widget, 0);
    return 0;
}

static void assign_message(Widget *widget, Dword reference) {
    LocalFrontendString text;
    gui::string_reference(&text.value, reference);
    invoke(widget, 7, &text.value);
}

static void start_message_if_empty(Widget *widget) {
    if (!gui::engine::pending::character_at(invoke<const gui::String *>(widget, 8), 0))
        invoke(widget, 48);
}

static void display_race_message(Widget *widget, Dword title) {
    if (gui::engine::images::find_separator(&widget->name, "TEXT") < 0) {
        start_message_if_empty(widget);
        assign_message(widget, title);
    } else
        assign_message(widget, 1363);
    pending::set_widget_visible(widget, 1);
}

static void display_busted_message(Widget *widget) {
    if (gui::engine::images::find_separator(&widget->name, "TEXT") < 0) {
        start_message_if_empty(widget);
        assign_message(widget, 1249);
    }
    pending::set_widget_visible(widget, 1);
}

static Dword player_slot(const Widget *widget) {
    return at<Dword>(system(72), 204 + widget->value_84 * 4);
}


static Dword __cdecl race_message_event(void *object, Dword event, Dword, Dword) {
    auto *widget = static_cast<Widget *>(object);
    if (!widget || !invoke<Dword>(widget, 2, flash_text_class_key))
        return 0;
    if (event == 18) {
        pending::set_widget_visible(widget, 0);
        invoke(widget, 7, &lifecycle::construction::unresolved::shared_string);
        pending::set_widget_callback(widget, 21, race_message_event);
        if (gui::string_data(&widget->name) &&
            gui::string_compare_ascii(&widget->name, "<TEXT_1>") == 0)
            widget->value_84 = 0;
        else if (gui::string_data(&widget->name) &&
                 gui::string_compare_ascii(&widget->name, "<TITLE_1>") == 0)
            widget->value_84 = 0;
        else
            widget->value_84 = 1;
        return 0;
    }
    if (event != 21)
        return 0;
    pending::set_widget_visible(widget, 0);
    if (at<Byte>(system(48), 104))
        return 0;

    if (at<Dword>(system(72), 152) == 1) {
        const Dword slot = player_slot(widget);
        if (slot != Dword(-1) &&
            Byte(network::race_pending(reinterpret_cast<Dword *>(
                pending::participant(at<void *>(system(92), 4), slot)))))
            display_race_message(widget, 1263);
    }

    void *rules = system(72);
    if (at<Dword>(rules, 148) == 1) {
        const Dword mode = at<Dword>(rules, 152);
        if (mode == 0 || mode == 2) {
            if (game::runtime::physics::split_screen() &&
                pending::participant_blocked(at<void *>(system(92), 4), player_slot(widget)))
                display_race_message(widget, 1249);
        } else if (mode == 10) {
            const Dword slot = at<Dword>(rules, 204 + widget->value_84 * 4);
            if (slot != Dword(-1) && pending::participant_blocked(at<void *>(system(92), 4), slot))
                display_busted_message(widget);
        }
    }

    switch (at<Dword>(system(72), 152)) {
    case 0: case 1: case 2: case 3: case 4: case 5:
    case 7: case 8: case 9: case 11: case 12:
        if (game::runtime::physics::split_screen() &&
            pending::participant_blocked(at<void *>(system(92), 4), player_slot(widget)))
            display_race_message(widget, 1249);
        break;
    case 10:
        if (pending::participant_blocked(at<void *>(system(92), 4), player_slot(widget)))
            display_busted_message(widget);
        break;
    }
    return 0;
}


static Dword __cdecl busted_overlay_event(void *object, Dword event, Dword, Dword) {
    auto *widget = static_cast<Widget *>(object);
    if (widget && invoke<Dword>(widget, 2, flash_text_class_key)) {
        if (event == 18) {
            pending::set_widget_visible(widget, 0);
            pending::set_widget_callback(widget, 21, busted_overlay_event);
        } else if (event == 21) {
            if (!at<Byte>(system(48), 104))
                pending::set_widget_visible(widget, 0);
            else if (widget->flags & 2) {
                pending::set_widget_visible(widget, 1);
                invoke(widget, 48);
            }
        }
    }
    return 0;
}

static gui::widgets::CallbackRecord hud_callbacks[3];


static void unregister_hud_callbacks() {
    for (int index = 2; index >= 0; --index)
        gui::widgets::unregister_callback(&hud_callbacks[index]);
}


static void register_hud_callbacks() {
    gui::widgets::register_callback(&hud_callbacks[0], gui::widgets::base_class_name,
                                   "GEVENT_BEGIN", 18, "Hud_WrongWay", wrong_way_event, 0);
    gui::widgets::register_callback(&hud_callbacks[1], gui::widgets::base_class_name,
                                   "GEVENT_BEGIN", 18, "HudView_Message", race_message_event, 0);
    gui::widgets::register_callback(&hud_callbacks[2], flash_text_class_key,
                                   "GEVENT_BEGIN", 18, "Hud_BustedOverlay", busted_overlay_event, 0);
    std::atexit(unregister_hud_callbacks);
}

namespace {
NFS_KERNEL_INITIALIZER(startup_register_hud_callbacks, ".CRT$XCU_NFS_006900B4",
                       register_hud_callbacks);
} // namespace
} // namespace nfs::application::frontend::hud
