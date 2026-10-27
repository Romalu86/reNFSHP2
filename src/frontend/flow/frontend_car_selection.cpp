#include "../screens/frontend_menu_roots.hpp"
#include "nfs/physics/frame/simulation_runtime.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include <cstring>
namespace nfs::gui::engine {
int seconds_to_ticks(float);
}
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::at;
using game::runtime::bytes;
using game::runtime::invoke;
using game::runtime::system;
using kernel::numeric::Extended;

CarSelection *construct_car_selection(CarSelection *screen, void *parent) {
    gui::widgets::pending::construct_interface(screen, parent);
    screen->car = screen->selection = 0;
    screen->previous_car = screen->preview_car = screen->preview_skin = -1;
    screen->preview_delay = 0;
    screen->delay_enabled = 0;
    unsigned initialized = 0;
    try {
        for (; initialized < 38; ++initialized)
            gui::string_construct(&screen->descriptions[initialized]);
    } catch (...) {
        while (initialized)
            gui::string_destruct(&screen->descriptions[--initialized]);
        destroy_interface(&screen->base);
        throw;
    }
    screen->skin = 0;
    screen->police = 0;
    screen->preview_dirty = 1;
    screen->player = 0;
    screen->preferences = nullptr;
    screen->preview_hint = screen->balance = screen->cost = screen->lock = screen->rotation_hint =
        screen->showcase = nullptr;
    screen->transmission = nullptr;
    screen->basic_colour = screen->logo = nullptr;
    screen->statistics = nullptr;
    screen->base.methods = car_selection_methods.data();
    return screen;
}

void destroy_car_selection(CarSelection *screen) {
    screen->base.methods = car_selection_methods.data();
    if (screen->preview_car >= 0 && screen->preview)
        reset_scene_camera(&screen->preview->base);
    for (int i = 37; i >= 0; --i)
        gui::string_destruct(&screen->descriptions[i]);
    destroy_interface(&screen->base);
}

Dword __fastcall load_car_selection(CarSelection *screen, void *, void *configuration) {
    gui::engine::pending::read_int(configuration, "CarSelectScreen.mPlayerIndex",
                                   reinterpret_cast<int *>(&screen->player), 0);
    return reinterpret_cast<Dword(__thiscall *)(Widget *, void *)>(interface_methods[4])(
        &screen->base, configuration);
}

Dword __fastcall save_car_selection(CarSelection *screen, void *, void *configuration) {
    gui::engine::pending::write_int(configuration, "CarSelectScreen.mPlayerIndex",
                                    int(screen->player), 0);
    return reinterpret_cast<Dword(__thiscall *)(Widget *, void *)>(interface_methods[5])(
        &screen->base, configuration);
}
static void delay_car_preview(CarSelection *screen) {
    screen->preview_delay =
        int(services::pending::gui_repeat_delay + gui::engine::seconds_to_ticks(.1f));
}

void refresh_selected_skin(CarSelection *screen) {
    const int previous = screen->skin;
    screen->skin = screen->police                                       ? 15
                   : screen->selection >= 38 && screen->selection <= 61 ? 16
                                                                        : screen->colours->selected;
    if (previous != screen->skin)
        delay_car_preview(screen);
}

void __fastcall car_selection_child(CarSelection *screen, void *, Widget *child, Dword event, Dword,
                                    Dword) {
    if (child == reinterpret_cast<Widget *>(screen->transmission) && event == 68) {
        auto *item = invoke<family::ListItem *>(
            &screen->transmission->items, 9,
            Dword(family::selected_list_row(&screen->transmission->items)), Dword(0));
        if (item)
            at<Dword>(car_preferences(screen), 52) = item->data;
    }
    if (child == reinterpret_cast<Widget *>(screen->colours)) {
        if (event == 40 && !(child->flags & 2))
            refresh_selected_skin(screen);
    } else if (child == reinterpret_cast<Widget *>(screen->car_list)) {
        if (event == 68) {
            commit_selected_car(screen);
            screen->preview_dirty = 0;
            return;
        }
        if (event == 56)
            screen->preview_dirty = 1;
    }
}

void __fastcall resume_car_selection(CarSelection *screen, void *) {
    screen->delay_enabled = 1;
    delay_car_preview(screen);
}

