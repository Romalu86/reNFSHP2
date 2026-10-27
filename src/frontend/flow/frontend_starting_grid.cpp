#include "../../gui/widgets/gui_widget_shared_methods.hpp"
#include "../screens/frontend_race_dialogs.hpp"
#include "../../gui/widgets/gui_widget_list.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
namespace nfs::application::pending {
void reset_race_round(void *);
}
namespace nfs::profiles {
gui::widgets::family::ListItem *construct_profile_list_item(gui::widgets::family::ListItem *);
}
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::at;
using game::runtime::invoke;
using game::runtime::system;

static Byte is_police_model_variant(int car) { return car >= 38 && car < 62; }

static void append_participant_car_label(void *record, gui::String *text) {
    void *entry = pending::grid_entry(system(72), at<Dword>(record, 4));
    gui::string_copy(text, &lifecycle::construction::unresolved::shared_string);
    const int car = at<int>(entry, 60);
    if (is_police_model_variant(car) || at<Dword>(entry, 64) == 16) {
        gui::string_append_ascii(text, "NFS");
        gui::string_append_reference(text, 733);
    }
    gui::string_append_reference(text, car == 1 ? 1385 :
        profile_car_label(game::runtime::bytes(system(52)) + 4, Dword(car)));
}

void participant_car_label(void *race, Dword slot, gui::String *text) {
    append_participant_car_label(pending::participant(race, slot), text);
}

static gui::String *construct_participant_name(void *record, gui::String *text) {
    gui::string_construct(text);
    try { gui::string_copy(text, &static_cast<pending::RaceParticipant *>(record)->name); }
    catch (...) { gui::string_destruct(text); throw; }
    return text;
}

gui::String *participant_display_name(void *race, gui::String *text, Dword slot) {
    return construct_participant_name(pending::participant(race, slot), text);
}
struct StartingGridItem {
    family::ListItem value;
    StartingGridItem() { profiles::construct_profile_list_item(&value); }
    ~StartingGridItem() {
        gui::string_destruct(&value.tooltip);
        gui::string_destruct(&value.text);
        value.methods = reinterpret_cast<const Method *>(gui::base_methods);
    }
};

static void __fastcall initialize_starting_grid(Widget *screen, void *) {
    family::dialog_initialize(screen, nullptr);
    at<Dword>(screen, 272) = 0;
    frame_clock::paused = 1;
    if (auto *track = static_cast<Widget *>(pending::find_child(screen, "<TRACK>"))) {
        auto *tracks = system(40);
        pending::LocalFrontendString text;
        gui::string_reference(&text.value, at<Dword>(at<void *>(tracks,
                                  12 + at<Dword>(tracks, 8) * 4), 12));
        invoke<void>(track, 7, &text.value);
    }
    auto *list = static_cast<Widget *>(pending::find_child(screen, "<LIST>"));
    if (!list || !invoke<Dword>(list, 2, family::list_box_class_key))
        return;
    void *race = at<void *>(system(92), 4);
    pending::reset_race_round(race);
    StartingGridItem item;
    Dword slots[24];
    int count = 0;
    const int total = at<int>(system(72), 168);
    for (int slot = 0; slot < total; ++slot) {
        void *rules = system(72), *entry = pending::grid_entry(rules, Dword(slot));
        if (at<Dword>(entry, 8) && !at<Dword>(entry, 0))
            continue;
        if (at<Dword>(entry, 16))
            continue;
        const Dword kind = at<Dword>(rules, 152);
        if ((kind == 10 || kind == 4) &&
            !game::runtime::ai::lifecycle::pending::participant_enabled(race, Dword(slot)))
            continue;
        slots[count++] = Dword(slot);
    }
    if (count)
        std::qsort(slots, count, sizeof(Dword), compare_grid_order);
    auto *items = &reinterpret_cast<family::ListBox *>(list)->items;
    for (int row = 0; row < count; ++row) {
        char number[16];
        std::sprintf(number, "%d", row + 1);
        {
            pending::LocalFrontendString position(number);
            gui::string_copy(&item.value.text, &position.value);
        }
        invoke<void>(items, 7, &item.value, Dword(row), Dword(0));
        gui::String name;
        participant_display_name(race, &name, slots[row]);
        try { gui::string_copy(&item.value.text, &name); }
        catch (...) { gui::string_destruct(&name); throw; }
        gui::string_destruct(&name);
        invoke<void>(items, 7, &item.value, Dword(row), Dword(1));
        pending::LocalFrontendString car;
        participant_car_label(race, slots[row], &car.value);
        gui::string_copy(&item.value.text, &car.value);
        invoke<void>(items, 7, &item.value, Dword(row), Dword(2));
    }
}

