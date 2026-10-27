#include "frontend_scene_widgets.hpp"
#include "../../vehicles/rendering/vehicle_renderable_frame.hpp"
#include "../../gui/widgets/gui_widget_shared_methods.hpp"
#include "nfs/graphics/display/viewport_transform.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include <cstring>
namespace nfs::game::world::camera {
void apply_menu_camera(void *, Dword *, float, float);
}
namespace nfs::application::frontend {
using game::runtime::at;
using game::runtime::bytes;
using game::runtime::invoke;
using game::runtime::system;
using namespace game::world::vehicle_render;
using kernel::numeric::Extended;

CarPreview *construct_car_preview(CarPreview *widget, void *parent) {
    construct_scene_widget(&widget->base, parent);
    widget->actor = nullptr;
    try {
        gui::string_construct_copy(&widget->resource,
                                   &lifecycle::construction::unresolved::shared_string);
    } catch (...) {
        destroy_scene_widget(&widget->base);
        throw;
    }
    widget->base.base.base.methods = car_preview_methods.data();
    return widget;
}

void destroy_car_preview(CarPreview *widget) {
    widget->base.base.base.methods = car_preview_methods.data();
    gui::string_destruct(&widget->resource);
    destroy_scene_widget(&widget->base);
}

void clear_car_preview(CarPreview *widget) {
    if (widget->actor) {
        release_preview_renderable(system(52), widget->actor);
        invoke<void>(system(44), 4);
    }
    widget->actor = nullptr;
    gui::string_copy(&widget->resource,
                     &lifecycle::construction::unresolved::shared_string);
}

void __fastcall release_car_preview(CarPreview *widget, void *) {
    release_scene_widget(&widget->base, nullptr);
    clear_car_preview(widget);
}

void load_car_preview(CarPreview *widget, Dword variant) {
    assign_vector_value(&widget->base.initial_position, &widget->base.position.x);
    if (void *camera = invoke<void *>(game::tick_services[0], 2)) {
        std::memcpy(widget->base.camera_origin, bytes(camera) + 4, 12);
        assign_vector_value(&widget->base.camera_position,
                            reinterpret_cast<float *>(bytes(camera) + 4));
    }
    std::memcpy(widget->base.camera_matrix, graphics::viewports::matrix(graphics::main_viewport),
                64);
    if (widget->resource.length) {
        const Dword catalogue =
            catalogue_index(bytes(system(52)) + 4, gui::string_ascii(&widget->resource));
        invoke<void>(system(44), 3);
        widget->actor = reinterpret_cast<void *>(
            game::world::dependency::bind_vehicle_slot(system(52), catalogue, variant));
    }
}

void __fastcall initialize_car_preview(CarPreview *widget, void *) {
    initialize_scene_widget(&widget->base, nullptr);
    if (!widget->resource.length)
        return;
    void *lighting = system(56);
    std::memcpy(bytes(lighting) + 12, &widget->base.ambient_sky.x, 12);
    std::memcpy(bytes(lighting) + 24, &widget->base.ambient_ground.x, 12);
    set_frontend_light(lighting, 0, &widget->base.lights[0].colour.x, 0,
                       &widget->base.lights[0].direction.x);
    set_frontend_light(system(56), 1, &widget->base.lights[1].colour.x, 0,
                       &widget->base.lights[1].direction.x);
    load_car_preview(widget, 0);
}

void __fastcall tick_car_preview(CarPreview *widget, void *, Dword elapsed) {
    tick_scene_widget(&widget->base, nullptr, elapsed);
}

Dword __fastcall load_car_preview_configuration(CarPreview *widget, void *, void *configuration) {
    return load_scene_widget(&widget->base, nullptr, configuration);
}

Dword __fastcall save_car_preview_configuration(CarPreview *widget, void *, void *configuration) {
    return save_scene_widget(&widget->base, nullptr, configuration);
}
float preview_sun_position[3];

void light_car_preview(CarPreview *widget, std::uint8_t mirror) {
    float sky[3], ground[3], directions[3][3];
    std::memcpy(sky, &widget->base.ambient_sky.x, 12);
    std::memcpy(ground, &widget->base.ambient_ground.x, 12);
    std::memcpy(directions[1], &widget->base.lights[1].direction.x, 12);
    std::memcpy(directions[2], &widget->base.lights[2].direction.x, 12);
    preview_sun_position[0] = 40;
    preview_sun_position[1] = 100;
    preview_sun_position[2] = 20;
    directions[0][0] = -.5f;
    directions[0][1] = -1;
    directions[0][2] = -.5f;
    if (mirror) {
        std::memcpy(sky, &widget->base.ambient_ground.x, 12);
        std::memcpy(ground, &widget->base.ambient_sky.x, 12);
        preview_sun_position[1] = -100;
        directions[1][1] = -directions[1][1];
        directions[2][1] = -directions[2][1];
        directions[0][1] = 1;
        at<Dword>(widget->actor, 3056) = 1;
    } else
        at<Dword>(widget->actor, 3056) = 0;
    void *lighting = system(56);
    std::memcpy(bytes(lighting) + 36, preview_sun_position, 12);
    std::memcpy(bytes(lighting) + 12, sky, 12);
    std::memcpy(bytes(lighting) + 24, ground, 12);
    Dword intensity;
    std::memcpy(&intensity, &widget->base.lights[0].intensity, 4);
    set_frontend_light(lighting, 0, &widget->base.lights[0].colour.x, intensity, directions[0]);
    std::memcpy(&intensity, &widget->base.lights[1].intensity, 4);
    set_frontend_light(lighting, 1, &widget->base.lights[1].colour.x, intensity, directions[1]);
    std::memcpy(&intensity, &widget->base.lights[2].intensity, 4);
    set_frontend_light(lighting, 2, &widget->base.lights[2].colour.x, intensity, directions[2]);
    for (Dword offset : {3080u, 3084u, 3088u, 3092u})
        at<float>(widget->actor, offset) = 1;
    at<Dword>(widget->actor, 3112) = 0;
}

void zoom_car_preview(CarPreview *widget) {
    if (!widget->base.drag_flags[0])
        return;
    void *camera = invoke<void *>(game::tick_services[0], 2);
    if (!camera)
        return;
    const auto format = kernel::numeric::current_format();
    auto X = [&](float f) { return Extended(f, format); };
    widget->base.zoom_step = 15;
    assign_vector_value(&widget->base.initial_camera, reinterpret_cast<float *>(bytes(camera) + 4));
    float &z = widget->base.initial_camera.z;
    if (z < widget->base.camera_origin[2])
        z = float(X(z) + X(1) / X(widget->base.zoom_step));
    else if (widget->base.camera_origin[2] < z)
        z = float(X(z) - X(1) / X(widget->base.zoom_step));
    if (z < 1)
        z = 1;
    else if (z > 23)
        z = 23;
    std::memcpy(bytes(camera) + 4, &widget->base.initial_camera.x, 12);
    float translation[16];
    graphics::math::translation(translation, widget->base.initial_camera.x,
                                widget->base.initial_camera.y, z);
    graphics::viewports::set_matrix(graphics::main_viewport, translation);
}

void __fastcall draw_car_preview(CarPreview *widget, void *, gui::engine::Engine *engine) {
    draw_scene_widget(&widget->base, nullptr, engine);
    invoke<void>(engine, 56, Dword(6), Dword(0), Dword(0));
    gui::engine::Rectangle rectangle = *gui::engine::current_viewport_bounds(&engine->base), pixels;
    gui::engine::to_pixels(&pixels, &rectangle);
    rectangle.top = pixels.top;
    rectangle.left = pixels.left;
    rectangle.width = pixels.width;
    rectangle.height = pixels.height;
    gui::engine::destroy_rectangle(&pixels);
    float x = 0, y = 0, width = 0, height = 0;
    Dword minimum = 0, maximum = 0;
    graphics::viewports::read_rectangle(graphics::main_viewport, &x, &y, &width, &height, &minimum,
                                        &maximum);
    graphics::viewports::rectangle(graphics::main_viewport, float(rectangle.left),
                                   float(rectangle.top), float(rectangle.width),
                                   float(rectangle.height), 0, 1);
    if (widget->actor) {
        void *car = at<void *>(system(68), 20);
        zoom_car_preview(widget);
        bind_renderable_car(widget->actor, car);
        game::world::camera::apply_menu_camera(game::tick_services[0], graphics::main_viewport, 1,
                                               0);
        update_scene_camera(&widget->base);
        graphics::viewports::begin(graphics::main_viewport);
        graphics::viewports::clear(graphics::main_viewport, 2);
        if (widget->base.mirrored && at<Dword>(system(52), 81292) == 1) {
            float rotation[9], position[3], extents[3];
            std::memcpy(rotation, widget->base.rotation, 36);
            for (unsigned i : {1u, 4u, 7u})
                rotation[i] = -rotation[i];
            std::memcpy(position, &widget->base.initial_position.x, 12);
            renderable_half_extents(widget->actor, extents);
            position[1] =
                float(Extended(position[1], kernel::numeric::current_format()) -
                      (Extended(extents[1], kernel::numeric::current_format()) + extents[1]));
            light_car_preview(widget, 1);
            at<Dword>(system(52), 81308) = at<Dword>(system(52), 81312);
            prepare_renderable_frame(widget->actor, game::tick_services[0], position, rotation);
        }
        light_car_preview(widget, 0);
        prepare_renderable_frame(widget->actor, game::tick_services[0],
                                 &widget->base.initial_position.x, widget->base.rotation);
        draw_renderable_lights(widget->actor, game::tick_services[0]);
        graphics::viewports::end(graphics::main_viewport);
        graphics::viewports::set_matrix(graphics::main_viewport, widget->base.camera_matrix);
    }
    float near_value, far_value;
    std::memcpy(&near_value, &minimum, 4);
    std::memcpy(&far_value, &maximum, 4);
    graphics::viewports::rectangle(graphics::main_viewport, x, y, width, height, near_value,
                                   far_value);
    invoke<void>(engine, 56, Dword(5), Dword(0), Dword(0));
    gui::engine::destroy_rectangle(&rectangle);
}
} // namespace nfs::application::frontend
