#include "gui_widgets.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
namespace nfs::gui::engine {

const Rectangle *current_viewport_bounds(const Base *engine) {
    return engine->viewport_count ? &engine->viewports[engine->viewport_count - 1] : &source_bounds;
}
namespace pending {
Rectangle *intersect_rectangle(Rectangle *, const Rectangle *, const Rectangle *);
}
} // namespace nfs::gui::engine
namespace nfs::gui::widgets {
using game::runtime::invoke;
namespace {
void copy_bounds(Rectangle *out, const Rectangle *source) {
    out->top = source->top;
    out->left = source->left;
    out->height = source->height;
    out->width = source->width;
}
} // namespace

void union_rectangle(Rectangle *rectangle, const Rectangle *other) {
    const int left = rectangle->left < other->left ? rectangle->left : other->left;
    const int top = rectangle->top < other->top ? rectangle->top : other->top;
    const int right = int(Dword(rectangle->left) + Dword(rectangle->width));
    const int other_right = int(Dword(other->left) + Dword(other->width));
    const int bottom = int(Dword(rectangle->top) + Dword(rectangle->height));
    const int other_bottom = int(Dword(other->top) + Dword(other->height));
    rectangle->left = left;
    rectangle->top = top;
    rectangle->width = int(Dword(right < other_right ? other_right : right) - Dword(left));
    rectangle->height = int(Dword(bottom < other_bottom ? other_bottom : bottom) - Dword(top));
}

Dword __fastcall apply_drawing_state(Widget *widget, void *, engine::Engine *engine) {
    if (!widget->value_d0)
        return 0;
    invoke(engine, 200 / 4, widget->value_d0);
    return 1;
}

void __fastcall prepare_draw(Widget *widget, void *, engine::Engine *engine) {
    if ((widget->flags & 2) && !(widget->layout_flags & 65536))
        return;
    if (widget->layout_flags & 1920)
        invoke(widget, 6, widget->layout_flags & 1920);
    const Dword count = widget->children.count;
    bool deferred = false;
    for (int index = int(count - 1); index >= 0; --index)
        if (auto *child =
                static_cast<Widget *>(events::container_value(&widget->children, Dword(index)))) {
            if (child->layout_flags & 32768)
                deferred = true;
            else
                invoke(child, 68 / 4, engine);
        }
    if (widget->layout_flags & 16676991)
        invoke(widget, 6, widget->layout_flags & 16676991);
    if (deferred) {
        const Dword deferred_count = widget->children.count;
        for (int index = int(deferred_count - 1); index >= 0; --index)
            if (auto *child =
                    static_cast<Widget *>(events::container_value(&widget->children, Dword(index))))
                if (child->layout_flags & 32768)
                    invoke(child, 68 / 4, engine);
    }
}

void __fastcall push_draw_bounds(Widget *widget, void *, engine::Engine *engine) {
    Rectangle clip, viewport;
    engine::construct_rectangle(&clip);
    engine::construct_rectangle(&viewport);
    if (!(widget->flags & 32776)) {
        const Rectangle *current_clip = invoke<const Rectangle *>(engine, 264 / 4);
        if (!(widget->flags & 32) && (!widget->parent || !(widget->parent->flags & 524288)))
            engine::pending::intersect_rectangle(
                &clip, engine::current_viewport_bounds(&engine->base), current_clip);
        else
            copy_bounds(&clip, current_clip);
    } else
        copy_bounds(&clip, &engine::source_bounds);
    invoke(engine, 256 / 4, &clip);
    copy_bounds(&viewport, &widget->bounds);
    if (!(widget->flags & 8)) {
        const Rectangle *parent_viewport = engine::current_viewport_bounds(&engine->base);
        if (widget->parent && !(widget->flags & 64)) {
            viewport.top = int(Dword(viewport.top) + Dword(widget->parent->scroll.y));
            viewport.left = int(Dword(viewport.left) + Dword(widget->parent->scroll.x));
        }
        viewport.top = int(Dword(viewport.top) + Dword(parent_viewport->top));
        viewport.left = int(Dword(viewport.left) + Dword(parent_viewport->left));
    }
    invoke(engine, 272 / 4, &viewport);
    Rectangle intersection;
    engine::pending::intersect_rectangle(&intersection, &viewport, &clip);
    copy_bounds(&widget->clip, &intersection);
    copy_bounds(&widget->padding, widget->flags & 128 ? &engine::source_bounds : &widget->clip);
}

void __fastcall pop_draw_bounds(Widget *widget, void *, engine::Engine *engine) {
    invoke(engine, 260 / 4);
    invoke(engine, 276 / 4);
    if (widget->parent && widget->padding.width && widget->padding.height)
        union_rectangle(&widget->parent->padding, &widget->padding);
}

void register_deferred_widget(ContextState *context, Widget *widget) {
    if (widget && !container_find(&context->fourth, events::compare_widget_id,
                                  reinterpret_cast<void *>(widget->id)))
        container_sorted_insert(&context->fourth, events::compare_widget_id,
                                reinterpret_cast<void *>(widget->id), widget, nullptr);
}

void __fastcall request_layout(Widget *widget, void *) {
    widget->options |= 2;
    if (widget->flags & 2097152)
        register_deferred_widget(context_state, widget);
    widget->timer = 0;
    events::post(context_state, widget->id, 19, 0, 0, 0);
}
} // namespace nfs::gui::widgets
