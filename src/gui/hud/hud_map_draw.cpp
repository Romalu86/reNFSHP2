#include "hud_map.hpp"
#include "../../graphics/cameras/camera_director_runtime.hpp"
#include "nfs/graphics/skids/skid_lifecycle.hpp"
namespace nfs::game::world::actor_resources {
void append_actor_scale(float *, float, float, float, float);
}
namespace nfs::application::frontend::hud {
using game::runtime::invoke;

void append_map_basis(float *matrix, const float *basis, const float *position) {
    float transform[16];
    graphics::math::from_compact_transform(transform, basis, position);
    graphics::math::append(matrix, transform);
}

void prepend_map_translation(float *matrix, float x, float y, float z) {
    float translation[16], result[16];
    graphics::math::translation(translation, x, y, z);
    graphics::math::multiply(translation, matrix, result);
    graphics::math::copy(matrix, result);
}

void __fastcall draw_map(Map *map, void *, gui::engine::Engine *engine) {
    invoke(engine, 48, gui::engine::current_viewport_bounds(&engine->base), &map->background,
           Dword(1));
    const Dword overlay = (map->background & 16777215) | 2667577344u;
    invoke(engine, 48, gui::engine::current_viewport_bounds(&engine->base), &overlay, Dword(0));
    if (!map->geometry || !map->vertex_count)
        return;
    invoke(engine, 64, gui::engine::current_viewport_bounds(&engine->base));
    invoke(engine, 56, Dword(1), Dword(1), Dword(0));
    float matrix[16], rotation[9], pitch[9];
    std::memcpy(matrix, game::runtime::skids::lifecycle::identity_matrix, sizeof(matrix));
    static const float identity[9] = {1, 0, 0, 0, 1, 0, 0, 0, 1}, zero[3] = {0, 0, 0};
    rotate_scene_basis_z(identity, float(filtered_map_value(map->heading_history)), rotation);
    game::world::camera::pitch_basis(identity, map->pitch, pitch);
    append_map_basis(matrix, rotation, zero);
    append_map_basis(matrix, pitch, zero);
    prepend_map_translation(matrix, map->translation[0], map->translation[1], map->translation[2]);
    const float scale = float(filtered_map_value(map->zoom_history) + map->zoom);
    game::world::actor_resources::append_actor_scale(matrix, scale, scale, 1, 1);
    graphics::gouraud::set_matrix(map->geometry, reinterpret_cast<const Dword *>(matrix));
    graphics::gouraud::draw(map->geometry);
    graphics::bindings::set_hud_matrix(reinterpret_cast<const Dword *>(matrix));
    draw_map_start_lines(map, engine);
    draw_map_vehicle_kind(map, engine, 6, &map->opponent_shape);
    draw_map_vehicle_kind(map, engine, 0, &map->opponent_shape);
    draw_map_vehicle_kind(map, engine, 3, &map->cop_shape);
    draw_map_vehicle_kind(map, engine, 5, &map->perpetrator_shape);
    draw_map_vehicle_kind(map, engine, 1, &map->car_shape);
    draw_map_vehicle_kind(map, engine, 2, &map->cop_shape);
    graphics::bindings::set_hud_matrix(game::runtime::skids::lifecycle::identity_matrix);
    invoke(engine, 65);
}
} // namespace nfs::application::frontend::hud
