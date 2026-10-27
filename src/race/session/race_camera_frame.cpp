#include "race_frame_draw.hpp"
#include "race_legacy_effects.hpp"
#include "race_particle_render.hpp"
#include "race_world_draw.hpp"
#include "nfs/graphics/backend/eagl_backend.hpp"
#include "nfs/graphics/configuration/render_settings.hpp"
#include "nfs/physics/frame/simulation_runtime.hpp"
#include "nfs/graphics/display/viewport_transform.hpp"
namespace nfs::game::world::camera {
void apply_menu_camera(void *, Dword *, float, float);
}
namespace nfs::game::world::frame {
 
static void draw_frame_annotations() {
}

void draw_camera_frame(void *director) {
    drawing_frame_rate = sample_frame_rate();
    camera::apply_menu_camera(director, graphics::main_viewport, 1, 0);
    begin_frame_capture(graphics::context);
    graphics::bindings::begin_world_viewport(graphics::main_viewport);
    graphics::viewports::clear(graphics::main_viewport,
        graphics::startup::stencil_enabled(&graphics::startup::settings, 0) ? 4 : 3);
    graphics::bindings::use_world_depth();
    track_render::apply_track_fog(subsystem(36), at<Dword>(director, 304), director, graphics::main_viewport);
    fog::begin(subsystem(128));
    fog::apply(subsystem(128), 0, 1);
    track_render::draw_opaque_track(subsystem(36), director);
    graphics::viewports::rectangle(graphics::ui_viewport, at<float>(director, 140),
        at<float>(director, 144), at<float>(director, 148), at<float>(director, 152), 0, 1);
    camera::apply_menu_camera(director, graphics::ui_viewport, at<float>(director, 156), .1f);
    graphics::viewports::begin(graphics::ui_viewport);
    draw_skid_traces(subsystem(116), director);
    graphics::viewports::end(graphics::ui_viewport);
    draw_spatial_objects(runtime::track_collision, director, graphics::main_viewport);
    draw_frame_annotations();
    at<float>(subsystem(52), 81308) = at<float>(subsystem(52), 81312);
    prepare_ai_vehicles(subsystem(76), director);
    fog::end(subsystem(128));
    track_render::draw_track_glows(subsystem(36), director);
    graphics::viewports::begin(graphics::ui_viewport);
    draw_ai_vehicle_lights(subsystem(76), director);
    graphics::viewports::end(graphics::ui_viewport);
    draw_world_effects(director);
    runtime::particles::lifecycle::collect_particle_groups(subsystem(64), director);
    runtime::particles::lifecycle::queue_particle_groups(subsystem(64), director);
    graphics::deferred::draw(&graphics::deferred::queue);
    runtime::particles::lifecycle::reset_particle_groups(subsystem(64));
    track_render::draw_track_overlay(subsystem(36), director);
    draw_pursuit_overlays(subsystem(76), director);
    draw_collision_overlay(&collision_overlay);
    graphics::viewports::end(graphics::main_viewport);
    if (dynamic_reflection_enabled() || at<Dword>(subsystem(52), 81272) == 2) {
        void *car = runtime::physics::local_car(static_cast<Dword *>(subsystem(68)), at<Dword>(director, 304));
        if (car) {
            void *body = at<void *>(car, 1768);
            const float position[] = {at<float>(body, 680), at<float>(body, 684), -at<float>(body, 688)};
            draw_environment_reflections(subsystem(32), director, position);
        }
    }
    graphics::bindings::begin_world_viewport(graphics::background_viewport);
    graphics::bindings::use_screen_depth();
    draw_screen_effects(director);
    graphics::viewports::end(graphics::background_viewport);
    end_frame_capture(graphics::context);
}
} // namespace nfs::game::world::frame
