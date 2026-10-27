#include "nfs/graphics/fonts/patchwork_font.hpp"
#include "nfs/graphics/primitives/immediate_draw.hpp"
#include "nfs/graphics/display/viewport_transform.hpp"
#include "nfs/gui/lifecycle/gui_coordinates.hpp"
#include "../../math/precision/extended_float.hpp"
#include <cstring>
namespace nfs::fonts::patchwork {
namespace gfx = graphics;
namespace {
using kernel::numeric::Extended;
Extended integer(std::int32_t value) {
    return Extended::integer(value, kernel::numeric::current_format());
}
float newline_y(Font *font, float scale, float y) {
    const auto step = std::int32_t(Dword((integer(font->ascent + font->descent) * scale).truncate_to_integer()));
    return float(integer(step) + y);
}
} // namespace

void draw_glyph(Cache *cache, const Glyph *glyph, Dword *viewport, float x, float y, float scale_x,
                float scale_y, Dword colour) {
    float origin_x, origin_y, width, height;
    Dword minimum_depth, maximum_depth;
    gfx::viewports::read_rectangle(viewport, &origin_x, &origin_y, &width, &height, &minimum_depth,
                                   &maximum_depth);
    x = float(integer(glyph->bearing_x) * scale_x + x);
    y = float(integer(glyph->bearing_y) * scale_y + y);
    const float right = float(integer(glyph->width) * scale_x + x);
    const float bottom = float(integer(glyph->height) * scale_y + y);
    const float left_u = float(integer(glyph->x) / integer(cache->page_width));
    const float right_u = float(integer(unsigned(glyph->x) + glyph->width) / integer(cache->page_width));
    const float top_v = float(integer(glyph->y) / integer(cache->page_height));
    const float bottom_v = float(integer(unsigned(glyph->y) + glyph->height) / integer(cache->page_height));
    gfx::immediate::set_textured_primitive(cache->draw, 5);
    float position[4], uv[4];
    gfx::gui_coordinates::vector(uv, left_u, top_v, 1, 1);
    gfx::gui_coordinates::screen_position(position, x, y, origin_x, origin_y, width, height);
    gfx::immediate::textured_vertex(cache->draw, position, &colour, uv);
    gfx::gui_coordinates::vector(uv, right_u, top_v, 1, 1);
    gfx::gui_coordinates::screen_position(position, right, y, origin_x, origin_y, width, height);
    gfx::immediate::textured_vertex(cache->draw, position, &colour, uv);
    gfx::gui_coordinates::vector(uv, left_u, bottom_v, 1, 1);
    gfx::gui_coordinates::screen_position(position, x, bottom, origin_x, origin_y, width, height);
    gfx::immediate::textured_vertex(cache->draw, position, &colour, uv);
    gfx::gui_coordinates::vector(uv, right_u, bottom_v, 1, 1);
    gfx::gui_coordinates::screen_position(position, right, bottom, origin_x, origin_y, width,
                                          height);
    gfx::immediate::textured_vertex(cache->draw, position, &colour, uv);
    gfx::immediate::end_textured_primitive(cache->draw);
    gfx::immediate::flush_textured(cache->draw);
}

void draw_page(Cache *cache, Dword *viewport, float x, float y, float scale, Dword colour) {
    upload(cache, viewport);
    float origin_x, origin_y, width, height;
    Dword minimum_depth, maximum_depth;
    gfx::viewports::read_rectangle(viewport, &origin_x, &origin_y, &width, &height, &minimum_depth,
                                   &maximum_depth);
    const float right = float(integer(cache->page_width) * scale + x);
    const float bottom = float(integer(cache->page_height) * scale + y);
    gfx::immediate::set_texture(cache->draw, reinterpret_cast<Dword>(cache->pages[0].texture));
    gfx::immediate::set_textured_primitive(cache->draw, 5);
    float position[4], uv[4];
    gfx::gui_coordinates::vector(uv, 0, 0, 1, 1);
    gfx::gui_coordinates::screen_position(position, x, y, origin_x, origin_y, width, height);
    gfx::immediate::textured_vertex(cache->draw, position, &colour, uv);
    gfx::gui_coordinates::vector(uv, 1, 0, 1, 1);
    gfx::gui_coordinates::screen_position(position, right, y, origin_x, origin_y, width, height);
    gfx::immediate::textured_vertex(cache->draw, position, &colour, uv);
    gfx::gui_coordinates::vector(uv, 0, 1, 1, 1);
    gfx::gui_coordinates::screen_position(position, x, bottom, origin_x, origin_y, width, height);
    gfx::immediate::textured_vertex(cache->draw, position, &colour, uv);
    gfx::gui_coordinates::vector(uv, 1, 1, 1, 1);
    gfx::gui_coordinates::screen_position(position, right, bottom, origin_x, origin_y, width,
                                          height);
    gfx::immediate::textured_vertex(cache->draw, position, &colour, uv);
    gfx::immediate::end_textured_primitive(cache->draw);
    gfx::immediate::flush_textured(cache->draw);
}

void draw_wide(Cache *cache, Font *font, const std::uint16_t *text, Dword *viewport, float x,
               float y, float scale_x, float scale_y, Dword colour) {
    if (!text || !viewport || !font)
        return;
    int length = 0;
    while (text[length])
        ++length;
    for (int i = 0; i < length; ++i)
        cache_glyph(cache, text[i], font);
    upload(cache, viewport);
    for (int page = 0; page < cache->page_count; ++page) {
        gfx::immediate::set_texture(cache->draw,
                                    reinterpret_cast<Dword>(cache->pages[page].texture));
        float pen_x = x, pen_y = y;
        for (int i = 0; i < length; ++i) {
            const std::uint16_t code = text[i];
            if (code == 10) {
                pen_x = x;
                pen_y = newline_y(font, scale_y, pen_y);
            } else {
                Glyph *glyph = find(cache, code, font);
                if (glyph && glyph->code == code) {
                    if (glyph->page == page)
                        draw_glyph(cache, glyph, viewport, pen_x, pen_y, scale_x, scale_y, colour);
                    pen_x = float(integer(glyph->advance) * scale_x + pen_x);
                }
            }
        }
    }
}

void draw_text(Cache *cache, Font *font, const char *text, Dword *viewport, float x, float y,
               float scale_x, float scale_y, Dword colour) {
    if (!text || !viewport || !font)
        return;
    const int length = static_cast<int>(std::strlen(text));
    // Both saved passes zero-extend the byte character.
    for (int i = 0; i < length; ++i)
        cache_glyph(cache, static_cast<Byte>(text[i]), font);
    upload(cache, viewport);
    for (int page = 0; page < cache->page_count; ++page) {
        gfx::immediate::set_texture(cache->draw,
                                    reinterpret_cast<Dword>(cache->pages[page].texture));
        float pen_x = x, pen_y = y;
        for (int i = 0; i < length; ++i) {
            const Byte code = static_cast<Byte>(text[i]);
            if (code == 10) {
                pen_x = x;
                pen_y = newline_y(font, scale_y, pen_y);
            } else {
                Glyph *glyph = find(cache, code, font);
                if (glyph && glyph->code == code) {
                    if (glyph->page == page)
                        draw_glyph(cache, glyph, viewport, pen_x, pen_y, scale_x, scale_y, colour);
                    pen_x = float(integer(glyph->advance) * scale_x + pen_x);
                }
            }
        }
    }
}
} // namespace nfs::fonts::patchwork
