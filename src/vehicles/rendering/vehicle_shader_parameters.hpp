#pragma once
#include "nfs/world/lifecycle/world_lifecycle.hpp"
namespace nfs::game::world::vehicle_render {
struct ShaderParameters {
    float damage_zone_amounts[64];
    float damage_scale[4];
    float light_block[28];
    float ambient_sky[4], ambient_ground[4];
    float hot_specular_colour[4], glass_specular_colour[4], rubber_specular_colour[4];
    float paint_specular_colour[4];
    float glass_light_constants[4], rubber_light_constants[4], paint_light_constants[4];
    float vehicle_position[4], camera_position[4], sun_position[4];
    float gloss_colour_constants[4], sphere_map_scale[4], sphere_map_constants[4];
    float pixel_standard_constants[4], standard_constants[4];
    Byte initialized;
    Byte reserved[3];
    Dword *environment_texture, *damage_texture, *static_environment_texture;
};
static_assert(sizeof(ShaderParameters) == 672 &&
              offsetof(ShaderParameters, light_block) == 272 &&
              offsetof(ShaderParameters, ambient_sky) == 384 &&
              offsetof(ShaderParameters, standard_constants) == 640 &&
              offsetof(ShaderParameters, environment_texture) == 660);
extern ShaderParameters shader_parameters;
} // namespace nfs::game::world::vehicle_render
