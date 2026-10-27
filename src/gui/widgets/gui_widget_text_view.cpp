#include "gui_widget_text.hpp"
#include "gui_widget_size.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
namespace nfs::gui::engine {
Dword rectangles_differ(const Rectangle *, const Rectangle *);
}
namespace nfs::gui::widgets::family {
using game::runtime::at;
using game::runtime::invoke;

void __fastcall text_view_set_text(Widget *widget, void *, const String *text) {
    Widget *child = find_child(widget, at<Dword>(widget, 300));
    if (!child || string_equal(invoke<const String *>(child, 8), text))
        return;
    Rectangle previous = child->bounds;
    previous.methods = engine::rectangle_methods;
    invoke(child, 7, text);
    invoke(child, 6, Dword(128));
    invoke(child, 6, Dword(64));
    if (engine::rectangles_differ(&previous, &child->bounds)) {
        widget->scroll.x = engine::default_point.x;
        widget->scroll.y = engine::default_point.y;
    }
    engine::destroy_rectangle(&previous);
}

const String *__fastcall text_view_text(Widget *widget, void *) {
    Widget *child = find_child(widget, at<Dword>(widget, 300));
    return child ? invoke<const String *>(child, 8)
                 : &lifecycle::construction::unresolved::shared_string;
}

void __fastcall text_view_initialize(Widget *widget, void *) {
    pending::request_general_layout(reinterpret_cast<GeneralWidget *>(widget), nullptr);
    auto *child = static_cast<Widget *>(application::pending::find_child(widget, "<TEXT>"));
    if (!child)
        return;
    at<Dword>(widget, 300) = child->id;
    const Method *child_methods = child->methods;
    const String *initial_text = widgets::text(widget, nullptr);
    reinterpret_cast<void(__thiscall *)(Widget *, const String *)>(child_methods[7])(child,
                                                                                   initial_text);
    invoke(child, 6, Dword(128));
    invoke(child, 6, Dword(64));
    auto *render = reinterpret_cast<engine::Engine *>(active_engine());
    if (!render)
        return;
    const Method *render_methods = at<const Method *>(render, 0);
    const Dword style = inherited_font_style(child);
    const Dword font = reinterpret_cast<Dword(__thiscall *)(engine::Engine *, Dword)>(
        render_methods[14])(render, style);
    Point step = at<Point>(widget, 284);
    step.methods = vector_methods;
    step.y = invoke<int>(render, 17);
    Rectangle measured;
    engine::construct_rectangle(&measured);
    String sample;
    string_construct_ascii(&sample, "W");
    const Dword measured_sample = invoke<Dword>(render, 15, &sample, &measured);
    string_destruct(&sample);
    if (measured_sample)
        step.x = measured.width;
    Point temporary;
    assign_size(&at<Point>(widget, 284), &temporary, &step);
    invoke(render, 14, font);
    engine::destroy_point(&temporary);
    engine::destroy_rectangle(&measured);
    engine::destroy_point(&step);
}
 
void __fastcall tree_initialize(Widget *widget, void *) {
    pending::request_general_layout(reinterpret_cast<GeneralWidget *>(widget), nullptr);
}
} // namespace nfs::gui::widgets::family
