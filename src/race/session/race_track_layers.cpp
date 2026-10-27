#include "race_world_draw.hpp"
#include "../../graphics/actors/model_render_runtime.hpp"
#include <cstring>
namespace nfs::game::world::track_render {

void clear_layer_flags(void *compartment) {
    for (unsigned index = 0; index < 12; ++index)
        if (void *layer = at<void *>(compartment, 84 + index * 4))
            at<std::uint16_t>(layer, 2) = 0;
}

void read_layer_sphere(void *layer, float *centre, float *radius) {
    const auto *sphere = offset(layer, 32 + at<std::uint16_t>(layer, 0) * 4);
    std::memcpy(centre, sphere, 12);
    std::memcpy(radius, sphere + 12, 4);
}
static bool visible_layer(void *layer) {
    if (at<int>(subsystem(36), 12) <= 1)
        return true;
    float centre[3], radius;
    read_layer_sphere(layer, centre, &radius);
    return graphics::model_render::sphere_visible(graphics::main_viewport, centre, radius) != 0;
}
static void draw_compartment_model(void *compartment) {
    void *model = at<void *>(compartment, 80);
    vehicle_render::pending::draw_model(model, reinterpret_cast<Dword *>(offset(model, 12)));
}

void draw_opaque_layers(void *compartment, Byte quality) {
    clear_layer_flags(compartment);
    void *layer = at<void *>(compartment, 108);
    if (layer && visible_layer(layer) && quality) {
        at<std::uint16_t>(layer, 2) = 1;
        draw_compartment_model(compartment);
        at<std::uint16_t>(layer, 2) = 0;
    }
    layer = at<void *>(compartment, 84);
    if (layer && visible_layer(layer)) {
        at<std::uint16_t>(layer, 2) = 1;
        draw_compartment_model(compartment);
        at<std::uint16_t>(layer, 2) = 0;
    }
}

void draw_surface_layers(void *compartment, Byte quality) {
    clear_layer_flags(compartment);
    for (unsigned index = 1; index < 12; ++index) {
        if (index == 6)
            continue;
        void *layer = at<void *>(compartment, 84 + index * 4);
        if (layer && visible_layer(layer) && (index < 6 || quality))
            at<std::uint16_t>(layer, 2) = 1;
    }
    draw_compartment_model(compartment);
}

void draw_reflection_layers(void *compartment) {
    clear_layer_flags(compartment);
    for (unsigned index = 0; index < 6; ++index) {
        void *layer = at<void *>(compartment, 84 + index * 4);
        if (layer && visible_layer(layer))
            at<std::uint16_t>(layer, 2) = 1;
    }
    draw_compartment_model(compartment);
}

void draw_layer_glows(void *compartment) {
    const float identity[9] = {1, 0, 0, 0, 1, 0, 0, 0, 1};
    if (void *glows = at<void *>(compartment, 164))
        draw_compartment_glows(glows, identity);
}
} // namespace nfs::game::world::track_render
