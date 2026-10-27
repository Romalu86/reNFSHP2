#include "frontend_particle_widget.hpp"
#include "nfs/physics/frame/simulation_runtime.hpp"
#include "../../gui/lifecycle/gui_render_numeric_x87.hpp"
#include <cstring>

#pragma STDC FENV_ACCESS ON
namespace nfs::gui::engine {
const Rectangle *current_viewport_bounds(const Base *);
}
namespace nfs::gui::widgets::family {
extern const Destroy vector_methods[];
}
namespace nfs::application::frontend {
using game::runtime::invoke;
using gui::engine::Point;
using gui::engine::Rectangle;
using gui::retained::number;
using gui::retained::integer;

static int truncate_particle_coordinate(long double value) {
    const Dword bits = gui::retained::truncate(value);
    int result;
    std::memcpy(&result, &bits, sizeof(result));
    return result;
}

static Point *image_dimensions(const Rectangle *rectangle, Point *out) {
    const int height = rectangle->height, width = rectangle->width;
    out->methods = gui::widgets::family::vector_methods;
    out->x = width;
    out->y = height;
    return out;
}

void draw_menu_particle(MenuParticle *particle, void *render, const Rectangle *viewport,
                        gui::widgets::ImageResource *image, Dword blend, const Point *centre) {
    Point size;
    image_dimensions(&image->bounds, &size);
    const float sine = float(game::runtime::turn_sine(particle->angle));
    const float cosine = float(game::runtime::turn_cosine(particle->angle));
    const float half_width = float(integer(size.x) * number(particle->scale) * .5f);
    const float half_height = float(integer(size.y) * number(particle->scale) * .5f);
    const float offset_x = float(game::runtime::turn_cosine(particle->angle) * integer(centre->x));
    const auto rotated_y = game::runtime::turn_sine(particle->angle) * integer(centre->y);
    const int x = truncate_particle_coordinate(
        (rotated_y + number(offset_x)) * number(particle->scale) +
        integer(viewport->left) + number(particle->x));
    const float offset_y = float(game::runtime::turn_cosine(particle->angle) * integer(centre->y) *
                                 number(particle->scale) + integer(viewport->top) +
                                 number(particle->y));
    const auto rotated_x = game::runtime::turn_sine(particle->angle) * integer(centre->x) *
                           number(particle->scale);
    const int y = truncate_particle_coordinate(number(offset_y) - rotated_x);
    Dword colour = Dword(std::uint8_t(gui::retained::truncate(number(particle->alpha) * 255.f))) << 24;
    colour |= Dword(std::uint8_t(gui::retained::truncate(number(particle->red) * 255.f))) << 16;
    colour |= Dword(std::uint8_t(gui::retained::truncate(number(particle->green) * 255.f))) << 8;
    colour |= Dword(std::uint8_t(gui::retained::truncate(number(particle->blue) * 255.f)));
    const float width_cos = float(number(half_width) * number(cosine));
    const float width_sin = float(number(half_width) * number(sine));
    const float height_cos = float(number(half_height) * number(cosine));
    const float height_sin = float(number(half_height) * number(sine));
    const auto retained_y = integer(y);
    const float centre_y = float(retained_y);
    const auto top_retained = retained_y - number(height_cos);
    const float top = float(top_retained);
    const float first_y = float(top_retained + number(width_sin));
    const int first_point_y = gui::retained::round_f32(first_y);
    const auto retained_x = integer(x);
    const float centre_x = float(retained_x);
    const auto left_retained = retained_x - number(height_sin);
    const float left = float(left_retained);
    const float first_x = float(left_retained - number(width_cos));
    const int first_point_x = gui::retained::round_f32(first_x);
    Point points[4];
    points[0] = {gui::engine::point_methods, first_point_x, first_point_y};
    const int second_y = gui::retained::round_f32(float(number(top) - number(width_sin)));
    const int second_x = gui::retained::round_f32(float(number(left) + number(width_cos)));
    points[1] = {gui::engine::point_methods, second_x, second_y};
    const int third_y = gui::retained::round_f32(
        float(number(centre_y) - number(width_sin) + number(height_cos)));
    const int third_x = gui::retained::round_f32(
        float(number(centre_x) + number(height_sin) + number(width_cos)));
    points[2] = {gui::engine::point_methods, third_x, third_y};
    const int fourth_y = gui::retained::round_f32(
        float(number(centre_y) + number(height_cos) + number(width_sin)));
    const int fourth_x = gui::retained::round_f32(
        float(number(centre_x) - number(width_cos) + number(height_sin)));
    points[3] = {gui::engine::point_methods, fourth_x, fourth_y};
    try {
        invoke(render, 43, image->image, points, points + 1, points + 2, points + 3, &colour,
               blend);
    } catch (...) {
        for (int index = 3; index >= 0; --index)
            gui::engine::destroy_point(points + index);
        gui::engine::destroy_point(&size);
        throw;
    }
    for (int index = 3; index >= 0; --index)
        gui::engine::destroy_point(points + index);
    gui::engine::destroy_point(&size);
}

void __fastcall draw_particle_widget(ParticleWidget *widget, void *, gui::engine::Engine *render) {
    if (!widget->particles)
        return;
    const Rectangle *viewport = gui::engine::current_viewport_bounds(&render->base);
    Rectangle bounds{gui::engine::rectangle_methods, viewport->top, viewport->left, viewport->width,
                     viewport->height};
    try {
        for (int index = 0; index < widget->maximum_particles; ++index) {
            MenuParticle *particle = menu_particle_at(widget, index);
            if (!particle) {
                gui::engine::destroy_rectangle(&bounds);
                return;
            }
            if (particle->remaining != 0)
                draw_menu_particle(particle, render, &bounds, &widget->image, widget->blend,
                                   &widget->centre);
        }
    } catch (...) {
        gui::engine::destroy_rectangle(&bounds);
        throw;
    }
    gui::engine::destroy_rectangle(&bounds);
}
} // namespace nfs::application::frontend
