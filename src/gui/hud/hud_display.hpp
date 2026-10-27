#pragma once
#include "hud_instruments.hpp"
#include "../../frontend/flow/frontend_flash_text.hpp"
#include "../../frontend/flow/frontend_race_results.hpp"
#include "nfs/gui/resources/gui_format.hpp"
namespace nfs::gui::engine {
Point *top_left(Point *, const Rectangle *);
}
namespace nfs::application::frontend::hud {
using gui::widgets::family::image_widget_methods;
struct Text {
    FlashText base;
    Binding binding;
    Dword display_type;
};
struct Image {
    GeneralWidget base;
    Dword texture;
};
struct StaticImage {
    Image base;
    Dword image_style;
    gui::widgets::ImageResource image;
    Dword drawing_state;
    float scale_x, scale_y;
};
struct StaticArt {
    StaticImage base;
    Binding binding;
    Dword display_type;
};
struct RearView {
    Image base;
};
struct ProgressPair;
struct ProgressBar {
    StaticImage base;
    Dword timer;
    float fraction;
    ProgressPair *pair;
};
struct ProgressPair {
    ProgressBar *first, *second, *active, *inactive;
    Dword selected;
    Byte blinking, hidden, padding[2];
};
struct Cuffometer {
    GeneralWidget base;
    Binding binding;
    Byte active, padding[3];
    Dword state;
    StaticImage *stars[5];
    ProgressPair progress;
    Dword reserved;
    gui::engine::Point position;
    Dword chase_state;
};
static_assert(sizeof(Text) == 324 && offsetof(Text, binding) == 312);
static_assert(sizeof(StaticImage) == 324 && offsetof(StaticImage, image) == 272);
static_assert(sizeof(StaticArt) == 336 && sizeof(RearView) == 268);
static_assert(sizeof(ProgressBar) == 336 && sizeof(ProgressPair) == 24);
static_assert(sizeof(Cuffometer) == 344 && offsetof(Cuffometer, progress) == 300 &&
              offsetof(Cuffometer, position) == 328);
extern const std::array<Method, 49> text_methods;
extern const std::array<Method, 48> static_art_methods;
extern const std::array<Method, 47> rear_view_methods;
extern const std::array<Method, 48> progress_bar_methods;
extern const std::array<Method, 47> cuffometer_methods;
extern Dword rear_view_id;
const char *__cdecl text_class_key();
const char *__cdecl static_art_class_key();
const char *__cdecl rear_view_class_key();
const char *__cdecl progress_bar_class_key();
const char *__cdecl cuffometer_class_key();
void *create_text(void *);
void *create_static_art(void *);
void *create_rear_view(void *);
void *create_progress_bar(void *);
void *create_cuffometer(void *);
void construct_progress_pair(ProgressPair *);
void toggle_progress_pair(ProgressPair *);
void select_progress_pair(ProgressPair *, Byte, float);
void __fastcall progress_bar_timer(ProgressBar *, void *, Dword);
void __fastcall draw_progress_bar(ProgressBar *, void *, gui::engine::Engine *);
void __fastcall initialize_cuffometer(Cuffometer *, void *);
void update_cuffometer(Cuffometer *);
void __fastcall prepare_cuffometer(Cuffometer *, void *, gui::engine::Engine *);
Dword __fastcall cuffometer_event(Cuffometer *, void *, Dword, Dword, Dword);
void refresh_text(Text *);
void __fastcall initialize_text(Text *, void *);
void __fastcall begin_text(Text *, void *);
Dword __fastcall text_event(Text *, void *, Dword, Dword, Dword);
void update_static_art(StaticArt *);
Dword __fastcall static_art_event(StaticArt *, void *, Dword, Dword, Dword);
void __fastcall prepare_rear_view(RearView *, void *, gui::engine::Engine *);
void __fastcall draw_rear_view(RearView *, void *, gui::engine::Engine *);
void write_mode_text(Text *, Dword, void *, gui::String *, gui::String *);
gui::String *construct_time_text(gui::String *, Dword, int);
gui::String *append_localized_text(gui::String *, const gui::String *, Dword);
} // namespace nfs::application::frontend::hud