void __fastcall update_car_selection(CarSelection *screen, void *, Dword elapsed) {
    if (screen->delay_enabled && screen->preview_delay > 0) {
        screen->preview_delay = int(Dword(screen->preview_delay) - elapsed);
        if (screen->preview_delay < 1) {
            screen->preview_delay = 0;
            if (screen->preview_dirty)
                update_car_preview(screen);
            else
                screen->preview_delay = 1;
        }
    }
}

void commit_selected_car(CarSelection *screen) {
    const int previous = screen->selection;
    screen->selection =
        int(menu_image_value(selected_menu_image(reinterpret_cast<Widget *>(screen->car_list))));
    const Dword mode = network::packets::rule_value(system(72), 1);
    if (mode == 4)
        at<int>(screen->preferences, 7524) = screen->selection;
    else if (network::packets::rule_value(system(72), 1) == 2)
        at<int>(screen->preferences, 7528) = screen->selection;
    else
        at<int>(car_preferences(screen), screen->police ? 7520 : 7516) = screen->selection;
    screen->car = screen->selection >= 86   ? 0
                  : screen->selection >= 62 ? screen->selection - 62
                  : screen->selection >= 38 ? screen->selection - 38
                                            : screen->selection;
    refresh_selected_skin(screen);
    if (previous != screen->selection)
        delay_car_preview(screen);
    refresh_car_selection(screen);
}
float clamp_car_zoom(float, float, float);

Dword __fastcall car_selection_key(CarSelection *screen, void *, Dword key, Dword modifiers) {
    if (key == 12 || key == 74 || key == 13 || key == 78) {
        auto *preview = screen->preview;
        if (preview && !(preview->base.base.base.flags & 1) && preview->actor) {
            if (screen->preview_delay)
                delay_car_preview(screen);
            const auto format = kernel::numeric::current_format();
            float &z = preview->base.camera_origin[2];
            z = float(Extended(z, format) + (key == 12 || key == 74 ? 22.f : -22.f));
            z = clamp_car_zoom(z, 1, 23);
            preview->base.drag_flags[0] = 1;
            preview->base.drag_flags[1] = 0;
        }
    } else if (key == 203 || key == 205) {
        if (screen->preview_delay)
            delay_car_preview(screen);
        if (screen->car_list) {
            gui::events::dispatch(screen->car_list, key == 203 ? 107 : 106, 0, 0);
            return 1;
        }
    }
    return reinterpret_cast<Dword(__thiscall *)(Widget *, Dword, Dword)>(interface_methods[32])(
        &screen->base, key, modifiers);
}

Dword __fastcall car_selection_key_up(CarSelection *screen, void *, Dword key, Dword modifiers) {
    if (key == 12 || key == 13 || key == 74 || key == 78) {
        auto *preview = screen->preview;
        if (preview && !(preview->base.base.base.flags & 1) && preview->actor) {
            std::memcpy(preview->base.camera_origin, &preview->base.initial_camera.x, 12);
            preview->base.requested_zoom = preview->base.zoom;
            preview->base.drag_flags[0] = 0;
        }
    }
    return style_result_34(&screen->base, nullptr, key, modifiers);
}

Dword __fastcall finish_car_selection(CarSelection *screen, void *, Dword result, Dword) {
    if (result == 4) {
        // Each cleanup call consumes one stack argument.
        if (!can_select_car(screen, 1))
            return 0;
        if (screen->player < 2) {
            void *rules = system(72);
            const Dword slot = at<Dword>(rules, 204 + screen->player * 4);
            at<int>(rules, 308 + slot * 84) = screen->car;
            at<int>(rules, 312 + slot * 84) = screen->skin;
            at<Dword>(rules, (slot + 3) * 84) = 0;
        }
        if (!game::runtime::physics::split_screen() || screen->player)
            pending::clear_screen_queue(system(96));
        else {
            pending::LocalFrontendString name("CarSelectP2"), type("CarSelectScreen");
            pending::enqueue_screen(system(96), name.release(), type.release());
        }
    }
    stop_track_description(system(84));
    screen->base.modal_result = result;
    return 1;
}
} // namespace nfs::application::frontend
