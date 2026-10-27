#include "hud_map.hpp"
#include "../../network/session/network_pursuit_runtime.hpp"
#include "nfs/world/resources/runtime_resources.hpp"
#include <cstring>
namespace nfs::application::frontend::hud {
using game::runtime::at;
using game::runtime::invoke;
using game::runtime::system;
using graphics::bindings::HudVertex;
using kernel::numeric::Extended;
static void *perpetrator_interface(void *entity) {
    return game::runtime::ai::entities::interface_at(entity, 2);
}

gui::engine::images::TexturePoint *construct_map_coordinate(gui::engine::images::TexturePoint *point) {
    point->methods = gui::engine::pending::coordinate_methods;
    point->x = point->y = 0;
    return point;
}

void draw_map_start_line(Map *map, void *, const float *position, const float *rotation) {
    if (double(filtered_map_value(map->zoom_history) + map->zoom) == 0)
        return;
    const auto format = kernel::numeric::current_format();
    const auto E = [&](float f) { return Extended(f, format); };
    for (int row = 0; row < 2; ++row) {
        const float north = float(E(16) - Extended::integer(row, format) * 16.f);
        for (int column = 0; column < 8; ++column) {
            const Dword colour = ((column ^ row) & 1) ? 4294967295u : 4278190080u;
            const float west = float((Extended::integer(column, format) - 4.f) * 16.f);
            const float east = float(E(west) + 16.f), south = float(E(north) - 16.f);
            float corners[4][3] = {
                {west, 0, north}, {east, 0, north}, {east, 0, south}, {west, 0, south}};
            HudVertex vertices[4];
            for (int index = 0; index < 4; ++index) {
                float world[3], normalized[3];
                game::runtime::matrices::transform_positions(1, corners[index], rotation, position,
                                                             world, 12);
                map_position(map, normalized, world);
                vertices[index].x = normalized[0];
                vertices[index].y = normalized[1];
                vertices[index].z = normalized[2];
                vertices[index].colour = colour;
            }
            // The caller clears the texture before these position/colour quads.
            graphics::bindings::draw_hud_quad(vertices, vertices + 1, vertices + 2, vertices + 3);
        }
    }
}

Dword *map_vehicle_colour(Dword *colour, void *car) {
    void *entity = at<void *>(car, 7988);
    if (!entity) {
        *colour = 4294967295u;
        return colour;
    }
    const int blink = std::int32_t(graphics::unresolved::camera_state) % 30;
    if (void *cop = game::runtime::ai::police(entity)) {
        if (game::runtime::ai::cars::network_chase(cop))
            *colour = 4294901760u;
        else if (!Byte(game::runtime::ai::cars::cop_unavailable(cop)))
            *colour = 4278255360u;
        else
            *colour = blink < 15 ? 4294901760u : 0;
    } else if (void *pursuer = invoke<void *>(perpetrator_interface(entity), 4)) {
        *colour = game::runtime::ai::cars::network_chase(pursuer)
                      ? (blink < 15 ? 4278190335u : 4294901760u)
                      : 4278190239u;
    } else if (void *renderable = at<void *>(car, 3372))
        *colour = at<Dword>(renderable, 4056);
    else
        *colour = 4294967295u;
    return colour;
}

void draw_map_vehicle(Map *map, void *engine, void *car, Dword colour,
                      const gui::widgets::ImageResource *shape) {
    struct Coordinates {
        gui::engine::images::TexturePoint values[2];
        Coordinates() {
            construct_map_coordinate(values);
            construct_map_coordinate(values + 1);
        }
        ~Coordinates() {
            values[1].methods = gui::base_methods;
            values[0].methods = gui::base_methods;
        }
    } uv;
    const auto format = kernel::numeric::current_format();
    const auto E = [&](float value) { return Extended(value, format); };
    const float width = float(Extended::integer(map->base.base.bounds.width, format));
    if (!shape->image || width == 0 ||
        double(filtered_map_value(map->zoom_history) + map->zoom) == 0)
        return;
    const Dword texture = invoke<Dword>(engine, 56, Dword(7), shape->image, uv.values);
    if (!texture)
        return;
    graphics::bindings::bind_hud_texture(texture);
    void *body = at<void *>(car, 1768);
    const float position[3] = {at<float>(body, 680), at<float>(body, 684),
                               -at<float>(body, 688)};
    const auto scale = filtered_map_value(map->zoom_history) + map->zoom;
    const float radius =
        float((Extended::integer(shape->bounds.width, format) * .5f) / (scale * width));
    float centre[3];
    map_position(map, centre, position);
    const float angle = float(-filtered_map_value(map->heading_history) - .125f);
    const float sine =
        float(Extended::from_double(double(game::runtime::turn_sine(angle)), format) * radius);
    const auto cosine =
        Extended::from_double(double(game::runtime::turn_cosine(angle)), format) * radius;
    HudVertex vertices[4] = {{float(E(centre[0]) - sine), float(E(centre[1]) + cosine), centre[2],
                              colour, uv.values[0].x, uv.values[0].y},
                             {float(E(centre[0]) + cosine), float(E(centre[1]) + sine), centre[2],
                              colour, uv.values[0].x, uv.values[1].y},
                             {float(E(centre[0]) + sine), float(E(centre[1]) - cosine), centre[2],
                              colour, uv.values[1].x, uv.values[1].y},
                             {float(E(centre[0]) - cosine), float(E(centre[1]) - sine), centre[2],
                              colour, uv.values[1].x, uv.values[0].y}};
    graphics::bindings::draw_hud_quad(vertices, vertices + 1, vertices + 2, vertices + 3);
}

void draw_map_start_lines(Map *map, void *engine) {
    graphics::bindings::bind_hud_texture(0);
    for (int index = 0; index < at<int>(system(24), 80); ++index) {
        auto *marker = static_cast<float *>(game::runtime::level_marker(system(24), 5, index));
        if (marker) {
            float rotation[9];
            game::world::quaternion_matrix(marker + 3, rotation);
            const float position[3] = {marker[0], marker[1], -marker[2]};
            draw_map_start_line(map, engine, position, rotation);
        }
    }
}
// Consumers use AL only.
Byte map_cop_targets_player(void *cop) {
    if (!cop)
        return 0;
    for (int player = 0; player < 2; ++player) {
        void *car = game::runtime::physics::local_car(system(68), player);
        if (car)
            if (void *entity = at<void *>(car, 7988)) {
                void *owner = invoke<void *>(perpetrator_interface(entity), 3);
                void *chase = owner ? at<void *>(owner, 8) : nullptr;
                if (chase && chase == game::runtime::ai::cars::network_chase(cop))
                    return 1;
            }
    }
    return 0;
}

void draw_map_vehicle_kind(Map *map, void *engine, Dword kind,
                           const gui::widgets::ImageResource *shape) {
    namespace nw = network::world;
    nw::Iterator iterator;
    nw::construct_iterator(&iterator, game::runtime::ai::lifecycle::state<Dword *>(7353312));
    Dword value = nw::first(&iterator);
    while (!nw::iterator_done(&iterator)) {
        void *entry = reinterpret_cast<void *>(value);
        if (at<Byte>(entry, 212)) {
            void *entity = invoke<void *>(perpetrator_interface(entry), 1);
            if (entity && at<Byte>(entity, 212) && Dword(game::runtime::ai::kind(entity)) == kind) {
                if (void *car = at<void *>(entity, 368)) {
                    bool draw = true;
                    if (kind == 3)
                        draw = map_cop_targets_player(
                                   invoke<void *>(perpetrator_interface(entity), 4)) != 0;
                    if (draw) {
                        Dword colour;
                        map_vehicle_colour(&colour, car);
                        draw_map_vehicle(map, engine, car, colour, shape);
                    }
                }
            }
        }
        value = nw::next(&iterator);
    }
}
} // namespace nfs::application::frontend::hud
