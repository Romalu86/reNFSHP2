#pragma once
#include "nfs/world/lifecycle/world_lifecycle.hpp"
#include "race_render_passes.hpp"
namespace nfs::game::world::frame {
extern float measured_frame_rate;
extern float drawing_frame_rate;
float sample_frame_rate();
void prepare_ai_vehicles(void *, void *);
void draw_ai_vehicle_lights(void *, void *);
void draw_pursuit_overlays(void *, void *);
void draw_actor_list(void *, void *, Dword *);
void draw_character_manager(void *, void *, Dword *);
void draw_spatial_objects(void *, void *, Dword *);
void draw_skid_traces(void *, void *);
void wrap_wheel_turn(float *);
void advance_wheel_turns(void *, float);
void advance_renderable_wheels(void *);
void advance_vehicle_renderables(void *);
Dword read_keyboard_key(int);
void begin_frame_capture(Dword *);
void end_frame_capture(Dword *);
Byte capture_backbuffer(Dword *, const char *, Byte);
Byte reflection_camera_enabled(void *);
Byte dynamic_reflection_enabled();
extern graphics::passes::ReflectionDistance reflection_distance;
void set_camera_field_of_view(float);
void update_reflection_directions(void *, void *);
void draw_environment_face(void *, void *, int);
void draw_environment_reflections(void *, void *, const float *);
struct CollisionOverlay {
    Byte enabled, edges, materials, brightness;
    float scale, origin_x, origin_z, depth;
};
static_assert(sizeof(CollisionOverlay) == 20);
extern CollisionOverlay collision_overlay;
void draw_collision_overlay(CollisionOverlay *);
void draw_camera_frame(void *);
}