Dword __cdecl starting_grid_race_button(void *sender, Dword event, Dword, Dword modifiers) {
    auto *widget = static_cast<Widget *>(sender);
    if (event == 18) {
        pending::set_widget_callback(widget, 43, starting_grid_race_button);
        pending::set_widget_callback(widget, 6, starting_grid_race_button);
        return 0;
    }
    if ((event != 6 || std::uint8_t(modifiers)) && event != 43)
        return 0;
    if (enabled_in_ancestors(widget)) {
        Widget *grid;
        {
            pending::LocalFrontendString type("HP2StartingGrid");
            grid = find_menu_ancestor(widget, &type.value,
                &lifecycle::construction::unresolved::shared_string, 0);
            if (grid && !invoke<Dword>(grid, 2, starting_grid_class_key))
                grid = nullptr;
        }
        if (grid && !grid->modal_result)
            family::finish_modal(grid, 20, 0);
    }
    return 1;
}

static void __fastcall tick_starting_grid(Widget *screen, void *, Dword ticks) {
    family::dialog_tick(screen, nullptr, ticks);
    if (!screen->modal_result)
        game::advance();
}

static void __fastcall draw_starting_grid(Widget *, void *, void *engine) {
    invoke<void>(engine, 56, Dword(6), Dword(0), Dword(0));
    draw_race_frame();
    invoke<void>(engine, 56, Dword(5), Dword(0), Dword(0));
}

static void destroy_starting_grid(Widget *screen) {
    screen->methods = starting_grid_methods.data();
    family::dialog_destroy(screen);
}

static Widget *__fastcall delete_starting_grid(Widget *screen, void *, Dword flags) {
    destroy_starting_grid(screen);
    if (flags & 1) gui::release(screen);
    return screen;
}

const char *__cdecl starting_grid_class_key() { return "HP2StartingGrid"; }

static const char *__fastcall starting_grid_name(Widget *, void *) { return "HP2StartingGrid"; }

static Dword __fastcall starting_grid_is_type(Widget *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(starting_grid_class_key) ||
           type == reinterpret_cast<const void *>(family::dialog_class_key) ||
           type == reinterpret_cast<const void *>(general_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

static Dword __fastcall starting_grid_is_name(Widget *, void *, const char *name) {
    return !std::strncmp(name,"HP2StartingGrid",16) || !std::strncmp(name,"GDialog",8) ||
           !std::strncmp(name,"GWidget",8) || !std::strncmp(name,"GUI",4);
}

const std::array<Method, 49> starting_grid_methods = {
    reinterpret_cast<Method>(::nfs::application::frontend::delete_starting_grid), // 0.
    reinterpret_cast<Method>(::nfs::application::frontend::starting_grid_name), // 1.
    reinterpret_cast<Method>(::nfs::application::frontend::starting_grid_is_type), // 2.
    reinterpret_cast<Method>(::nfs::application::frontend::starting_grid_is_name), // 3.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::dialog_load), // 4.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::dialog_save), // 5.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::layout), // 6.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_text), // 7.
    reinterpret_cast<Method>(::nfs::gui::widgets::text), // 8.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::hit_test), // 9.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::navigate), // 10.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::dialog_enter_modal), // 11.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::dialog_leave_modal), // 12.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::dialog_event), // 13.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::dialog_configure_geometry), // 14.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_two_arguments), // 15.
    reinterpret_cast<Method>(::nfs::gui::widgets::apply_drawing_state), // 16.
    reinterpret_cast<Method>(::nfs::gui::widgets::prepare_draw), // 17.
    reinterpret_cast<Method>(::nfs::gui::widgets::push_draw_bounds), // 18.
    reinterpret_cast<Method>(::nfs::application::frontend::draw_starting_grid), // 19.
    reinterpret_cast<Method>(::nfs::gui::widgets::pop_draw_bounds), // 20.
    reinterpret_cast<Method>(::nfs::application::frontend::initialize_starting_grid), // 21.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_no_arguments), // 22.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_render_argument), // 23.
    reinterpret_cast<Method>(::nfs::application::frontend::tick_starting_grid), // 24.
    reinterpret_cast<Method>(::nfs::gui::widgets::clear_layout_request), // 25.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_layout_argument), // 26.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_parent_enter), // 27.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::dialog_parent_leave), // 28.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_parent_argument), // 29.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_child_argument), // 30.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_modal_state), // 31.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::dialog_key_down), // 32.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_result), // 33.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_34), // 34.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_35), // 35.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::dialog_button_down), // 36.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_37), // 37.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_38), // 38.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::update_active_style), // 39.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::dialog_pointer_press), // 40.
    reinterpret_cast<Method>(::nfs::gui::widgets::capture_style), // 41.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::dialog_accept), // 42.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::dialog_cancel), // 43.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_scroll), // 44.
    reinterpret_cast<Method>(::nfs::gui::widgets::process_general_command), // 45.
    reinterpret_cast<Method>(::nfs::gui::widgets::process_general_notification), // 46.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::show_dialog), // 47.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::hide_dialog), // 48.
};

void *create_starting_grid(void *parent) {
    auto *screen = static_cast<Widget *>(gui::allocate_flags(288,0,"HP2StartingGrid"));
    if (!screen) return nullptr;
    try {
        gui::widgets::pending::construct_dialog(screen,parent);
        screen->methods=starting_grid_methods.data();
        return screen;
    } catch (...) { gui::release_flags(screen, 0, "HP2StartingGrid"); throw; }
}
} // namespace nfs::application::frontend
