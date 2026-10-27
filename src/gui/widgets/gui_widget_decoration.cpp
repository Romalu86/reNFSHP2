#include "gui_widget_controls.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
namespace nfs::gui::engine {
const Rectangle *current_viewport_bounds(const Base *);

void inflate_rectangle(Rectangle *bounds, int amount) {
    const int horizontal_limit = int(Dword(0) - Dword(bounds->width >> 1));
    const int vertical_limit = int(Dword(0) - Dword(bounds->height >> 1));
    if (amount < horizontal_limit)
        amount = horizontal_limit;
    if (amount < vertical_limit)
        amount = vertical_limit;
    bounds->left = int(Dword(bounds->left) - Dword(amount));
    bounds->top = int(Dword(bounds->top) - Dword(amount));
    bounds->width = int(Dword(bounds->width) + Dword(amount) * 2);
    bounds->height = int(Dword(bounds->height) + Dword(amount) * 2);
}
} // namespace nfs::gui::engine
namespace nfs::gui::widgets::family {
using game::runtime::at;
using game::runtime::invoke;

void draw_image_box_shadow(Widget *widget, const Rectangle *bounds, engine::Engine *render) {
    if (!(at<Dword>(widget, 268) & 2))
        return;
    Rectangle shadow{engine::rectangle_methods, int(Dword(bounds->top) + 2),
                     int(Dword(bounds->left) + 2), bounds->width, bounds->height};
    Dword colour;
    invoke(render, 23, &colour, Dword(8));
    invoke(render, 48, &shadow, &colour, Dword((at<Dword>(widget, 268) & 9) != 0));
    engine::destroy_rectangle(&shadow);
}

void draw_image_box_border(Widget *widget, const Rectangle *bounds, engine::Engine *render) {
    const int width = bounds->width, height = bounds->height;
    if (!width || !height)
        return;
    const Dword horizontal_inset = invoke<Dword>(render, 49, Dword(1));
    const Dword vertical_inset = invoke<Dword>(render, 49, Dword(0));
    Rectangle border{engine::rectangle_methods, bounds->top, bounds->left,
                     int(Dword(width) - horizontal_inset), int(Dword(height) - vertical_inset)};
    const Dword style = at<Dword>(widget, 268);
    if (style & 128) {
        Dword colour;
        invoke(render, 23, &colour, Dword(5));
        invoke(render, 48, &border, &colour, Dword(0));
        engine::inflate_rectangle(&border, 1);
        invoke(render, 48, &border, &colour, Dword(0));
    } else if (style & 76) {
        Point top_left{engine::point_methods, border.left, border.top};
        Point top_right{engine::point_methods, int(Dword(border.left) + Dword(border.width)),
                        border.top};
        Point bottom_right{engine::point_methods, top_right.x,
                           int(Dword(border.top) + Dword(border.height))};
        Point bottom_left{engine::point_methods, border.left, bottom_right.y};
        Dword dark, light;
        if (style & 64) {
            invoke(render, 23, &dark, Dword(5));
            light = dark;
        } else {
            invoke(render, 23, &dark, Dword(7));
            invoke(render, 23, &light, Dword(6));
            if (!(style & 8)) {
                const Dword temporary = dark;
                dark = light;
                light = temporary;
            }
        }
        invoke(render, 46, &top_right, &bottom_right, dark, dark);
        invoke(render, 46, &top_left, &top_right, light, light);
        invoke(render, 46, &bottom_left, &top_left, light, light);
        invoke(render, 46, &bottom_right, &bottom_left, dark, dark);
        engine::destroy_point(&bottom_left);
        engine::destroy_point(&bottom_right);
        engine::destroy_point(&top_right);
        engine::destroy_point(&top_left);
    }
    if (style & 32) {
        Point top_left{engine::point_methods, border.left, border.top};
        Point top_right{engine::point_methods, int(Dword(border.left) + Dword(border.width)),
                        border.top};
        Point bottom_right{engine::point_methods, top_right.x,
                           int(Dword(border.top) + Dword(border.height))};
        Point bottom_left{engine::point_methods, border.left, bottom_right.y};
        Dword colour;
        invoke(render, 23, &colour, Dword(5));
        invoke(render, 46, &top_left, &bottom_right, colour, colour);
        invoke(render, 46, &top_right, &bottom_left, colour, colour);
        engine::destroy_point(&bottom_left);
        engine::destroy_point(&bottom_right);
        engine::destroy_point(&top_right);
        engine::destroy_point(&top_left);
    }
    engine::destroy_rectangle(&border);
}

void __fastcall image_box_draw(Widget *widget, void *, Dword render_address) {
    auto *render = reinterpret_cast<engine::Engine *>(render_address);
    const auto *bounds = engine::current_viewport_bounds(&render->base);
    invoke(render, 21, inherited_palette(widget));
    draw_image_box_shadow(widget, bounds, render);
    const Dword style = at<Dword>(widget, 268);
    const Dword fill = (style & 13) != 0;
    if (style & 16)
        invoke(render, 47, bounds, &at<Dword>(widget, 272), &at<Dword>(widget, 276),
               &at<Dword>(widget, 284), &at<Dword>(widget, 280), fill);
    else
        invoke(render, 48, bounds, &widget->colour, fill);
    draw_image_box_border(widget, bounds, render);
}

void __fastcall image_bar_initialize(Widget *widget, void *) {
    at<Dword>(widget, 272) = named_child_id(widget, "<HEAD>");
    at<Dword>(widget, 276) = named_child_id(widget, "<MIDDLE>");
    at<Dword>(widget, 280) = named_child_id(widget, "<TAIL>");
    pending::request_general_layout(reinterpret_cast<GeneralWidget *>(widget), nullptr);
}

void __fastcall image_bar_prepare_draw(Widget *widget, void *, Dword render_address) {
    prepare_draw(widget, nullptr, reinterpret_cast<engine::Engine *>(render_address));
    Rectangle bounds;
    client_bounds(widget, &bounds);
    Widget *start = find_child(widget, at<Dword>(widget, 272));
    Widget *middle = find_child(widget, at<Dword>(widget, 276));
    Widget *end = find_child(widget, at<Dword>(widget, 280));
    Rectangle first{engine::rectangle_methods, 0, 0, 0, 0};
    Rectangle second = first, third = first;
    if (start)
        first = start->bounds;
    if (middle)
        second = middle->bounds;
    if (end)
        third = end->bounds;
    if (!(at<Dword>(widget, 268) & 1)) {
        first.left = bounds.left;
        third.left = int(Dword(bounds.left) + Dword(bounds.width) - Dword(third.width));
        second.left = int(Dword(bounds.left) + Dword(first.width));
        second.width = int(Dword(third.left) - Dword(first.width) - Dword(bounds.left));
    } else {
        first.top = bounds.top;
        third.top = int(Dword(bounds.top) + Dword(bounds.height) - Dword(third.height));
        second.top = int(Dword(bounds.top) + Dword(first.height));
        second.height = int(Dword(third.top) - Dword(first.height) - Dword(bounds.top));
    }
    if (start) {
        start->bounds.left = first.left;
        start->bounds.top = first.top;
        start->bounds.width = first.width;
        start->bounds.height = first.height;
    }
    if (middle) {
        middle->bounds.left = second.left;
        middle->bounds.top = second.top;
        middle->bounds.width = second.width;
        middle->bounds.height = second.height;
    }
    if (end) {
        end->bounds.left = third.left;
        end->bounds.top = third.top;
        end->bounds.width = third.width;
        end->bounds.height = third.height;
    }
    engine::destroy_rectangle(&bounds);
}
} // namespace nfs::gui::widgets::family
