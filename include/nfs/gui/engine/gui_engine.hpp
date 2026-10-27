#pragma once
#include "nfs/gui/lifecycle/gui_modal.hpp"
#include "nfs/graphics/fonts/font_faces.hpp"
#include <cstddef>
#include <initializer_list>
namespace nfs::gui::engine {
using Byte = std::uint8_t;
using Method = void(__cdecl *)();
struct Rectangle {
    const Destroy *methods;
    int top, left, width, height;
};
struct Point {
    const Destroy *methods;
    int x, y;
};
struct Base {
    const Method *methods;
    Rectangle clips[32];
    Dword clip_count;
    Rectangle viewports[32];
    Dword viewport_count;
};
struct FontStyle {
    float scale_x, scale_y;
    Dword flags;
    float height, spacing;
    int font;
    Dword handle;
};
struct Palette {
    const Destroy *methods;
    Dword colours[12];
    String name;
    Dword handle;
};
struct Engine {
    Base base;
    Container font_styles, palettes;
    FontStyle default_font;
    FontStyle *font;
    Palette default_palette;
    Palette *palette;
    Container textures;
    Dword drawing_state, reserved_5b0, drawing_depth;
    Container images;
    Dword raster_inset, image_format;
    Byte image_scratch[480];
    Dword reserved_7b0;
    Rectangle frame_bounds;
    Byte vertex_scratch[680];
    String resource_name;
    Rectangle resource_bounds;
    Dword active_texture;
    Dword flat_state[17], textured_state[17], additive_state[17];
    Dword textured_draw[152 / 4];
    Dword *viewport;
    float matrix[16];
    Dword *context;
    void(__cdecl *frame_callback)();
    float scale_x, scale_y;
    Byte clear_frame, padding[3];
};
static_assert(sizeof(Rectangle) == 20 && sizeof(Point) == 12 && sizeof(Base) == 1292);
static_assert(sizeof(FontStyle) == 28 && sizeof(Palette) == 76 && sizeof(Engine) == 3160);
static_assert(offsetof(Engine, default_palette) == 1356 && offsetof(Engine, images) == 1464);
static_assert(offsetof(Engine, resource_name) == 2672 && offsetof(Engine, textured_draw) == 2920);
static_assert(offsetof(Engine, context) == 3140 && offsetof(Engine, clear_frame) == 3156);
extern const Destroy rectangle_methods[], point_methods[], palette_methods[];
extern const Method base_methods[73], methods[79];
extern Rectangle default_bounds, source_bounds, destination_bounds;
extern Point default_point;
extern float horizontal_scale, vertical_scale, inverse_horizontal_scale, inverse_vertical_scale;
extern float horizontal_offset, vertical_offset;
extern Dword palette_serial, caret_visible, caret_deadline;
Rectangle *construct_rectangle(Rectangle *);
Rectangle *construct_rectangle_bounds(Rectangle *, int left, int top, int width, int height);
void destroy_rectangle(Rectangle *);
Point *construct_point(Point *);
void destroy_point(Point *);
void destroy_coordinate(Point *);
Palette *construct_palette(Palette *);
void destroy_palette(Palette *);
Base *construct_base(Base *);
void destroy_base(Base *);
Engine *construct(Engine *, Dword *);
void destroy(Engine *);
void configure_coordinates(const Rectangle *, const Rectangle *);
int find_font(const String *);
fonts::Font *get_font(int);
fonts::faces::Face *get_font_face(int);
long double font_height(int, float);
void uppercase(String *);
namespace pending {
Dword character_at(const String *, int);
void *create_configuration(void *, const String *);
void destroy_configuration(void *);
void select_section(void *, const char *);
Dword read_rectangle(void *, const char *, Rectangle *, const Rectangle *);
void __fastcall visit_font_styles(Engine *, void *, void(__cdecl *)(void *, Dword, Dword, Dword), Dword,
                                  Dword);
void __fastcall visit_fonts(Engine *, void *, void(__cdecl *)(void *, Dword, Dword), Dword,
                            Dword);
Dword __fastcall find_font_style(Engine *, void *, const String *);
String *__fastcall font_style_name(Engine *, void *, String *, Dword);
Dword __fastcall create_font_style(Engine *, void *, Dword, Dword, Dword, Dword, Dword, Dword, Dword, Dword,
                                  Dword);
Dword __fastcall query_font_style(Engine *, void *, Dword, Dword, Dword, Dword, Dword, Dword, Dword, Dword,
                                 Dword,
                                 Dword);
Dword __fastcall update_font_style(Engine *, void *, Dword, Dword, Dword, Dword, Dword, Dword, Dword,
                                  Dword);
Dword __fastcall save_font_style(Engine *, void *, const String *);
Dword __fastcall select_font_style(Engine *, void *, Dword);
Dword __fastcall measure_string(Engine *, void *, Dword, Dword);
Dword __fastcall glyph_metrics(Engine *, void *, Dword, Dword *);
Dword __fastcall font_pixel_height(Engine *, void *);
Dword __fastcall copy_palette(Engine *, void *, Dword, Dword, Dword);
Dword __fastcall find_palette(Engine *, void *, const String *);
String *__fastcall palette_name(Engine *, void *, String *, Dword);
Dword __fastcall select_palette(Engine *, void *, Dword);
Dword __fastcall save_palette(Engine *, void *, const String *);
Dword __fastcall load_image(Engine *, void *, const String *);
Dword __fastcall acquire_image(Engine *, void *, Dword);
String *__fastcall image_name(Engine *, void *, String *, Dword);
Dword __fastcall query_image(Engine *, void *, Dword, Dword, Dword);
void __fastcall release_image(Engine *, void *, Dword);
Dword __fastcall load_texture(Engine *, void *, const String *);
Dword __fastcall acquire_texture(Engine *, void *, Dword);
void __fastcall release_texture(Engine *, void *, Dword);
Dword __fastcall texture_frame_count(Engine *, void *, Dword, Dword *);
Dword __fastcall texture_frame(Engine *, void *, Dword, int);
Dword __fastcall bind_image_frame(Engine *, void *, Dword, Dword, int);
Dword __fastcall find_texture_frame(Engine *, void *, Dword, const String *);
Byte __fastcall image_uses_texture(Engine *, void *, Dword, Dword);
String *__fastcall texture_name(Engine *, void *, String *, Dword);
void __fastcall draw_string(Engine *, void *, Dword, Dword, Dword, Dword, Dword);
void __fastcall draw_image_quad(Engine *, void *, Dword, Dword, Dword, Dword, Dword, Dword,
                                Dword);
void __fastcall draw_image_uv(Engine *, void *, Dword, Dword, Dword, Dword, Dword, Dword);
void __fastcall draw_image_rectangle(Engine *, void *, Dword, Dword, Dword, Dword);
void __fastcall draw_line(Engine *, void *, Dword, Dword, Dword, Dword);
void __fastcall draw_rectangle(Engine *, void *, Dword, Dword, Dword, Dword, Dword, Dword);
void __fastcall reset_resources(Engine *, void *);
void __fastcall begin_frame(Engine *, void *);
void __fastcall end_frame(Engine *, void *);
void __fastcall clear_resources(Engine *, void *);
Dword __fastcall control_rendering(Engine *, void *, Dword, Dword, Dword);
void draw_root(void *, void *, Dword);
void draw_overlay(void *);
void finish_window_frame();
void finish_input_frame();
void __fastcall tick_root(Engine *, void *, void *);
} // namespace pending
} // namespace nfs::gui::engine
