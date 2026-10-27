#include "../screens/frontend_menu_roots.hpp"
#include "nfs/physics/frame/simulation_runtime.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
namespace nfs::gui::engine {
int seconds_to_ticks(float);
}
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::at;
using game::runtime::bytes;
using game::runtime::invoke;
using game::runtime::system;
static void hide_car_element(Widget *widget) {
    if (widget) {
        set_enabled(widget, 0);
        pending::set_widget_visible(widget, 0);
    }
}
static void append_transmission(ProfileToggle *toggle, Dword reference, Dword value) {
    pending::LocalFrontendString text;
    gui::string_reference(&text.value, reference);
    family::ListItem item;
    construct_toggle_item(&item, &text.value, value);
    try {
        invoke<void>(&toggle->items, 8, &item);
    } catch (...) {
        gui::string_destruct(&item.tooltip);
        gui::string_destruct(&item.text);
        throw;
    }
    gui::string_destruct(&item.tooltip);
    gui::string_destruct(&item.text);
}

void __fastcall initialize_car_selection(CarSelection *screen, void *) {
    reinterpret_cast<void(__thiscall *)(Widget *)>(interface_methods[21])(&screen->base);
    screen->vehicles = system(52);
    screen->showcase = static_cast<Widget *>(pending::find_child(screen, "<Showcase>"));
    hide_car_element(screen->showcase);
    screen->colours =
        reinterpret_cast<ColourWheel *>(track_child(screen, "<CAR_COLOR>", colour_wheel_class_key));
    screen->logo = track_child(screen, "<logotype>", family::anim_image_class_key);
    screen->preview_hint = static_cast<Widget *>(pending::find_child(screen, "<t1>"));
    screen->lock = static_cast<Widget *>(pending::find_child(screen, "<lock>"));
    screen->cost = static_cast<Widget *>(pending::find_child(screen, "<Cost>"));
    screen->balance = static_cast<Widget *>(pending::find_child(screen, "<BNK2>"));
    screen->basic_colour = static_cast<Widget *>(pending::find_child(screen, "<DEBUG_BASICCOLOR>"));
    screen->car_list = reinterpret_cast<MenuImageList *>(
        track_child(screen, "<CAR_IL>", menu_image_list_class_key));
    screen->preview =
        reinterpret_cast<CarPreview *>(track_child(screen, "<CAR>", car_preview_class_key));
    screen->statistics =
        reinterpret_cast<Ticker *>(track_child(screen, "<STATS>", ticker_class_key));
    hide_car_element(reinterpret_cast<Widget *>(screen->statistics));
    screen->rotation_hint = static_cast<Widget *>(pending::find_child(screen, "<TRot>"));
    hide_car_element(screen->rotation_hint);
    hide_car_element(screen->basic_colour);
    if (std::uint8_t(services::special_race_mode(system(72)))) {
        hide_car_element(screen->lock);
        screen->lock = nullptr;
        hide_car_element(screen->cost);
        screen->cost = nullptr;
    }
    family::move_to_first_draw_position(reinterpret_cast<Widget *>(screen->colours));
    clear_colour_wheel(screen->colours);
    if (game::runtime::physics::split_screen())
        if (auto *title = track_child(screen, "t0", family::text_class_key)) {
            pending::LocalFrontendString text;
            gui::string_reference(&text.value, screen->player == 1 ? 454 : 453);
            invoke<void>(title, 7, &text.value);
        }
    const int count = at<int>(screen->vehicles, 4);
    for (int i = 0; i < count; ++i) {
        if (catalogue_entry_traffic(bytes(screen->vehicles) + 4, i)) {
            gui::string_copy(&screen->descriptions[i],
                             &lifecycle::construction::unresolved::shared_string);
            break;
        }
        gui::string_assign(&screen->descriptions[i],
                           catalogue_entry_resource(bytes(screen->vehicles) + 4, i));
    }
    void *rules = system(72);
    screen->preferences = bytes(rules) + 2488;
    const Dword slot = at<Dword>(rules, 204 + screen->player * 4);
    void *entry = slot < 24 ? bytes(rules) + 248 + slot * 84 : nullptr;
    at<Dword>(entry, 0) = 1;
    const Dword mode = at<Dword>(rules, 152);
    screen->police = mode == 4 || mode == 9 || mode == 10 || mode == 11 || mode == 13 ||
                     (mode == 6 && !screen->player);
    at<Dword>(entry, 8) = screen->police;
    populate_car_selection(screen);
    screen->skin = at<short>(car_preferences(screen), 7414 + Dword(screen->car) * 2);
    screen->colours->selected = screen->skin;
    refresh_colour_wheel(screen->colours);
    commit_selected_car(screen);
    hide_car_element(reinterpret_cast<Widget *>(screen->preview));
    hide_car_element(screen->preview_hint);
    hide_car_element(screen->lock);
    hide_car_element(screen->cost);
    if (pending::find_child(screen, "<btna>"))
        hide_car_element(static_cast<Widget *>(pending::find_child(screen, "<btna>")));
    screen->preview_delay =
        int(services::pending::gui_repeat_delay + gui::engine::seconds_to_ticks(.1f));
    screen->transmission = reinterpret_cast<ProfileToggle *>(
        track_child(screen, "<TRANY>", family::toggle_button_class_key));
    if (screen->transmission) {
        invoke<void>(&screen->transmission->items, 1);
        append_transmission(screen->transmission, 690, 0);
        append_transmission(screen->transmission, 1, 1);
        if (at<Dword>(car_preferences(screen), 52) == 1)
            invoke<void>(&screen->transmission->items, 10, Dword(1), Dword(1));
        else
            invoke<void>(&screen->transmission->items, 10, Dword(0), Dword(1));
    }
}
} // namespace nfs::application::frontend
