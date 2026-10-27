#include "gui_widget_sequence.hpp"
#include "../lifecycle/gui_images.hpp"
#include "nfs/physics/frame/simulation_runtime.hpp"
#include "../lifecycle/gui_render_numeric_x87.hpp"
#include <algorithm>
#include <cstring>

#pragma STDC FENV_ACCESS ON
namespace nfs::gui::engine {
const Rectangle *current_viewport_bounds(const Base *);
Point *top_left(Point *, const Rectangle *);
Point *top_right(Point *, const Rectangle *);
Point *bottom_left(Point *, const Rectangle *);
Point *bottom_right(Point *, const Rectangle *);
namespace pending {
extern const Destroy coordinate_methods[];
}
} // namespace nfs::gui::engine
namespace nfs::gui::widgets::family {
using game::runtime::at;
using game::runtime::invoke;
using gui::retained::integer;
using gui::retained::number;
using gui::retained::read_field;
using gui::retained::write_field;

static int signed_coordinate(Dword bits) {
    int result;
    std::memcpy(&result, &bits, sizeof(result));
    return result;
}

static void rotate_image_points(Point *first, Point *second, Point *third, Point *fourth,
                                float turns) {
    Point *points[] = {first, second, third, fourth};
    int original_x[4], original_y[4];
    for (unsigned index = 0; index != 4; ++index)
        original_x[index] = read_field(points[index]->x);
    for (unsigned index = 0; index != 4; ++index)
        original_y[index] = read_field(points[index]->y);
    int min_x = original_x[0], max_x = original_x[0];
    int min_y = original_y[0], max_y = original_y[0];
    for (unsigned index = 1; index != 4; ++index) {
        min_x = std::min(min_x, original_x[index]);
        max_x = std::max(max_x, original_x[index]);
        min_y = std::min(min_y, original_y[index]);
        max_y = std::max(max_y, original_y[index]);
    }
    const int centre_x = signed_coordinate(Dword(min_x) + Dword(max_x)) / 2;
    const int centre_y = signed_coordinate(Dword(min_y) + Dword(max_y)) / 2;
    for (unsigned index = 0; index != 4; ++index) {
        write_field(points[index]->x, signed_coordinate(Dword(original_x[index]) - Dword(centre_x)));
        write_field(points[index]->y, signed_coordinate(Dword(original_y[index]) - Dword(centre_y)));
    }
    const float cosine = float(game::runtime::turn_cosine(turns));
    const float sine = float(game::runtime::turn_sine(turns));
    const auto finish_x = [&](long double value) {
        return signed_coordinate(Dword(gui::retained::round_f32(float(value))) + Dword(centre_x));
    };
    const auto finish_y = [&](long double value) {
        return signed_coordinate(Dword(gui::retained::round_f32(float(value))) + Dword(centre_y));
    };
    const auto first_y_sine = integer(read_field(first->y)) * sine;
    const auto first_x_cosine = integer(read_field(first->x)) * cosine;
    const int first_x = finish_x(first_y_sine + first_x_cosine);
    const auto first_y_cosine = integer(read_field(first->y)) * cosine;
    const auto first_x_sine = integer(read_field(first->x)) * sine;
    const int first_y = finish_y(first_y_cosine - first_x_sine);

    // The next X inputs precede the previous point's stores; Y is reloaded after them.
    const auto second_y_sine = integer(read_field(second->y)) * sine;
    const auto second_x_input = integer(read_field(second->x));
    write_field(first->x, first_x);
    write_field(first->y, first_y);
    const auto second_x_cosine = second_x_input * cosine;
    const int second_x = finish_x(second_y_sine + second_x_cosine);
    const auto second_y_cosine = integer(read_field(second->y)) * cosine;
    const auto second_x_sine = integer(read_field(second->x)) * sine;
    const int second_y = finish_y(second_y_cosine - second_x_sine);

    const auto third_x_cosine = integer(read_field(third->x)) * cosine;
    write_field(second->x, second_x);
    const auto third_y_input = integer(read_field(third->y));
    write_field(second->y, second_y);
    const auto third_y_sine = third_y_input * sine;
    const int third_x = finish_x(third_x_cosine + third_y_sine);
    const auto third_y_cosine = integer(read_field(third->y)) * cosine;
    const auto third_x_sine = integer(read_field(third->x)) * sine;
    const int third_y = finish_y(third_y_cosine - third_x_sine);

    const auto fourth_x_cosine = integer(read_field(fourth->x)) * cosine;
    const auto fourth_y_input = integer(read_field(fourth->y));
    write_field(third->x, third_x);
    write_field(third->y, third_y);
    const auto fourth_y_sine = fourth_y_input * sine;
    const int fourth_x = finish_x(fourth_x_cosine + fourth_y_sine);
    const auto fourth_y_cosine = integer(read_field(fourth->y)) * cosine;
    const auto fourth_x_sine = integer(read_field(fourth->x)) * sine;
    const int fourth_y = finish_y(fourth_y_cosine - fourth_x_sine);
    write_field(fourth->x, fourth_x);
    write_field(fourth->y, fourth_y);
}

void __fastcall static_image_draw(Widget *widget, void *, Dword render_address) {
    const Dword handle = invoke<ImageResource *>(widget, 47)->image;
    if (!handle)
        return;
    auto *render = reinterpret_cast<engine::Engine *>(render_address);
    const Rectangle *current = engine::current_viewport_bounds(&render->base);
    Rectangle bounds{engine::rectangle_methods, current->top, current->left, current->width,
                     current->height};
    Dword style = at<Dword>(widget, 268);
    const Dword blend = style & 256 ? 1 : style & 512 ? 2 : style & 1024 ? 3 : 0;
    if (style & 2048)
        bounds.width = invoke<ImageResource *>(widget, 47)->bounds.width;
    if (at<Dword>(widget, 268) & 4096)
        bounds.height = invoke<ImageResource *>(widget, 47)->bounds.height;
    style = at<Dword>(widget, 268);
    try {
        if (style & 8192) {
            const Rectangle source = invoke<ImageResource *>(widget, 47)->bounds;
            if (source.width && source.height) {
                engine::images::TexturePoint first{engine::pending::coordinate_methods, 0, 0};
                engine::images::TexturePoint last{engine::pending::coordinate_methods,
                                                  float(integer(bounds.width) *
                                                        number(at<float>(widget, 316)) /
                                                        integer(source.width)),
                                                  float(integer(bounds.height) *
                                                        number(at<float>(widget, 320)) /
                                                        integer(source.height))};
                invoke(render, 44, handle, &bounds, &widget->colour, &first, &last, blend);
            }
        } else if (at<float>(widget, 312) == 0 && !(style & 248)) {
            invoke(render, 45, handle, &bounds, &widget->colour, blend);
        } else {
            Point points[4];
            engine::top_left(points, &bounds);
            engine::top_right(points + 1, &bounds);
            engine::bottom_right(points + 2, &bounds);
            engine::bottom_left(points + 3, &bounds);
            const auto swap_coordinates = [](Point &a, Point &b) {
                std::swap(a.x, b.x);
                std::swap(a.y, b.y);
            };
            if (at<Dword>(widget, 268) & 8) {
                swap_coordinates(points[0], points[3]);
                swap_coordinates(points[1], points[2]);
            }
            if (at<Dword>(widget, 268) & 16) {
                swap_coordinates(points[0], points[1]);
                swap_coordinates(points[3], points[2]);
            }
            if (at<float>(widget, 312) != 0) {
                const float turns = float(number(at<float>(widget, 312)) * -0.00277777784503996372222900390625f);
                rotate_image_points(points, points + 1, points + 2, points + 3, turns);
            }
            style = at<Dword>(widget, 268);
            const unsigned first = style & 32 ? 1 : style & 64 ? 2 : style & 128 ? 3 : 0;
            try {
                invoke(render, 43, handle, points + first, points + ((first + 1) & 3),
                       points + ((first + 2) & 3), points + ((first + 3) & 3), &widget->colour,
                       blend);
            } catch (...) {
                for (int index = 3; index >= 0; --index)
                    engine::destroy_point(points + index);
                throw;
            }
            for (int index = 3; index >= 0; --index)
                engine::destroy_point(points + index);
        }
    } catch (...) {
        engine::destroy_rectangle(&bounds);
        throw;
    }
    engine::destroy_rectangle(&bounds);
}
} // namespace nfs::gui::widgets::family
