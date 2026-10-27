#pragma once
#include "../flow/frontend_track_selection.hpp"
#include "../../race/session/race_session.hpp"
#include "../../gui/lifecycle/gui_configuration_values.hpp"
namespace nfs::application::frontend {
struct CarSelection {
    Widget base;
    int car, selection, previous_car, preview_car, preview_skin, preview_delay;
    std::uint8_t delay_enabled, padding_119[3];
    gui::String descriptions[38];
    int skin;
    std::uint8_t police, preview_dirty, padding_41a[2];
    Dword player;
    void *preferences, *vehicles;
    ColourWheel *colours;
    Widget *preview_hint, *balance, *cost, *lock, *rotation_hint, *showcase;
    ProfileToggle *transmission;
    Widget *basic_colour;
    Widget *logo;
    MenuImageList *car_list;
    CarPreview *preview;
    Ticker *statistics;
};
struct TournamentStatus {
    Widget base;
};
struct TreeWidget;
struct TreeRace {
    int bronze_branch;
    std::uint8_t trunk_unlocked, padding_5[3];
    int criteria[18];
    int rewards[3];
    Dword type, opponents, car_class, skill, objective, cop_intensity, race_count, title, time_limit;
    Dword roadblocks, helicopters, backup, traffic;
    Dword tracks[8][7];
    int car_count;
    Dword cars[24];
    int opponent_car_count;
    Dword opponent_cars[24];
    Dword reserved_238, trunk, branches[2], navigation[4], status;
    std::uint8_t processed, padding_25d[3];
};
struct TreeScreen {
    Widget base;
    Dword race_names[14];
    int mode;
    Dword skill_names[3];
    Widget *subtitle;
    TreeWidget *tree;
    Widget *description;
    void *preferences;
    int selection;
    std::uint8_t purchasing, padding_15d[3];
    TreeRace *races;
    int delay;
};
struct TreeEventDialog {
    Widget base;
    alignas(4) std::uint8_t dialog_state[32];
    gui::String resource;
    TreeRace *event;
    Dword selection;
};
struct TreeWidget {
    ActorPreview base;
    void *model_images;
    gui::String resource, camera;
    void *texture_bank, *selected_image;
    float rotation;
    int selection;
    float camera_depth;
};
static_assert(sizeof(CarSelection) == 1116 && offsetof(CarSelection, preview) == 1108);
static_assert(sizeof(TreeScreen) == 360 && offsetof(TreeScreen, selection) == 344);
static_assert(sizeof(TreeEventDialog) == 316 && sizeof(TreeWidget) == 864);
static_assert(sizeof(TreeRace) == 608);
extern const std::array<Method, 42> car_selection_methods, tournament_status_methods,
    tree_screen_methods;
extern const std::array<Method, 49> tree_event_dialog_methods;
extern const std::array<Method, 47> tree_widget_methods;
const char *__cdecl car_selection_class_key();
const char *__cdecl tournament_status_class_key();
const char *__cdecl tree_screen_class_key();
const char *__cdecl tree_event_dialog_class_key();
const char *__cdecl tree_widget_class_key();
CarSelection *construct_car_selection(CarSelection *, void *);
void destroy_car_selection(CarSelection *);
Dword __fastcall load_car_selection(CarSelection *, void *, void *);
Dword __fastcall save_car_selection(CarSelection *, void *, void *);
void __fastcall car_selection_child(CarSelection *, void *, Widget *, Dword, Dword, Dword);
void __fastcall initialize_car_selection(CarSelection *, void *);
void __fastcall resume_car_selection(CarSelection *, void *);
void __fastcall update_car_selection(CarSelection *, void *, Dword);
Dword __fastcall finish_car_selection(CarSelection *, void *, Dword, Dword);
Dword __fastcall car_selection_key(CarSelection *, void *, Dword, Dword);
Dword __fastcall car_selection_key_up(CarSelection *, void *, Dword, Dword);
void populate_car_selection(CarSelection *);
void refresh_car_selection(CarSelection *);
void update_car_preview(CarSelection *);
void refresh_car_colours(CarSelection *);
void refresh_selected_skin(CarSelection *);
void flush_car_selection(CarSelection *);
void show_car_statistics(CarSelection *, std::uint8_t);
Dword can_select_car(CarSelection *, std::uint8_t);
Dword purchase_selected_car(CarSelection *);
void car_purchase_error();
void commit_selected_car(CarSelection *);
gui::String *car_logo_name(gui::String *, Dword);
gui::String *car_frame_name(gui::String *, Dword);
inline void *car_preferences(CarSelection *screen) {
    return pending::player_preferences(game::runtime::system(72), screen->player);
}
inline Dword car_cost(CarSelection *screen) {
    void *record = game::runtime::bytes(screen->vehicles) + Dword(screen->car) * 1228;
    Dword cost = game::runtime::at<Dword>(record, 44);
    if (screen->selection > 37 && int(cost) <= game::runtime::at<int>(record, 48))
        cost = game::runtime::at<Dword>(record, 48);
    return cost;
}
inline bool car_unlocked(CarSelection *screen) {
    return std::uint8_t(services::special_race_mode(game::runtime::system(72))) ||
           game::runtime::at<std::uint8_t>(screen->preferences, 7328 + Dword(screen->selection));
}
void align_menu_image_list(Widget *);
Dword selected_menu_image(Widget *);
Dword menu_image_value(Dword);
int menu_image_index(Widget *, Dword);
void refresh_menu_image_navigation(MenuImageList *);
void play_car_description(void *, Dword);
std::uint8_t catalogue_entry_enabled(void *, int);
std::uint8_t catalogue_entry_traffic(void *, int);
const char *catalogue_entry_resource(void *, int);
Dword catalogue_resource_make(void *, const char *);
Dword catalogue_resource_name(void *, const char *);
Dword catalogue_specification(void *, int, Dword);
const char *__cdecl flash_text_class_key();
gui::engine::Point *menu_rectangle_center(const gui::engine::Rectangle *, gui::engine::Point *);
extern Dword tournament_page_state;
TournamentStatus *construct_tournament_status(TournamentStatus *, void *);
void destroy_tournament_status(TournamentStatus *);
void __fastcall tournament_status_child(TournamentStatus *, void *, Widget *, Dword, Dword, Dword);
void __fastcall initialize_tournament_status(TournamentStatus *, void *);
Dword __fastcall tournament_status_key(TournamentStatus *, void *, Dword, Dword);
Dword event_round_points(void *, Dword, Dword);
bool event_round_failed(void *, Dword, Dword);
std::uint16_t event_stage_time(void *, Dword);
std::int16_t event_best_lap_time(void *, Dword);
Dword event_round_position(void *, Dword, Dword);
extern Dword tournament_sort_round;
int __cdecl compare_tournament_round(const void *, const void *);
int __cdecl compare_grid_order(const void *, const void *);
const char *tournament_driver_name(Dword);
void format_tournament_time(int, char *);
TreeScreen *construct_tree_screen(TreeScreen *, void *);
void destroy_tree_screen(TreeScreen *);
void __fastcall initialize_tree_screen(TreeScreen *, void *);
void __fastcall resume_tree_screen(TreeScreen *, void *);
void __fastcall tick_tree_screen(TreeScreen *, void *, Dword);
Dword __fastcall tree_screen_event(TreeScreen *, void *, Dword, Dword, Dword);
void __fastcall tree_screen_child(TreeScreen *, void *, Widget *, Dword, Dword, Dword);
Dword __fastcall tree_screen_key(TreeScreen *, void *, Dword, Dword);
Dword __fastcall tree_screen_key_up(TreeScreen *, void *, Dword, Dword);
Dword __fastcall finish_tree_screen(TreeScreen *, void *, Dword, Dword);
TreeRace *initialize_tree_race(TreeRace *);
void load_tree_events(TreeScreen *, gui::String);
void read_tree_text_list(gui::String, Dword *, int);
Widget *tree_race_button(TreeScreen *, int);
void enable_tree_selection(TreeScreen *, std::uint8_t);
void refresh_tree_selection(TreeScreen *);
void open_tree_event(TreeScreen *);
TreeEventDialog *construct_tree_event_dialog(TreeEventDialog *, void *);
void destroy_tree_event_dialog(TreeEventDialog *);
void __fastcall initialize_tree_event_dialog(TreeEventDialog *, void *);
Dword __fastcall finish_tree_event_dialog(TreeEventDialog *, void *, Dword, Dword);
void populate_tree_event_dialog(TreeEventDialog *);
TreeWidget *construct_tree_widget(TreeWidget *, void *);
void destroy_tree_widget(TreeWidget *);
void __fastcall initialize_tree_widget(TreeWidget *, void *);
void __fastcall resume_tree_widget(TreeWidget *, void *);
Dword __fastcall load_tree_widget(TreeWidget *, void *, void *);
Dword __fastcall save_tree_widget(TreeWidget *, void *, void *);
void __fastcall draw_tree_widget(TreeWidget *, void *, gui::engine::Engine *);
gui::String *tree_event_image_name(gui::String *, Dword);
gui::String *tree_node_name(gui::String *, int);
Dword bind_tree_event_image(TreeWidget *, int, Dword, std::uint8_t);
Dword bind_tree_medal_image(TreeWidget *, int, Dword);
void animate_tree_selection(TreeWidget *);
void begin_tree_exit(TreeWidget *);
void zoom_tree_selection_in(TreeWidget *);
void zoom_tree_selection_out(TreeWidget *);
} // namespace nfs::application::frontend
