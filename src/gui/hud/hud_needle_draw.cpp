#include "hud_instruments.hpp"
#include <cmath>
namespace nfs::application::frontend::hud {
using namespace gui::widgets;
using game::runtime::invoke;
using kernel::numeric::Extended;

void __fastcall draw_needle(Needle *needle, void *, void *render) {
    const auto format = kernel::numeric::current_format();
    auto E = [&](float f) { return Extended(f, format); };
    auto *widget = &needle->base.base;
    const auto *viewport = gui::engine::current_viewport_bounds(static_cast<gui::engine::Base *>(render));
    gui::engine::Point origin{gui::engine::point_methods, viewport->left, viewport->top};
    gui::engine::Rectangle clip{gui::engine::rectangle_methods,
        int(Dword(needle->subclip.top) + Dword(origin.y)), int(Dword(needle->subclip.left) + Dword(origin.x)),
        needle->subclip.width, needle->subclip.height};
    if (needle->style & 4) invoke<void>(render, 64, &clip);
    gui::engine::Rectangle client;
    client_bounds(widget, &client);
    gui::engine::Rectangle bounds = needle->image.bounds;
    const Dword colour = widget->colour;
    if (needle->style & 1) {
        bounds.top = 0;
        const float displacement = float(Extended::integer(int(Dword(client.width) - Dword(bounds.width)), format) * needle->value);
        bounds.left = game::runtime::rounded_cell(displacement);
        bounds.left = int(Dword(bounds.left) + Dword(origin.x));
        bounds.top = int(Dword(bounds.top) + Dword(origin.y));
        if (!needle->image.image) {
            gui::engine::Point first{gui::engine::point_methods, bounds.left, bounds.top};
            gui::engine::Point last{gui::engine::point_methods, bounds.left, int(Dword(bounds.top) + Dword(client.height))};
            invoke<void>(render, 46, &first, &last, colour, colour);
            first.x = int(Dword(first.x) + 1); last.x = int(Dword(last.x) + 1);
            const Dword second_colour = widget->colour;
            invoke<void>(render, 46, &first, &last, second_colour, second_colour);
        } else invoke<void>(render, 45, needle->image.image, &bounds, &widget->colour, Dword(0));
    } else {
        const int radial_offset = (widget->bounds.width >> 1) - (bounds.height >> 1);
        int offset = radial_offset;
        const float angle = float((E(needle->end_angle) - needle->start_angle) * needle->value + needle->start_angle);
        const float cosine = float(game::runtime::turn_cosine(angle));
        const auto sine = Extended::from_double(double(game::runtime::turn_sine(angle)), format);
        if ((needle->style & 2) && cosine != 0) {
            const float doubled = float(E(cosine) + cosine);
            const float projected = float(Extended::integer(client.height, format) / E(doubled));
            offset = int(Dword(game::runtime::rounded_cell(projected)) - Dword(bounds.height / 2));
        }
        const auto distance = Extended::integer(offset, format);
        const float stored_distance = float(distance);
        const float horizontal = float(distance * float(sine));
        const float vertical = float(E(stored_distance) * cosine);
        const int y = int(Dword(origin.y) + Dword(client.height / 2) - Dword(game::runtime::rounded_cell(vertical)) - Dword(bounds.height / 2));
        const int x = int(Dword(origin.x) + Dword(client.width / 2) - Dword(game::runtime::rounded_cell(horizontal)) - Dword(bounds.width / 2));
        gui::engine::Point centre{gui::engine::point_methods, x, y};
        if (!needle->image.image) {
            gui::engine::Point edge{gui::engine::point_methods,
                int(Dword(origin.x) + Dword(client.width / 2)), int(Dword(origin.y) + Dword(client.height / 2))};
            invoke<void>(render, 46, &centre, &edge, colour, colour);
            const Dword shadow = 2130706432;
            gui::engine::Point shift{gui::engine::point_methods, 1, 0};
            gui::engine::Point second{gui::engine::point_methods,
                int(Dword(edge.x) + Dword(shift.x)), int(Dword(edge.y) + Dword(shift.y))};
            gui::engine::Point first{gui::engine::point_methods,
                int(Dword(centre.x) + Dword(shift.x)), int(Dword(centre.y) + Dword(shift.y))};
            invoke<void>(render, 46, &first, &second, shadow, shadow);
        } else {
            gui::engine::Point points[4];
            for (auto &point : points) gui::engine::construct_point(&point);
            rotate_needle_corners(points, points + 1, points + 2, points + 3, angle, bounds.width, bounds.height);
            for (auto &point : points) {
                point.x = int(Dword(point.x) + Dword(centre.x));
                point.y = int(Dword(point.y) + Dword(centre.y));
            }
            invoke<void>(render, 43, needle->image.image, points, points + 1, points + 2, points + 3, &widget->colour, Dword(0));
            for (int index = 3; index >= 0; --index) gui::engine::destroy_point(points + index);
        }
    }
    if (needle->style & 4) invoke<void>(render, 65);
}
}
