#include "gui_widget_text.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
namespace nfs::gui::widgets::family {
using game::runtime::at;
using game::runtime::invoke;

void __fastcall text_layout(Widget *widget, void *, Dword flags) {
    auto *render = reinterpret_cast<engine::Engine *>(active_engine());
    if (render && (flags & 64)) {
        Rectangle measured;
        engine::construct_rectangle(&measured);
        using SetFont = Dword(__thiscall *)(engine::Engine *, Dword);
        using Measure = Dword(__thiscall *)(engine::Engine *, const String *, Rectangle *);
        const Method *font_methods = render->base.methods;
        const Dword requested_font = inherited_font_style(widget);
        const Dword font = reinterpret_cast<SetFont>(font_methods[14])(render, requested_font);
        if (!(at<Dword>(widget, 264) & 1)) {
            const Method *measure_methods = render->base.methods;
            const String *text = invoke<const String *>(widget, 8);
            if (reinterpret_cast<Measure>(measure_methods[15])(render, text, &measured)) {
                const Dword width =
                    Dword(measured.left) + at<Dword>(widget, 272) * 2 + Dword(measured.width);
                const int top = widget->bounds.top, left = widget->bounds.left;
                const int height = invoke<int>(render, 17);
                widget->bounds.left = left;
                widget->bounds.top = top;
                widget->bounds.height = height;
                widget->bounds.width = int(width);
            }
        } else {
            const bool fit_width = (at<Dword>(widget, 264) & 17) != 17;
            const Dword font_height = invoke<Dword>(render, 17);
            Rectangle result = widget->bounds;
            result.methods = engine::rectangle_methods;
            String line;
            string_construct(&line);
            if (fit_width) {
                if (at<Dword>(widget, 264) & 2)
                    result.left = int(Dword(result.left) + Dword(result.width));
                else if (at<Dword>(widget, 264) & 4)
                    result.left = int(Dword(result.left) + Dword(result.width / 2));
                result.width = 0;
            }
            int state[3];
            if (first_text_line(widget, &line, &measured, state)) {
                result.height = int(at<Dword>(widget, 276) + font_height);
                int width = int(Dword(measured.width) + at<Dword>(widget, 272) * 2);
                if (fit_width && result.width <= width)
                    result.width = width;
                while (next_text_line(widget, &line, &measured, state)) {
                    result.height =
                        int(Dword(result.height) + at<Dword>(widget, 276) + font_height);
                    width = int(Dword(measured.width) + at<Dword>(widget, 272) * 2);
                    if (fit_width && result.width <= width)
                        result.width = width;
                }
            }
            if (fit_width) {
                if (at<Dword>(widget, 264) & 2)
                    result.left = int(Dword(result.left) - Dword(result.width));
                else if (at<Dword>(widget, 264) & 4)
                    result.left = int(Dword(result.left) - Dword(result.width / 2));
            }
            widget->bounds.top = result.top;
            widget->bounds.left = result.left;
            widget->bounds.height = result.height;
            widget->bounds.width = result.width;
            string_destruct(&line);
            engine::destroy_rectangle(&result);
        }
        invoke(render, 14, font);
        engine::destroy_rectangle(&measured);
    }
    pending::layout(widget, nullptr, flags);
}
// RET14h includes a fifth, unused stack argument.
void __fastcall draw_text_line(Widget *, void *, engine::Engine *render, const String *text,
                               const Rectangle *bounds, const Dword *colour, Dword) {
    invoke(render, 42, text, Dword(bounds->left), Dword(bounds->top), colour, Dword(0));
}

void __fastcall text_draw(Widget *widget, void *, Dword render_address) {
    auto *render = reinterpret_cast<engine::Engine *>(render_address);
    Rectangle clip = *invoke<const Rectangle *>(render, 66);
    clip.methods = engine::rectangle_methods;
    if (at<Dword>(widget, 264) & 8) {
        engine::intersect_in_place(&clip, engine::current_viewport_bounds(&render->base));
        invoke(render, 64, &clip);
    }
    Rectangle viewport = *engine::current_viewport_bounds(&render->base);
    viewport.methods = engine::rectangle_methods;
    Point offset{engine::point_methods, int(Dword(widget->scroll.x) + Dword(viewport.left)),
                 int(Dword(widget->scroll.y) + Dword(viewport.top))};
    Dword colour = widget->colour;
    clip.top = int(Dword(clip.top) - Dword(viewport.top) - Dword(widget->scroll.y));
    clip.left = int(Dword(clip.left) - Dword(viewport.left) - Dword(widget->scroll.x));
    using SetStyle = void(__thiscall *)(engine::Engine *, Dword);
    using Fill = void(__thiscall *)(engine::Engine *, const Rectangle *, const Dword *, Dword);
    const Method *font_methods = render->base.methods;
    const Dword font = inherited_font_style(widget);
    reinterpret_cast<SetStyle>(font_methods[14])(render, font);
    const Method *palette_methods = render->base.methods;
    const Dword palette = inherited_palette(widget);
    reinterpret_cast<SetStyle>(palette_methods[21])(render, palette);
    const Dword style = at<Dword>(widget, 264);
    if (style & 64) {
        Dword background;
        const Method *methods = render->base.methods;
        Dword *colour = invoke<Dword *>(render, 23, &background, Dword(4));
        reinterpret_cast<Fill>(methods[48])(render, &viewport, colour, Dword(1));
    }
    if (!(style & 407)) {
        Rectangle bounds, translated;
        text_content_bounds(widget, &bounds);
        engine::translate_rectangle(&translated, &bounds, &offset);
        const Method *methods = widget->methods;
        const String *text = invoke<const String *>(widget, 8);
        using Draw = void(__thiscall *)(Widget *, engine::Engine *, const String *,
                                        const Rectangle *, const Dword *, Dword);
        reinterpret_cast<Draw>(methods[47])(widget, render, text, &translated, &colour, Dword(0));
        engine::destroy_rectangle(&translated);
        engine::destroy_rectangle(&bounds);
    } else {
        String line;
        string_construct(&line);
        Rectangle bounds;
        engine::construct_rectangle(&bounds);
        int state[3];
        const Dword initial_colour = colour;
        auto draw_visible_line = [&] {
            if (clip.top < int(Dword(bounds.top) + Dword(bounds.height)) &&
                bounds.top < int(Dword(clip.top) + Dword(clip.height))) {
                Rectangle translated;
                engine::translate_rectangle(&translated, &bounds, &offset);
                invoke(widget, 47, render, &line, &translated, &colour, Dword(state[0]));
                engine::destroy_rectangle(&translated);
            }
        };
        if (first_text_line(widget, &line, &bounds, state)) {
            draw_visible_line();
            if (at<Dword>(widget, 264) & 1)
                while (next_text_line(widget, &line, &bounds, state) &&
                       bounds.top <= int(Dword(clip.top) + Dword(clip.height))) {
                    colour = initial_colour;
                    draw_visible_line();
                }
        }
        engine::destroy_rectangle(&bounds);
        string_destruct(&line);
    }
    const Dword current_style = at<Dword>(widget, 264);
    if (current_style & 32) {
        Dword border;
        const Method *methods = render->base.methods;
        Dword *colour = invoke<Dword *>(render, 23, &border, Dword(5));
        reinterpret_cast<Fill>(methods[48])(render, &viewport, colour, Dword(0));
    }
    if (current_style & 8)
        invoke(render, 65);
    engine::destroy_point(&offset);
    engine::destroy_rectangle(&viewport);
    engine::destroy_rectangle(&clip);
}
} // namespace nfs::gui::widgets::family
