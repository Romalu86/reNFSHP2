#pragma once
#include "frontend_application_widgets.hpp"
#include "../../gui/widgets/gui_widget_sequence.hpp"
#include "../../gui/widgets/gui_widget_shared_methods.hpp"
namespace nfs::gui::widgets {
Dword read_image_resource(void *, const char *, ImageResource *);
Dword write_image_resource(void *, const char *, ImageResource *);
} // namespace nfs::gui::widgets
namespace nfs::gui::widgets::family {
void *construct_image_widget(void *, void *);
void destroy_image_widget(Widget *);
extern const Method image_widget_methods[47];
Widget *sibling_by_id(Widget *, Dword);
} // namespace nfs::gui::widgets::family
namespace nfs::gui::engine::pending {
Dword read_point(void *, const char *, Point *, const Point *);
Dword write_point(void *, const char *, const Point *, const Point *);
} // namespace nfs::gui::engine::pending
namespace nfs::application::frontend {
struct MenuParticle {
    float x, y, velocity_x, velocity_y;
    float red, green, blue, alpha;
    float remaining, angle, angular_velocity, scale;
    int order;
};
struct ParticleWidget {
    GeneralWidget base;
    Dword image_style;
    float period;
    Dword update_timer, spawn_timer, serial;
    MenuParticle *particles;
    gui::widgets::ImageResource image;
    int maximum_particles;
    Dword start_colour, middle_colour, end_colour;
    float spawn_rate, life, middle_life, fade_in, fade_out;
    float angle, angle_randomness, angular_velocity, angular_spread, angular_drag;
    float velocity[2], drag[2], acceleration[2];
    float spread, scatter_x, scatter_y, scale, growth;
    gui::engine::Point centre;
    Dword blend;
    int sort_direction;
    Dword style;
};
static_assert(sizeof(MenuParticle) == 52 && offsetof(MenuParticle, order) == 48);
static_assert(sizeof(ParticleWidget) == 452 && offsetof(ParticleWidget, image) == 288);
static_assert(offsetof(ParticleWidget, velocity) == 384 &&
              offsetof(ParticleWidget, centre) == 428);
extern const std::array<Method, 47> particle_widget_methods;
float menu_particle_period(float);
void spawn_menu_particle(ParticleWidget *);
void advance_menu_particles(ParticleWidget *, float);
void warm_up_menu_particles(ParticleWidget *);
void allocate_menu_particles(ParticleWidget *);
MenuParticle *menu_particle_at(ParticleWidget *, int);
void draw_menu_particle(MenuParticle *, void *, const gui::engine::Rectangle *,
                        gui::widgets::ImageResource *, Dword, const gui::engine::Point *);
Dword __fastcall load_particle_widget(ParticleWidget *, void *, void *);
Dword __fastcall save_particle_widget(ParticleWidget *, void *, void *);
void __fastcall draw_particle_widget(ParticleWidget *, void *, gui::engine::Engine *);
void read_particle_polar(const float *, float *, float *);
void write_particle_polar(float *, float, float);
} // namespace nfs::application::frontend
