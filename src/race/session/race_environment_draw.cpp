#include "race_frame_draw.hpp"
#include "race_world_draw.hpp"
#include "nfs/graphics/math/eagl_math.hpp"
#include "nfs/graphics/primitives/immediate_draw.hpp"
#include <cstring>
namespace nfs::game::world::camera {
void menu_camera_matrix(void *, float *);
}
namespace nfs::game::world::frame {

void draw_environment_face(void *environment, void *director, int face) {
    if (!at<Byte>(environment, 4))
        return;
    Dword *viewport = graphics::main_viewport;
    auto *pass = at<graphics::passes::TargetPass *>(environment, 400) + face;
    graphics::main_viewport = pass->viewport;
    float matrix[16];
    camera::menu_camera_matrix(offset(environment, 88), matrix);
    pass = at<graphics::passes::TargetPass *>(environment, 400) + face;
    graphics::passes::set_camera(pass, matrix, at<float>(environment, 116),
                                  at<float>(environment, 136), at<float>(environment, 140),
                                  at<float>(environment, 144));
    graphics::passes::begin(pass);
    if (face == 2) {
        track_render::draw_opaque_track(subsystem(36), director);
        draw_spatial_objects(runtime::track_collision, director, graphics::main_viewport);
        void *vehicles = subsystem(52);
        at<float>(vehicles, 81308) = 25;
        at<Byte>(vehicles, 81297) = 0;
        prepare_ai_vehicles(subsystem(76), director);
        draw_ai_vehicle_lights(subsystem(76), director);
        at<Byte>(subsystem(52), 81297) = 1;
    } else
        track_render::draw_reflection_track(subsystem(36), director);
    graphics::passes::end(pass);
    graphics::main_viewport = viewport;
}

void draw_environment_reflections(void *environment, void *director, const float *position) {
    if (!at<Byte>(environment, 4))
        return;
    std::memcpy(offset(environment, 164), position, 12);
    at<float>(environment, 172) = -at<float>(environment, 172);
    update_reflection_directions(environment, director);
    graphics::passes::set_distance_level(&reflection_distance, 0);
    const Dword frame = ++at<Dword>(environment, 596);
    auto face = [&](int index, unsigned up_offset, unsigned target_offset) {
        std::memcpy(offset(environment, 148), offset(environment, up_offset), 12);
        std::memcpy(offset(environment, 92), offset(environment, 164), 12);
        std::memcpy(offset(environment, 104), offset(environment, target_offset), 12);
        at<float>(environment, 140) = 0.35f;
        at<float>(environment, 144) = 13000;
        at<float>(environment, 116) = 90;
        at<float>(environment, 136) = 1;
        camera::menu_camera_matrix(offset(environment, 88),
                                   reinterpret_cast<float *>(offset(environment, 24)));
        draw_environment_face(environment, director, index);
    };
    if (frame & 1)
        face(2, 272, 200);
    else if (at<Dword>(subsystem(52), 81272) == 2) {
        face(0, 248, 176);
        face(5, 308, 236);
        face(1, 260, 188);
        face(3, 284, 212);
    }
    if (!at<Byte>(environment, 4))
        return;
    float diagonal[16], matrix[16];
    graphics::math::diagonal(diagonal, 0.5f, 0.5f, 1, 1);
    graphics::math::copy(matrix, diagonal);
    auto *pass = reinterpret_cast<graphics::passes::TargetPass *>(offset(environment, 404));
    graphics::passes::set_camera(pass, matrix, graphics::camera_field_of_view, 1.33333337306976318359375f, 1, 15000);
    graphics::passes::begin(pass);
    for (unsigned index = 0; index < 768; index += 128)
        if (index != 512)
            graphics::immediate::draw_array(reinterpret_cast<Dword *>(offset(at<void *>(environment, 512), index)), 294);
    graphics::passes::end(pass);
}
} // namespace nfs::game::world::frame
