#pragma once
#include "../screens/frontend_menu_pages.hpp"
#include "../../gui/widgets/gui_widget_image_list.hpp"
#include "../../gui/widgets/gui_widget_resources.hpp"
#include "../../core/lifecycle/kernel_values.hpp"
#include "../../math/precision/extended_float.hpp"
namespace nfs::gui::widgets::family {
struct ListBox;
void *construct_image_widget(void *, void *);
void destroy_image_widget(Widget *);
} // namespace nfs::gui::widgets::family
namespace nfs::gui::engine {
Rectangle *to_pixels(Rectangle *, const Rectangle *);
}
namespace nfs::game::world::camera {
void apply_menu_camera(void *, kernel::Dword *, float, float);
}
namespace nfs::application::frontend {
using SceneVector = game::runtime::values::VectorValue;
extern SceneVector scene_default_vector;
struct SceneLight {
    SceneVector colour, direction;
    float intensity;
};
struct SceneWidget {
    GeneralWidget base;
    Dword image_style;
    SceneVector position, initial_position, angles;
    float rotation[9];
    SceneVector angular_velocity, angular_step, camera_position, initial_camera;
    float camera_matrix[16], pitch;
    std::uint8_t rotates, mirrored, drag_flags[2];
    float camera_origin[3], zoom_step;
    SceneVector ambient_sky, ambient_ground, sun_position, sun_colour;
    Dword lighting_flags[2];
    SceneLight lights[3];
    int drag_x, drag_y;
    SceneVector drag_angles;
    float drag_velocity;
    Dword drag_clock;
    int previous_x;
    Dword rotation_timer;
    float rotation_angle, requested_zoom, zoom;
};
struct ActorPreview {
    SceneWidget base;
    void *actor;
    float extent;
    gui::String resource;
    Dword animation;
    float frame, frame_step;
    std::uint8_t loop, padding[3];
    gui::String offset_type;
};
struct CarPreview {
    SceneWidget base;
    void *actor;
    gui::String resource;
};
struct TrackSelection {
    Widget base;
    int track, direction;
    gui::String track_name;
    void *preferences;
    Widget *lock, *balance, *cost;
    ProfileToggle *laps;
    gui::widgets::family::ListBox *playlist;
    void *event_progress;
    ActorPreview *preview;
    Widget *track_images;
    struct ImageWheel *direction_wheel;
    TypeSpinControl *track_selector;
    Ticker *description;
    std::uint8_t preview_dirty, padding[3];
    Widget *next;
    void *tracks;
};
struct ColourWheel {
    GeneralWidget base;
    Dword swatches[3], up, down, previous, next;
    int selected, count;
    Dword colours[25];
    gui::String names[25];
};
struct MenuImageList {
    gui::widgets::family::ImageList base;
    int navigation_count;
    Dword reserved, left, right;
};
struct ImageWheel {
    GeneralWidget base;
    Dword up, down;
    int selection, count;
    Widget *image;
};
static_assert(sizeof(SceneWidget) == 736 && offsetof(SceneWidget, lights) == 576);
static_assert(sizeof(ActorPreview) == 800 && sizeof(CarPreview) == 760);
static_assert(sizeof(TrackSelection) == 344);
static_assert(sizeof(ColourWheel) == 900 && sizeof(MenuImageList) == 404);
static_assert(sizeof(ImageWheel) == 284);
extern const std::array<Method, 47> scene_widget_methods, actor_preview_methods,
    car_preview_methods;
extern const std::array<Method, 47> colour_wheel_methods, image_wheel_methods;
extern const std::array<Method, 42> track_selection_methods;
extern const std::array<Method, 51> menu_image_list_methods;
const char *__cdecl scene_widget_class_key();
const char *__cdecl actor_preview_class_key();
const char *__cdecl car_preview_class_key();
const char *__cdecl track_selection_class_key();
const char *__cdecl colour_wheel_class_key();
const char *__cdecl image_wheel_class_key();
const char *__cdecl menu_image_list_class_key();
SceneWidget *construct_scene_widget(SceneWidget *, void *);
void destroy_scene_widget(SceneWidget *);
void __fastcall initialize_scene_widget(SceneWidget *, void *);
void __fastcall tick_scene_widget(SceneWidget *, void *, Dword);
void __fastcall draw_scene_widget(SceneWidget *, void *, gui::engine::Engine *);
void __fastcall release_scene_widget(SceneWidget *, void *);
Dword __fastcall load_scene_widget(SceneWidget *, void *, void *);
Dword __fastcall save_scene_widget(SceneWidget *, void *, void *);
Dword __fastcall cursor_scene_widget(SceneWidget *, void *, void *);
Dword __fastcall drag_scene_widget(SceneWidget *, void *, Dword, Dword);
Dword __fastcall press_scene_widget(SceneWidget *, void *, Dword, Dword);
void __fastcall release_scene_pointer(SceneWidget *, void *, Dword, Dword);
void __fastcall leave_scene_widget(SceneWidget *, void *, Dword);
void update_scene_camera(SceneWidget *);
std::uint8_t reset_scene_camera(SceneWidget *);
using game::runtime::values::assign_vector_value;
void begin_scene_drag(SceneWidget *, const gui::engine::Point *);
void update_scene_drag(SceneWidget *, const gui::engine::Point *);
void end_scene_drag(SceneWidget *);
void rotate_scene_basis_z(const float *, float, float *);
void scene_euler_angles(const float *, float *, float *, float *);
void scene_euler_matrix(float, float, float, float *);
Dword read_scene_vector(void *, const char *, SceneVector *, const SceneVector *);
Dword write_scene_vector(void *, const char *, const SceneVector *, int, const SceneVector *);
void __fastcall initialize_menu_image_list(MenuImageList *, void *);
Dword __fastcall menu_image_list_event(MenuImageList *, void *, Dword, Dword, Dword);
Dword __fastcall menu_image_list_key(MenuImageList *, void *, Dword, Dword);
Dword __fastcall menu_image_list_key_up(MenuImageList *, void *, Dword, Dword);
void __fastcall menu_image_list_child(MenuImageList *, void *, Widget *, Dword, Dword, Dword);
void __fastcall menu_image_list_press(MenuImageList *, void *, Dword, Dword);
void __fastcall menu_image_list_release(MenuImageList *, void *, Dword, Dword);
void __fastcall initialize_image_wheel(ImageWheel *, void *);
Dword __fastcall image_wheel_event(ImageWheel *, void *, Dword, Dword, Dword);
void __fastcall image_wheel_child(ImageWheel *, void *, Widget *, Dword, Dword, Dword);
Dword __fastcall image_wheel_key(ImageWheel *, void *, Dword, Dword);
void __fastcall initialize_colour_wheel(ColourWheel *, void *);
Dword __fastcall colour_wheel_event(ColourWheel *, void *, Dword, Dword, Dword);
void __fastcall colour_wheel_child(ColourWheel *, void *, Widget *, Dword, Dword, Dword);
Dword __fastcall colour_wheel_key(ColourWheel *, void *, Dword, Dword);
void refresh_colour_wheel(ColourWheel *);
void clear_colour_wheel(ColourWheel *);
void append_colour_wheel(ColourWheel *, gui::String, Dword);
void colour_wheel_value(ColourWheel *, Dword *, int);
ActorPreview *construct_actor_preview(ActorPreview *, void *);
void destroy_actor_preview(ActorPreview *);
void load_preview_actor(ActorPreview *);
void set_preview_actor(ActorPreview *, gui::String);
void release_preview_actor(ActorPreview *);
void __fastcall initialize_actor_preview(ActorPreview *, void *);
void __fastcall tick_actor_preview(ActorPreview *, void *, Dword);
void __fastcall draw_actor_preview(ActorPreview *, void *, gui::engine::Engine *);
Dword __fastcall load_actor_preview(ActorPreview *, void *, void *);
Dword __fastcall save_actor_preview(ActorPreview *, void *, void *);
int selected_type_spin_row(TypeSpinControl *);
Dword type_spin_count(TypeSpinControl *);
Dword type_spin_value(TypeSpinControl *, int);
void select_type_spin(TypeSpinControl *, int);
void enable_type_spin_value(TypeSpinControl *, int, std::uint8_t);
void clear_track_playlist(gui::widgets::family::ListBox *, int);
bool track_playlist_empty(gui::widgets::family::ListBox *);
bool track_playlist_full(gui::widgets::family::ListBox *);
Dword next_track_playlist_row(gui::widgets::family::ListBox *);
void unpack_track_choice(Dword, Dword *, Dword *);
void read_track_playlist_row(gui::widgets::family::ListBox *, Dword, Dword *, Dword *, Dword *);
Dword append_track_playlist(gui::widgets::family::ListBox *, const gui::String *, Dword, Dword, int);
void remove_last_track_playlist(gui::widgets::family::ListBox *);
TrackSelection *construct_track_selection(TrackSelection *, void *);
void destroy_track_selection(TrackSelection *);
void refresh_track_selection(TrackSelection *);
void populate_track_laps(TrackSelection *);
void __fastcall initialize_track_selection(TrackSelection *, void *);
Dword __fastcall track_selection_event(TrackSelection *, void *, Dword, Dword, Dword);
Dword __fastcall track_selection_key(TrackSelection *, void *, Dword, Dword);
void __fastcall update_track_selection(TrackSelection *, void *, Dword);
Dword __fastcall finish_track_selection(TrackSelection *, void *, Dword, Dword);
Dword purchase_selected_track(TrackSelection *);
CarPreview *construct_car_preview(CarPreview *, void *);
void destroy_car_preview(CarPreview *);
void clear_car_preview(CarPreview *);
void load_car_preview(CarPreview *, Dword);
void light_car_preview(CarPreview *, std::uint8_t);
void zoom_car_preview(CarPreview *);
void __fastcall initialize_car_preview(CarPreview *, void *);
void __fastcall release_car_preview(CarPreview *, void *);
void __fastcall tick_car_preview(CarPreview *, void *, Dword);
void __fastcall draw_car_preview(CarPreview *, void *, gui::engine::Engine *);
Dword __fastcall load_car_preview_configuration(CarPreview *, void *, void *);
Dword __fastcall save_car_preview_configuration(CarPreview *, void *, void *);
} // namespace nfs::application::frontend
