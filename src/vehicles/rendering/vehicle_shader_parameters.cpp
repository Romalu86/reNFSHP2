#include "vehicle_shader_parameters.hpp"
#include "nfs/graphics/resources/eagl_assets.hpp"
#include "nfs/core/services/service_lifecycle.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
namespace nfs::game::runtime::particles::pending {
kernel::Dword *find_fsh_image(kernel::Dword *, const char *);
}
namespace nfs::services {

void unregister_render_parameter(const char *name) {
    eagl::pool_remove(&eagl::external_symbols, name);
}
} // namespace nfs::services
namespace nfs::game::world::vehicle_render {

ShaderParameters shader_parameters{};
// Initialize the light basis during static initialization.
void initialize_vehicle_light_basis() {
    for (float &value : shader_parameters.light_block)
        value = 0;
    shader_parameters.light_block[0] = shader_parameters.light_block[4] =
        shader_parameters.light_block[8] = 1;
}
namespace {
Dword *create_texture(Dword *shape, const char *label) {
    auto *texture = static_cast<Dword *>(lifecycle::allocate(48, label, lifecycle::cars_pool));
    if (texture) {
        try {
            return graphics::tar::construct_shape(texture, shape);
        } catch (...) {
            runtime::track::pending::release_named(texture, label, lifecycle::cars_pool);
            throw;
        }
    }
    return texture;
}
void release_texture(Dword *&texture) {
    if (texture) {
        graphics::tar::destroy(texture);
        eagl::deallocator(texture, 48);
        texture = nullptr;
    }
}
} // namespace
} // namespace nfs::game::world::vehicle_render
namespace nfs::game::world::pending {

void initialize_vehicle_shader_parameters() {
    using namespace vehicle_render;
    auto &parameters = shader_parameters;
    if (!graphics::device || parameters.initialized)
        return;
    auto *vehicle_manager = subsystem(52);
    auto *images = at<Dword *>(vehicle_manager, 78600);
    parameters.environment_texture =
        create_texture(runtime::particles::pending::find_fsh_image(images, "refl"),
                       "GLOBAL STATIC MODEL ENVIRO TAR");
    // The first allocation/constructor precedes the saved manager reload.
    Dword *environment = at<Dword *>(subsystem(52), 78608);
    parameters.static_environment_texture = create_texture(
        reinterpret_cast<Dword *>(reinterpret_cast<Dword>(environment) + environment[5]),
        "STATIC ENV MAP TAR");
    parameters.damage_texture =
        create_texture(runtime::particles::pending::find_fsh_image(images, "swap"),
                       "GLOBAL STATIC MODEL DAMAGE TAR");
    services::register_render_parameter("gAmbientGround", parameters.ambient_ground);
    services::register_render_parameter("gAmbientSky", parameters.ambient_sky);
    services::register_render_parameter("gVehicleLightBlock", parameters.light_block);
    services::register_render_parameter("gVehicleStandardConstants", parameters.standard_constants);
    services::register_render_parameter("gVehiclePixelStandardConstants",
                                        parameters.pixel_standard_constants);
    services::register_render_parameter("gPaintSpecularColour", parameters.paint_specular_colour);
    services::register_render_parameter("gGlassSpecularColour", parameters.glass_specular_colour);
    services::register_render_parameter("gHotSpecularColour", parameters.hot_specular_colour);
    services::register_render_parameter("gRubberSpecularColour", parameters.rubber_specular_colour);
    services::register_render_parameter("gPaintLightConstants", parameters.paint_light_constants);
    services::register_render_parameter("gRubberLightConstants", parameters.rubber_light_constants);
    services::register_render_parameter("gGlassLightConstants", parameters.glass_light_constants);
    services::register_render_parameter("gGlossColourConstants", parameters.gloss_colour_constants);
    services::register_render_parameter("gSphereMapConstants", parameters.sphere_map_constants);
    services::register_render_parameter("gSphereMapScale", parameters.sphere_map_scale);
    services::register_render_parameter("gCameraPosition", parameters.camera_position);
    services::register_render_parameter("gSunPosition", parameters.sun_position);
    services::register_render_parameter("gVehiclePosition", parameters.vehicle_position);
    services::register_render_parameter("gTextureEnviro", parameters.environment_texture);
    services::register_render_parameter("gTextureDamage", parameters.damage_texture);
    services::register_render_parameter("gVehicleDamageScale", parameters.damage_scale);
    services::register_render_parameter("gVehicleDamageZoneAmounts",
                                        parameters.damage_zone_amounts);
    parameters.initialized = 1;
}
// The release list deliberately differs from the register list.
void release_vehicle_shader_parameters() {
    using namespace vehicle_render;
    if (shader_parameters.initialized) {
        for (const char *name : {"gAmbientGround",           "gAmbientSky",
                                 "gVehicleLightBlock",       "gVehicleStandardConstants",
                                 "gVehicleLightConstants",   "gSphereMapConstants",
                                 "gGlossColourConstants",    "gSphereMapScale",
                                 "gCameraPosition",          "gSunPosition",
                                 "gVehiclePosition",         "gPaintLightConstants",
                                 "gRubberSpecularColour",    "gRubberLightConstants",
                                 "gPaintSpecularColour",     "gGlassSpecularColour",
                                 "gGlassLightConstants",     "gTextureEnviro",
                                 "gHotSpecularColour",       "gVehiclePixelStandardConstants",
                                 "gTextureDamage",           "gVehicleDamageScale",
                                 "gVehicleDamageZoneAmounts"})
            services::unregister_render_parameter(name);
        release_texture(shader_parameters.damage_texture);
        release_texture(shader_parameters.environment_texture);
        release_texture(shader_parameters.static_environment_texture);
    }
    shader_parameters.initialized = 0;
}
} // namespace nfs::game::world::pending
#pragma section(".CRT$XCU_NFS_0069000C", read)
extern "C" __declspec(allocate(".CRT$XCU_NFS_0069000C")) void(__cdecl *const nfs_vehicle_light_startup)() =
    nfs::game::world::vehicle_render::initialize_vehicle_light_basis;
