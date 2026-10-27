#pragma once
#include "nfs/graphics/resources/eagl_objects.hpp"
namespace nfs::graphics::bindings {
extern Dword *drawing_viewport;
extern Dword near_plane, far_plane;
extern float half_fov_tangent;
void use_screen_depth();
void use_world_depth();
void begin_world_viewport(Dword *);
void begin_coloured_viewport(Dword *, Dword);
}
namespace nfs::graphics::model_render {
void set_active_target(Dword *);
void enable_render_target(Dword *);
void disable_render_target(Dword *);
}
namespace nfs::graphics::passes {
struct TargetPass {
    Dword *target;
    void *readback;
    Dword *colour_target, *depth_target;
    Dword *viewport;
    float matrix[16];
    float field_of_view, aspect, near_plane, far_plane;
    std::uint8_t enabled, padding[3];
    Dword projection;
};
static_assert(sizeof(TargetPass) == 108 && offsetof(TargetPass, enabled) == 100);
void begin(TargetPass *);
void end(TargetPass *);
void set_camera(TargetPass *, const float *, float, float, float, float);
struct ReflectionDistance {
    std::uint8_t enabled, padding[3];
    float distance;
};
void set_distance_level(ReflectionDistance *, int);
}
namespace nfs::game::world::fog {
struct Transition {
    float origin, difference, increment;
    int frame;
};
struct State {
    const void *methods;
    Transition start_transition, end_transition, density_transition, scale_transition;
    kernel::Dword alpha, red, green, blue;
    float start, finish, density, scale;
    kernel::Dword table_mode, vertex_mode;
    int frames;
    std::uint8_t active, enabled, padding[2];
};
static_assert(sizeof(State) == 116 && offsetof(State, start) == 84 &&
              offsetof(State, active) == 112);
void begin(void *);
void end(void *);
void apply(void *, std::uint8_t, std::uint8_t);
void configure(State *, float, float, float, float, kernel::Dword, kernel::Dword, int);
}
