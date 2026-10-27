#include "race_world_draw.hpp"
#include "race_render_passes.hpp"
#include "../../vehicles/rendering/vehicle_renderable_resources.hpp"
#include "../../math/precision/extended_float.hpp"
namespace nfs::game::world::track_render {
 
void prepare_track_directory(void *) {
}
 
void prepare_reflection_directory(void *) {
}
static void draw_sky_model(void *sky, void *camera) {
    float matrix[16];
    const float *pose = invoke<const float *>(camera, 2);
    const auto format = kernel::numeric::current_format();
    const float height = float(kernel::numeric::Extended(pose[2], format) - 12.4f);
    compose_track_transform(matrix, 1, 1, 1, 0, 0, 0, pose[1], height, -pose[3]);
    lifecycle::phase_dependencies::set_track_phase(0);
    fog::apply(subsystem(128), 1, 1);
    vehicle_render::pending::draw_model(sky, reinterpret_cast<Dword *>(matrix));
    fog::apply(subsystem(128), 0, 1);
}

void draw_track_directory(void *directory, void *camera) {
    if (void *sky = at<void *>(directory, 84))
        draw_sky_model(sky, camera);
    if (void *model = at<void *>(directory, 28)) {
        lifecycle::phase_dependencies::set_track_phase(0);
        vehicle_render::pending::draw_model(model, reinterpret_cast<Dword *>(offset(model, 12)));
    }
}

void draw_reflection_directory(void *directory, void *camera) {
    if (void *sky = at<void *>(directory, 84))
        draw_sky_model(sky, camera);
}
// Complete body is RET4.
void draw_directory_overlay(void *, void *) {
}
} // namespace nfs::game::world::track_render
