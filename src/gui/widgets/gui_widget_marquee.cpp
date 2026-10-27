#include "gui_widget_controls.hpp"
#include "nfs/core/application/application.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
namespace nfs::gui::engine {
void inflate_rectangle(Rectangle *, int);
}
namespace nfs::gui::widgets::family {
using game::runtime::at;
using game::runtime::invoke;
Rectangle *absolute_widget_bounds(Widget *, Rectangle *);

int scale_gui_distance(int units) {
    return int(Dword(engine::source_bounds.width) * Dword(units)) / 1000;
}

Rectangle *marquee_bounds(Widget *widget, Rectangle *out) {
    const Point first = at<Point>(widget, 264), second = at<Point>(widget, 276);
    const int left = first.x < second.x ? first.x : second.x;
    const int top = first.y < second.y ? first.y : second.y;
    const int right = first.x > second.x ? first.x : second.x;
    const int bottom = first.y > second.y ? first.y : second.y;
    *out = {engine::rectangle_methods, top, left, int(Dword(right) - Dword(left)),
            int(Dword(bottom) - Dword(top))};
    return out;
}

Dword marquee_pointer_edges(Widget *widget) {
    const Point *position = pending::pointer_position();
    const int margin = scale_gui_distance(5);
    Rectangle expanded, contracted;
    marquee_bounds(widget, &expanded);
    marquee_bounds(widget, &contracted);
    engine::inflate_rectangle(&expanded, margin);
    engine::inflate_rectangle(&contracted, int(Dword(0) - Dword(margin)));
    Dword edges = 0;
    if (contains_point(&contracted, position)) {
        if (at<Dword>(widget, 292) & 1)
            edges = 1;
    } else if (contains_point(&expanded, position)) {
        if (!(at<Dword>(widget, 292) & 2))
            edges = 1;
        else {
            auto edge_is_near = [margin](int a, int b) {
                const Dword difference = Dword(a) - Dword(b);
                const Dword sign = Dword(int(difference) >> 31);
                return int((difference ^ sign) - sign) < margin;
            };
            if (edge_is_near(position->x, at<int>(widget, 280)))
                edges = 2;
            if (edge_is_near(position->x, at<int>(widget, 268)))
                edges |= 8;
            if (edge_is_near(position->y, at<int>(widget, 284)))
                edges |= 4;
            if (edge_is_near(position->y, at<int>(widget, 272)))
                edges |= 16;
        }
    }
    engine::destroy_rectangle(&contracted);
    engine::destroy_rectangle(&expanded);
    return edges;
}

Dword __fastcall marquee_drawing_state(Widget *widget, void *, Dword render_address) {
    auto *render = reinterpret_cast<engine::Engine *>(render_address);
    const Dword edges = at<Dword>(widget, 288);
    if (!(at<Dword>(widget, 292) & 3) || !edges)
        return apply_drawing_state(widget, nullptr, render);
    Dword state = invoke<Dword>(render, 52);
    if (edges == 1)
        state = 7;
    else if (edges & 8) {
        state = 11;
        if (edges & 16)
            state = 10;
        if (edges & 4)
            state = 9;
    } else if (edges & 2) {
        state = 11;
        if (edges & 16)
            state = 9;
        if (edges & 4)
            state = 10;
    } else
        state = 12;
    invoke(render, 50, state);
    return 1;
}

void __fastcall marquee_draw(Widget *widget, void *, Dword render_address) {
    auto *render = reinterpret_cast<engine::Engine *>(render_address);
    if (widget->flags & 1) {
        Rectangle bounds;
        marquee_bounds(widget, &bounds);
        invoke(render, 48, &bounds, &at<Dword>(widget, 300), Dword(0));
        engine::destroy_rectangle(&bounds);
        return;
    }
    Point top_left = at<Point>(widget, 264), bottom_right = at<Point>(widget, 276);
    top_left.methods = bottom_right.methods = engine::point_methods;
    Point top_right{engine::point_methods, bottom_right.x, top_left.y};
    Point bottom_left{engine::point_methods, top_left.x, bottom_right.y};
    auto edge = [&](Point *first, Point *second, Dword mask) {
        const Dword colour =
            at<Dword>(widget, 288) & mask ? at<Dword>(widget, 296) : widget->colour;
        invoke(render, 46, first, second, colour, colour);
    };
    edge(&top_left, &bottom_left, 8);
    edge(&top_left, &top_right, 16);
    edge(&top_right, &bottom_right, 2);
    edge(&bottom_right, &bottom_left, 4);
    engine::destroy_point(&bottom_left);
    engine::destroy_point(&bottom_right);
    engine::destroy_point(&top_right);
    engine::destroy_point(&top_left);
}

Dword __fastcall marquee_key_down(Widget *widget, void *, Dword key, Dword modifiers) {
    if ((at<Dword>(widget, 292) & 3) &&
        (key == 200 || key == 203 || key == 205 || key == 208)) {
        if (at<Dword>(widget, 288) == 1) {
            const Dword delta = key == 205 || key == 208 ? 1 : Dword(-1);
            const Dword offset = key == 203 || key == 205 ? 268 : 272;
            at<Dword>(widget, offset + 12) += delta;
            at<Dword>(widget, offset) += delta;
            events::post_widget(widget, 78, 0, 0);
        }
        return 1;
    }
    return pending::general_key(reinterpret_cast<GeneralWidget *>(widget), nullptr, key, modifiers);
}

Dword __fastcall marquee_key_up(Widget *widget, void *, Dword key, Dword modifiers) {
    if ((at<Dword>(widget, 292) & 3) && at<Dword>(widget, 288) &&
        (key == 200 || key == 203 || key == 205 || key == 208)) {
        release_capture(widget);
        events::post_widget(widget, 40, 0, 0);
        return 1;
    }
    return style_result_34(widget, nullptr, key, modifiers);
}

Dword __fastcall marquee_pointer_press(Widget *widget, void *, Dword button, Dword modifiers) {
    if (std::uint8_t(modifiers) || button)
        return pending::update_focus_style(widget, nullptr, button, modifiers);
    Rectangle bounds;
    marquee_bounds(widget, &bounds);
    Point position = *pending::pointer_position();
    position.methods = engine::point_methods;
    engine::inflate_rectangle(&bounds, scale_gui_distance(5));
    if (contains_point(&bounds, &position)) {
        at<Dword>(widget, 288) = marquee_pointer_edges(widget);
        capture_widget(widget);
    } else {
        Rectangle absolute;
        absolute_widget_bounds(widget, &absolute);
        const Dword inside = contains_point(&absolute, &position);
        engine::destroy_rectangle(&absolute);
        if (inside) {
            const Dword style = at<Dword>(widget, 292);
            if ((style & 1) && !(style & 2)) {
                const Dword dx = Dword(position.x) - at<Dword>(widget, 268);
                const Dword dy = Dword(position.y) - at<Dword>(widget, 272);
                at<Dword>(widget, 268) = Dword(position.x);
                at<Dword>(widget, 272) = Dword(position.y);
                at<Dword>(widget, 280) += dx;
                at<Dword>(widget, 284) += dy;
                at<Dword>(widget, 288) = 1;
                events::post_widget(widget, 78, 0, 0);
                capture_widget(widget);
                engine::destroy_point(&position);
                engine::destroy_rectangle(&bounds);
                return 1;
            }
            if (style & 2) {
                if (style & 1) {
                    at<Dword>(widget, 268) = Dword(position.x);
                    at<Dword>(widget, 272) = Dword(position.y);
                }
                at<Dword>(widget, 280) = Dword(position.x);
                at<Dword>(widget, 284) = Dword(position.y);
                at<Dword>(widget, 288) = 6;
                if (style & 1)
                    events::post_widget(widget, 78, 0, 0);
                events::post_widget(widget, 77, 0, 0);
            }
        }
        capture_widget(widget);
    }
    engine::destroy_point(&position);
    engine::destroy_rectangle(&bounds);
    return 1;
}

void __fastcall marquee_pointer_idle(Widget *widget, void *, Dword first, Dword modifiers) {
    if (!(modifiers & 29360128))
        at<Dword>(widget, 288) = marquee_pointer_edges(widget);
    style_result_38(widget, nullptr, first, modifiers);
}

Dword __fastcall marquee_pointer_move(Widget *widget, void *, Dword delta_address, Dword modifiers) {
    Point position = *pending::pointer_position();
    position.methods = engine::point_methods;
    Rectangle bounds;
    marquee_bounds(widget, &bounds);
    Dword result = 1;
    if (application::pending::captured_widget_id != widget->id) {
        if (!(modifiers & 29360128))
            at<Dword>(widget, 288) = marquee_pointer_edges(widget);
        else
            result = pending::update_active_style(widget, nullptr, delta_address, modifiers);
    } else {
        const Dword edges = at<Dword>(widget, 288);
        if (edges == 1) {
            const auto *delta = reinterpret_cast<const Point *>(delta_address);
            at<Dword>(widget, 268) += Dword(delta->x);
            at<Dword>(widget, 272) += Dword(delta->y);
            at<Dword>(widget, 280) += Dword(delta->x);
            at<Dword>(widget, 284) += Dword(delta->y);
            events::post_widget(widget, 78, 0, 0);
        } else {
            if (edges & 2)
                at<Dword>(widget, 280) = Dword(position.x);
            if (edges & 4)
                at<Dword>(widget, 284) = Dword(position.y);
            if (edges & 8)
                at<Dword>(widget, 268) = Dword(position.x);
            if (edges & 16)
                at<Dword>(widget, 272) = Dword(position.y);
            if (edges)
                events::post_widget(widget, 77, 0, 0);
        }
    }
    engine::destroy_rectangle(&bounds);
    engine::destroy_point(&position);
    return result;
}
} // namespace nfs::gui::widgets::family
