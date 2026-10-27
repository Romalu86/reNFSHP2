#include "nfs/gui/engine/gui_engine.hpp"
#include "nfs/gui/resources/gui_format.hpp"
#include "../lifecycle/gui_images.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include "../lifecycle/gui_render_numeric_x87.hpp"
#include <cmath>
namespace gui_numeric = ::nfs::gui::retained;
namespace nfs::gui::engine {
Rectangle *to_pixels(Rectangle *, const Rectangle *);
Rectangle *to_view(Rectangle *, const Rectangle *);
Dword *copy_colour(Dword *, const Dword *);
namespace pending {
using game::runtime::at;
using game::runtime::invoke;

Dword update_scissor(Engine *);
Rectangle *intersect_rectangle(Rectangle *, const Rectangle *, const Rectangle *);
void draw_pixel_line(Engine *, int, int, int, int, Dword, Dword);
namespace {
// The saved SEH cleanup changes each live rectangle back to the base vtable.
struct RectangleCleanup {
    Rectangle &value;
    ~RectangleCleanup() {
        destroy_rectangle(&value);
    }
};
} // namespace

static void draw_font_ascii(int index, Dword *viewport, float x, float y, const char *text) {
    if (auto *face = get_font_face(index))
        fonts::faces::draw(face, viewport, x, y, text);
}

static void draw_font_wide(int index, Dword *viewport, float x, float y, const std::uint16_t *text) {
    if (auto *face = get_font_face(index))
        if (face->font) {
            if (face->patchwork)
                fonts::patchwork::draw_wide(face->patchwork, face->font, text, viewport, x, y,
                                            face->font->scale_x, face->font->scale_y,
                                            face->font->unknown_20);
            else
                fonts::draw_wide(face->font, x, y, text);
        }
}

void __fastcall draw_string(Engine *engine, void *, Dword string_address, Dword horizontal,
                            Dword vertical, Dword colour_address, Dword bounds_address) {
    const auto *text = reinterpret_cast<const String *>(string_address);
    if (!text->length || !update_scissor(engine))
        return;
    auto *font = get_font(engine->font->font);
    if (!font)
        return;
    const Dword saved_pool = kernel::default_pool;
    kernel::default_pool = lifecycle::ui_pool;
    Rectangle intersection, clipping;
    intersect_rectangle(&intersection, invoke<const Rectangle *>(engine, 66), &source_bounds);
    to_pixels(&clipping, &intersection);
    RectangleCleanup clipping_cleanup{clipping};
    destroy_rectangle(&intersection);
    const Dword flags = engine->font->flags;

    const auto coordinate = [&](int value, float scale, float offset) {
        const float stored = float(gui_numeric::integer(value) * gui_numeric::number(scale) +
                                   gui_numeric::number(offset));
        return int(gui_numeric::round_f32(stored));
    };
    const int x = coordinate(int(horizontal), horizontal_scale, horizontal_offset);
    const int y = coordinate(int(vertical), vertical_scale, vertical_offset);
    font->unknown_28 = 0;
    font->scale_x =
        float(gui_numeric::number(engine->scale_x) * gui_numeric::number(engine->font->scale_x) *
              gui_numeric::number(horizontal_scale));
    font->scale_y =
        float(gui_numeric::number(engine->font->scale_y) * gui_numeric::number(engine->scale_y) *
              gui_numeric::number(vertical_scale));
    at<Dword>(font, 72) = 1;
    const auto *colour = reinterpret_cast<const Dword *>(colour_address);
    Dword foreground, shadow, shadow_argb = *colour & 4278190080u;
    copy_colour(&foreground, colour);
    copy_colour(&shadow, &shadow_argb);
    const auto draw = [&](float left, float top) {
        if (images::suspend_drawing())
            return;
        if (text->wide)
            draw_font_wide(engine->font->font, engine->viewport, left, top,
                           static_cast<const std::uint16_t *>(string_data(text)));
        else
            draw_font_ascii(engine->font->font, engine->viewport, left, top,
                            static_cast<const char *>(string_data(text)));
    };
    if (flags & 16) {
        font->unknown_20 = shadow;
        draw(float(gui_numeric::integer(x) + gui_numeric::number(0.75f)),
             float(gui_numeric::integer(y) + gui_numeric::number(0.75f)));
    }
    font->unknown_20 = foreground;
    draw(float(x), float(y));
    if (flags & 2)
        draw(float(x), float(y));
    Rectangle bounds;
    construct_rectangle(&bounds);
    RectangleCleanup bounds_cleanup{bounds};
    if (bounds_address || (flags & 12)) {
        invoke<Dword>(engine, 15, text, &bounds);
        Rectangle pixels;
        to_pixels(&pixels, &bounds);
        bounds.top = pixels.top;
        bounds.left = pixels.left;
        bounds.width = pixels.width;
        bounds.height = pixels.height;
        destroy_rectangle(&pixels);
        Point origin{point_methods, x, y};
        bounds.left = int(Dword(bounds.left) + Dword(origin.x));
        bounds.top = int(Dword(bounds.top) + Dword(origin.y));
        destroy_point(&origin);
    }
    if ((flags & 12) && (Dword(bounds.width) | Dword(bounds.height))) {
        const float pixel_height =
            float(gui_numeric::number(engine->font->height) * gui_numeric::number(vertical_scale));
        const int rounded_height = int(gui_numeric::round_f32(pixel_height));
        const auto ascent =
            gui_numeric::integer(rounded_height) * gui_numeric::number(engine->font->spacing);
        const int right = int(Dword(bounds.left) + Dword(bounds.width));
        if (flags & 4) {
            const int line_y = int(Dword(gui_numeric::truncate(ascent * gui_numeric::number(0.5f) +
                                                              gui_numeric::number(0.5f))) +
                                   Dword(y));
            draw_pixel_line(engine, bounds.left, line_y, right, line_y, *colour, *colour);
        }
        if (flags & 8) {
            const int line_y =
                int(Dword(gui_numeric::truncate(ascent + gui_numeric::number(0.5f))) + 1 + Dword(y));
            draw_pixel_line(engine, bounds.left, line_y, right, line_y, *colour, *colour);
        }
    }
    if (bounds_address) {
        Rectangle view;
        to_view(&view, &bounds);
        auto *out = reinterpret_cast<Rectangle *>(bounds_address);
        out->left = view.left;
        out->top = view.top;
        out->width = view.width;
        out->height = view.height;
        destroy_rectangle(&view);
    }
    kernel::default_pool = saved_pool;
}
} // namespace pending
} // namespace nfs::gui::engine
