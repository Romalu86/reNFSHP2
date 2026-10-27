#include "frontend_particle_widget.hpp"
#include "nfs/physics/frame/simulation_runtime.hpp"
#include "../../gui/lifecycle/gui_render_numeric_x87.hpp"

#pragma STDC FENV_ACCESS ON
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
namespace nfs::application::frontend {
using namespace gui::widgets;
using namespace gui::engine::pending;
using gui::retained::number;
static const FlagDefinition particle_blends[] = {
    {"GDC_BLEND_NORMAL", 0, 0},
    {"GDC_BLEND_ADDITIVE", 0, 1},
    {nullptr, 0, 0}};
static const FlagDefinition particle_styles[] = {
    {"FEPS_NOWINDUP", 0, 1},
    {"FEPS_ONFOCUS", 0, 2},
    {"FEPS_ONSIBLINGFOCUS", 0, 4},
    {nullptr, 0, 0}};
struct ParticleFloatSetting {
    const char *name;
    unsigned offset;
    float fallback;
};
static constexpr ParticleFloatSetting scalar_settings[] = {
    {"FEP.mSpawnRate", offsetof(ParticleWidget, spawn_rate), 0},
    {"FEP.mLife", offsetof(ParticleWidget, life), 0},
    {"FEP.mMidLife", offsetof(ParticleWidget, middle_life), .5f},
    {"FEP.mFadeInRate", offsetof(ParticleWidget, fade_in), 1},
    {"FEP.mFadeOutRate", offsetof(ParticleWidget, fade_out), 1},
    {"FEP.mAngle", offsetof(ParticleWidget, angle), 0},
    {"FEP.mAngleRand", offsetof(ParticleWidget, angle_randomness), 0},
    {"FEP.mAngleVel", offsetof(ParticleWidget, angular_velocity), 0},
    {"FEP.mAngleSpread", offsetof(ParticleWidget, angular_spread), 0},
    {"FEP.mAngleDrag", offsetof(ParticleWidget, angular_drag), 0},
    {"FEP.mStartSize", offsetof(ParticleWidget, scale), 0},
    {"FEP.mGrowBy", offsetof(ParticleWidget, growth), 0},
    {"FEP.mScatterX", offsetof(ParticleWidget, scatter_x), 0},
    {"FEP.mScatterY", offsetof(ParticleWidget, scatter_y), 0}};

void read_particle_polar(const float *vector, float *direction, float *magnitude) {
    const float y = vector[1], x = vector[0];
    const float squared = float(number(x) * x + number(y) * y);
    const float length = float(__builtin_sqrtl(number(squared)));
    const float direction_y = vector[1], direction_x = vector[0];
    *magnitude = length;
    *direction = float(game::runtime::turn_direction_retained(direction_x, direction_y) * 360.f);
}

void write_particle_polar(float *vector, float direction, float magnitude) {
    const float turns = float(number(direction) * 0.00277777784503996372222900390625f);
    vector[0] = float(game::runtime::turn_sine(turns) * magnitude);
    vector[1] = float(game::runtime::turn_cosine(turns) * magnitude);
}

Dword __fastcall load_particle_widget(ParticleWidget *widget, void *, void *configuration) {
    gui::widgets::read_image_resource(configuration, "FEP.mImage", &widget->image);
    read_integer(configuration, "FEP.mMaxParticles", &widget->maximum_particles, 0);
    read_enum(configuration, "FEP.mBlendMode", &widget->blend, particle_blends, 0);
    read_flags(configuration, "FEP.mFEPStyle", &widget->style, particle_styles, 0);
    const Dword black = 0;
    read_colour(configuration, "FEP.mStartColor", &widget->start_colour, &black);
    read_colour(configuration, "FEP.mMidColor", &widget->middle_colour, &black);
    read_colour(configuration, "FEP.mEndColor", &widget->end_colour, &black);
    for (const auto &setting : scalar_settings)
        read_float(configuration, setting.name, &game::runtime::at<float>(widget, setting.offset),
                   setting.fallback);
    read_point(configuration, "FEP.mCG", &widget->centre, &gui::engine::default_point);
    const struct {
        float *vector;
        const char *magnitude, *direction;
    } polar_settings[] = {{widget->velocity, "FEP.mVelocity", "FEP.mVelocityDir"},
                          {widget->drag, "FEP.mDrag", "FEP.mDragDir"},
                          {widget->acceleration, "FEP.mAcceleration", "FEP.mAccelerationDir"}};
    for (const auto &setting : polar_settings) {
        float direction, magnitude;
        read_particle_polar(setting.vector, &direction, &magnitude);
        read_float(configuration, setting.magnitude, &magnitude, 0);
        read_float(configuration, setting.direction, &direction, 0);
        write_particle_polar(setting.vector, direction, magnitude);
    }
    read_float(configuration, "FEP.mSpread", &widget->spread, 0);
    read_integer(configuration, "FEP.mSortDir", &widget->sort_direction, 0);
    if (widget->maximum_particles > 99)
        widget->maximum_particles = 100;
    if (widget->middle_life < 0.01f)
        widget->middle_life = 0.01f;
    else if (widget->middle_life > 0.99f)
        widget->middle_life = 0.99f;
    if (widget->fade_in < 0)
        widget->fade_in = 0;
    else if (widget->fade_in > 10)
        widget->fade_in = 10;
    if (widget->fade_out < 0)
        widget->fade_out = 0;
    else if (widget->fade_out > 10)
        widget->fade_out = 10;
    allocate_menu_particles(widget);
    return reinterpret_cast<Dword(__thiscall *)(void *, void *)>(family::image_widget_methods[4])(
        widget, configuration);
}
// Serializing polar fields also rewrites their Cartesian values.
Dword __fastcall save_particle_widget(ParticleWidget *widget, void *, void *configuration) {
    gui::widgets::write_image_resource(configuration, "FEP.mImage", &widget->image);
    write_int(configuration, "FEP.mMaxParticles", widget->maximum_particles, 0);
    write_enum(configuration, "FEP.mBlendMode", widget->blend, particle_blends, 0);
    write_flags(configuration, "FEP.mFEPStyle", widget->style, particle_styles, 0);
    const Dword black = 0;
    write_colour(configuration, "FEP.mStartColor", &widget->start_colour, &black);
    write_colour(configuration, "FEP.mMidColor", &widget->middle_colour, &black);
    write_colour(configuration, "FEP.mEndColor", &widget->end_colour, &black);
    for (const auto &setting : scalar_settings)
        write_float(configuration, setting.name, game::runtime::at<float>(widget, setting.offset),
                    2, 0);
    write_point(configuration, "FEP.mCG", &widget->centre, &gui::engine::default_point);
    const struct {
        float *vector;
        const char *magnitude, *direction;
    } polar_settings[] = {{widget->velocity, "FEP.mVelocity", "FEP.mVelocityDir"},
                          {widget->drag, "FEP.mDrag", "FEP.mDragDir"},
                          {widget->acceleration, "FEP.mAcceleration", "FEP.mAccelerationDir"}};
    for (const auto &setting : polar_settings) {
        float direction, magnitude;
        read_particle_polar(setting.vector, &direction, &magnitude);
        write_float(configuration, setting.magnitude, magnitude, 2, 0);
        write_float(configuration, setting.direction, direction, 2, 0);
        write_particle_polar(setting.vector, direction, magnitude);
    }
    write_float(configuration, "FEP.mSpread", widget->spread, 2, 0);
    write_int(configuration, "FEP.mSortDir", widget->sort_direction, 0);
    return reinterpret_cast<Dword(__thiscall *)(void *, void *)>(family::image_widget_methods[5])(
        widget, configuration);
}
} // namespace nfs::application::frontend
