#include "gui_widget_interaction.hpp"
#include "nfs/core/application/application.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include "../../math/precision/extended_float.hpp"
#include <cmath>
namespace nfs::gui::widgets::family {
using game::runtime::at;
using game::runtime::invoke;
namespace {
Rectangle border_bounds(Widget *widget, Dword id) {
    Rectangle bounds{engine::rectangle_methods, 0, 0, 0, 0};
    if (Widget *child = find_child(widget, id))
        engine::copy_rectangle_fields(&bounds, &child->bounds);
    return bounds;
}
void store_border_bounds(Widget *widget, Dword id, const Rectangle *bounds) {
    if (Widget *child = find_child(widget, id))
        engine::copy_rectangle_fields(&child->bounds, bounds);
}
} // namespace

void __fastcall frame_initialize(Widget *widget, void *) {
    auto *frame = reinterpret_cast<Frame *>(widget);
    if (!(frame->style & 16)) {
        frame->top = named_child_id(widget, "<T>");
        frame->bottom = named_child_id(widget, "<B>");
        frame->left = named_child_id(widget, "<L>");
        frame->right = named_child_id(widget, "<R>");
        frame->top_left = named_child_id(widget, "<TL>");
        frame->top_right = named_child_id(widget, "<TR>");
        frame->bottom_left = named_child_id(widget, "<BL>");
        frame->bottom_right = named_child_id(widget, "<BR>");
    } else {
        using kernel::numeric::Extended;
        const auto format = kernel::numeric::current_format();
        const int width =
            int(std::lrintf(float(Extended(engine::inverse_horizontal_scale, format) * 6.0f)));
        const int height =
            int(std::lrintf(float(Extended(engine::inverse_vertical_scale, format) * 6.0f)));
        Dword *ids[] = {&frame->left,        &frame->right,       &frame->top,
                       &frame->bottom,      &frame->top_left,    &frame->top_right,
                       &frame->bottom_left, &frame->bottom_right};
        for (Dword *id : ids) {
            auto *child = static_cast<Widget *>(create_image_box(widget));
            at<Dword>(child, 268) = 8;
            const Rectangle bounds{engine::rectangle_methods, 0, 0, width, height};
            engine::copy_rectangle_fields(&child->bounds, &bounds);
            child->colour = widget->colour;
            initialize_tree(child);
            *id = child->id;
        }
    }
    pending::request_general_layout(reinterpret_cast<GeneralWidget *>(widget), nullptr);
}

Dword frame_pointer_edges(Widget *widget) {
    auto *frame = reinterpret_cast<Frame *>(widget);
    if (!(frame->style & 15))
        return 0;
    Point local;
    from_screen_point(widget, &local, pending::pointer_position());
    const Rectangle top = border_bounds(widget, frame->top),
                    left = border_bounds(widget, frame->left),
                    right = border_bounds(widget, frame->right),
                    bottom = border_bounds(widget, frame->bottom),
                    top_left = border_bounds(widget, frame->top_left),
                    top_right = border_bounds(widget, frame->top_right),
                    bottom_left = border_bounds(widget, frame->bottom_left),
                    bottom_right = border_bounds(widget, frame->bottom_right);
    const Dword style = frame->style;
    Dword edges = 0;
    Rectangle combined, temporary;
    if (style & 3) {
        engine::union_copy(&temporary, &left, &top_left);
        engine::union_copy(&combined, &temporary, &bottom_left);
        if (contains_point(&combined, &local) && (style & 1))
            edges = 1;
        else {
            engine::union_copy(&temporary, &right, &bottom_right);
            engine::union_copy(&combined, &temporary, &top_right);
            if (contains_point(&combined, &local) && (style & 2))
                edges = 2;
        }
    }
    if (style & 12) {
        engine::union_copy(&temporary, &top, &top_left);
        engine::union_copy(&combined, &temporary, &top_right);
        if (contains_point(&combined, &local) && (style & 4))
            edges |= 4;
        else {
            engine::union_copy(&temporary, &bottom, &bottom_left);
            engine::union_copy(&combined, &temporary, &bottom_right);
            if (contains_point(&combined, &local) && (style & 8))
                edges |= 8;
        }
    }
    engine::destroy_point(&local);
    return edges;
}

Dword __fastcall frame_drawing_state(Widget *widget, void *, Dword render_address) {
    auto *frame = reinterpret_cast<Frame *>(widget);
    auto *render = reinterpret_cast<engine::Engine *>(render_address);
    if (application::pending::captured_widget_id != widget->id)
        frame->resize_edges = frame_pointer_edges(widget);
    const Dword edges = frame->resize_edges;
    if (!edges)
        return apply_drawing_state(widget, nullptr, render);
    Dword state = invoke<Dword>(render, 52);
    if (edges & 1) {
        state = 11;
        if (edges & 4)
            state = 10;
        if (edges & 8)
            state = 9;
    } else if (edges & 2) {
        state = 11;
        if (edges & 4)
            state = 9;
        if (edges & 8)
            state = 10;
    } else if (edges & 12)
        state = 12;
    invoke(render, 50, state);
    return 1;
}

Dword __fastcall frame_pointer_move(Widget *widget, void *, Dword first, Dword modifiers) {
    auto *frame = reinterpret_cast<Frame *>(widget);
    Widget *parent = widget->parent;
    if (application::pending::captured_widget_id != widget->id || !parent) {
        if (frame->resize_edges)
            return 1;
        return pending::update_active_style(widget, nullptr, first, modifiers);
    }
    if (!frame->resize_edges)
        return pending::update_active_style(widget, nullptr, first, modifiers);
    Point local;
    from_screen_point(parent, &local, pending::pointer_position());
    Rectangle client;
    client_bounds(parent, &client);
    Rectangle resized = parent->bounds;
    int dx = 0, dy = 0;
    if (frame->resize_edges & 4) {
        const int lower = int(Dword(client.height) - Dword(frame->minimum_height) + Dword(client.top));
        if (lower <= local.y)
            local.y = lower;
        if (frame->maximum_height) {
            const int upper =
                int(Dword(client.height) - Dword(frame->maximum_height) + Dword(client.top));
            if (local.y <= upper)
                local.y = upper;
        }
        resized.top = int(Dword(resized.top) + Dword(local.y) - Dword(client.top));
        dy = int(Dword(0) - (Dword(local.y) - Dword(client.top)));
        resized.height = int(Dword(resized.height) + Dword(dy));
    } else if (frame->resize_edges & 8) {
        const int minimum = int(Dword(frame->minimum_height) + Dword(client.top));
        if (local.y <= minimum)
            local.y = minimum;
        if (frame->maximum_height) {
            const int maximum = int(Dword(frame->maximum_height) + Dword(client.top));
            if (maximum <= local.y)
                local.y = maximum;
        }
        dy = int(Dword(local.y) - Dword(client.height) - Dword(client.top));
        resized.height = int(Dword(resized.height) + Dword(dy));
    }
    if (frame->resize_edges & 1) {
        const int maximum =
            int(Dword(client.width) - Dword(frame->minimum_width) + Dword(client.left));
        if (maximum <= local.x)
            local.x = maximum;
        if (frame->maximum_width) {
            // Compare the vertical coordinate here.
            const int edge =
                int(Dword(client.width) - Dword(frame->maximum_width) + Dword(client.left));
            local.x = local.y > edge ? local.y : edge;
        }
        if (local.x < int(Dword(client.width) + Dword(client.left))) {
            resized.left = int(Dword(resized.left) + Dword(local.x) - Dword(client.left));
            dx = int(Dword(0) - (Dword(local.x) - Dword(client.left)));
            resized.width = int(Dword(resized.width) + Dword(dx));
        }
    } else if (frame->resize_edges & 2) {
        const int minimum = int(Dword(frame->minimum_width) + Dword(client.left));
        if (local.x <= minimum)
            local.x = minimum;
        if (frame->maximum_width) {
            const int edge = int(Dword(frame->maximum_width) + Dword(client.left));
            local.x = local.y > edge ? local.y : edge;
        }
        dx = int(Dword(local.x) - Dword(client.width) - Dword(client.left));
        resized.width = int(Dword(resized.width) + Dword(dx));
    }
    if (resized.width < 1)
        resized.width = 0;
    if (resized.height < 1)
        resized.height = 0;
    engine::copy_rectangle_fields(&parent->bounds, &resized);
    if (dx || dy)
        events::dispatch(parent, 42, Dword(dx), Dword(dy));
    engine::destroy_rectangle(&client);
    engine::destroy_point(&local);
    return 1;
}

Dword __fastcall frame_pointer_release(Widget *widget, void *, Dword button, Dword modifiers) {
    auto *frame = reinterpret_cast<Frame *>(widget);
    if (!button && application::pending::captured_widget_id == widget->id) {
        release_capture(widget);
        frame->resize_edges = 0;
        return 1;
    }
    return frame->resize_edges ? 1 : capture_style(widget, nullptr, button, modifiers);
}

Dword __fastcall frame_hit_test(Widget *widget, void *) {
    if (!pending::hit_test(widget, nullptr))
        return 0;
    return widget->parent && invoke<Dword>(widget->parent, 9) ? 0 : 1;
}

Dword __fastcall frame_pointer_press(Widget *widget, void *, Dword button, Dword modifiers) {
    auto *frame = reinterpret_cast<Frame *>(widget);
    if (!button && (frame->style & 15) && application::pending::captured_widget_id != widget->id) {
        if (frame->resize_edges) {
            capture_widget(widget);
            return 1;
        }
    } else if (frame->resize_edges)
        return 1;
    return pending::update_focus_style(widget, nullptr, button, modifiers);
}

void __fastcall frame_prepare_draw(Widget *widget, void *, Dword render_address) {
    auto *frame = reinterpret_cast<Frame *>(widget);
    Widget *parent = widget->parent;
    if (parent && visible_in_ancestors(widget)) {
        Rectangle client;
        client_bounds(parent, &client);
        engine::copy_rectangle_fields(&widget->bounds, &client);
        Rectangle top = border_bounds(widget, frame->top),
                  left = border_bounds(widget, frame->left),
                  right = border_bounds(widget, frame->right),
                  bottom = border_bounds(widget, frame->bottom),
                  top_left = border_bounds(widget, frame->top_left),
                  top_right = border_bounds(widget, frame->top_right),
                  bottom_left = border_bounds(widget, frame->bottom_left);
        Rectangle bottom_right{engine::rectangle_methods, 0, 0, 0, 0};
        if (Widget *child = find_child(widget, frame->bottom_right)) {
            bottom_right.width = child->bounds.width;
            bottom_right.height = child->bounds.height;
        }
        top_left.left = int(Dword(client.left) - Dword(top_left.width));
        top_left.top = int(Dword(client.top) - Dword(top_left.height));
        const int right_edge = int(Dword(client.width) + Dword(client.left));
        const int bottom_edge = int(Dword(client.height) + Dword(client.top));
        top_right.top = int(Dword(client.top) - Dword(top_right.height));
        bottom_left.left = int(Dword(client.left) - Dword(bottom_left.width));
        top.left = int(Dword(top_left.width) + Dword(top_left.left));
        top.top = int(Dword(client.top) - Dword(top.height));
        top.width = int(Dword(right_edge) - Dword(top.left));
        bottom.left = int(Dword(bottom_left.left) + Dword(bottom_left.width));
        bottom.width = int(Dword(right_edge) - Dword(bottom.left));
        left.top = int(Dword(top_left.height) + Dword(top_left.top));
        left.left = int(Dword(client.left) - Dword(left.width));
        left.height = int(Dword(bottom_edge) - Dword(left.top));
        right.top = int(Dword(top_right.top) + Dword(top_right.height));
        right.left = right_edge;
        right.height = int(Dword(bottom_edge) - Dword(right.top));
        bottom_right.top = bottom_edge;
        bottom_right.left = right_edge;
        bottom.top = bottom_edge;
        top_right.left = right_edge;
        bottom_left.top = bottom_edge;
        store_border_bounds(widget, frame->top, &top);
        store_border_bounds(widget, frame->left, &left);
        store_border_bounds(widget, frame->right, &right);
        store_border_bounds(widget, frame->bottom, &bottom);
        store_border_bounds(widget, frame->top_left, &top_left);
        store_border_bounds(widget, frame->top_right, &top_right);
        store_border_bounds(widget, frame->bottom_left, &bottom_left);
        store_border_bounds(widget, frame->bottom_right, &bottom_right);
        engine::destroy_rectangle(&client);
    }
    prepare_draw(widget, nullptr, reinterpret_cast<engine::Engine *>(render_address));
}
} // namespace nfs::gui::widgets::family
