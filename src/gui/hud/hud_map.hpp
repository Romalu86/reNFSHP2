#pragma once
#include "hud_display.hpp"
#include "hud_draw_primitives.hpp"
#include "../../graphics/primitives/gouraud_array_runtime.hpp"
#include "../lifecycle/gui_images.hpp"
#include "nfs/graphics/math/eagl_math.hpp"
namespace nfs::graphics::gouraud {
void read_vertex(Dword *, Dword, float *, Dword *);
}
namespace nfs::gui::engine::pending {
extern const Destroy coordinate_methods[];
}
namespace nfs::application::frontend::hud {
struct Map {
    GeneralWidget base;
    Dword vertex_count;
    float half_width, half_height, centre_x, centre_z, zoom;
    float zoom_history[4];
    float pursuit_zoom, pursuit_distance, zoom_rate;
    float heading_history[4], pitch;
    float translation[3];
    Dword style;
    gui::widgets::ImageResource car_shape, opponent_shape, perpetrator_shape, cop_shape;
    Dword background, spline_colour;
    int section_detail_limit;
    Byte reversed, first_draw, padding[2];
    Dword *geometry;
    Dword pool;
    float extent;
};
static_assert(sizeof(Map) == 540 && offsetof(Map, vertex_count) == 264 &&
              offsetof(Map, car_shape) == 352 && offsetof(Map, geometry) == 528);
extern const std::array<Method, 47> map_methods;
const char *__cdecl map_class_key();
void *create_map(void *);
void __fastcall initialize_map(Map *, void *);
void __fastcall release_map(Map *, void *);
void __fastcall tick_map(Map *, void *, Dword);
void __fastcall prepare_map(Map *, void *, gui::engine::Engine *);
void __fastcall draw_map(Map *, void *, gui::engine::Engine *);
Dword __fastcall load_map(Map *, void *, void *);
Dword __fastcall save_map(Map *, void *, void *);
void build_map_geometry(Map *);
void clear_map_geometry(Map *);
void map_position(Map *, float *, const float *);
void set_map_matrix(Map *, const Dword *);
kernel::numeric::Extended map_car_heading(Map *);
void update_map_tracking(Map *, float, Byte);
kernel::numeric::Extended filtered_map_value(const float *);
void draw_map_start_line(Map *, void *, const float *, const float *);
Dword *map_vehicle_colour(Dword *, void *);
void draw_map_vehicle(Map *, void *, void *, Dword, const gui::widgets::ImageResource *);
void draw_map_start_lines(Map *, void *);
Byte map_cop_targets_player(void *);
void draw_map_vehicle_kind(Map *, void *, Dword, const gui::widgets::ImageResource *);
Dword map_camera_visible(void *);
Dword map_camera_hidden(int);
Byte map_car_is_racer(void *);
gui::String *localized_prefix(gui::String *, Dword, const gui::String *);
gui::engine::images::TexturePoint *construct_map_coordinate(gui::engine::images::TexturePoint *);
void append_map_basis(float *, const float *, const float *);
void prepend_map_translation(float *, float, float, float);
} // namespace nfs::application::frontend::hud
