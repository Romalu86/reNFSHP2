#include "nfs/gui/engine/gui_engine.hpp"
#include "nfs/gui/resources/gui_format.hpp"
#include "../lifecycle/gui_render_numeric_x87.hpp"
#include <cmath>
namespace gui_numeric = ::nfs::gui::retained;
namespace nfs::gui::engine {


Rectangle *to_view(Rectangle *out, const Rectangle *source) {

    const auto coordinate = [&](int value, float scale, float offset) {
        const float stored = float(gui_numeric::integer(value) * gui_numeric::number(scale) -
                                   gui_numeric::number(offset));
        return int(gui_numeric::round_f32(stored));
    };
    const int bottom = coordinate(int(Dword(source->top) + Dword(source->height)),
                                  inverse_vertical_scale, vertical_offset);
    const int right = coordinate(int(Dword(source->left) + Dword(source->width)),
                                 inverse_horizontal_scale, horizontal_offset);
    const int top = coordinate(source->top, inverse_vertical_scale, vertical_offset);
    const int left = coordinate(source->left, inverse_horizontal_scale, horizontal_offset);
    *out = Rectangle{rectangle_methods, top, left, int(Dword(right) - Dword(left)),
                     int(Dword(bottom) - Dword(top))};
    return out;
}
namespace pending {
// This copy of the case map takes a 16-bit code.
static int alternate_text_case(std::uint16_t code) {
    return fonts::alternate_case(code);
}

static const Byte *find_text_glyph(const fonts::Font *font, Dword code) {
    const auto *glyphs = reinterpret_cast<const Byte *>(font) + font->glyph_offset;
    const int stride = font->flags & 262144 ? 16 : 12;
    const int index = int(code - 32);
    if (index < font->count) {
        const auto *glyph = reinterpret_cast<const Byte *>(reinterpret_cast<Dword>(glyphs) +
                                                           Dword(index) * Dword(stride));
        if (*reinterpret_cast<const std::uint16_t *>(glyph) == code)
            return glyph;
    }
    return fonts::find_code(code, glyphs, font->count, stride);
}
namespace {

template <class Character>
void measure_glyphs(fonts::Font *font, const Character *text, float *left, float *top, float *width,
                    float *height) {

    const auto number = [&](int value) { return gui_numeric::integer(value); };
    const long double scale_x = gui_numeric::number(font->scale_x),
                      scale_y = gui_numeric::number(font->scale_y);
    float x = 0, y = 0, minimum_x = 1e7f, minimum_y = 1e7f, maximum_x = -1e7f, maximum_y = -1e7f;
    Dword previous = 0;
    for (; *text; ++text) {
        const Dword code = *text;
        const Byte *glyph = find_text_glyph(font, code);
        if (!glyph) {
            if (code == 10) {
                const int line =
                    int(gui_numeric::truncate(number(font->ascent + font->descent) * scale_y));
                y = float(number(line) + gui_numeric::number(y));
                x = 0;
                previous = 0;
                continue;
            }
            const int other_case = alternate_text_case(std::uint16_t(code));
            if (other_case)
                glyph = find_text_glyph(font, Dword(other_case));
            if (!glyph)
                glyph = find_text_glyph(font, 127);
            if (!glyph)
                continue;
        }
        const auto pen_x =
            number(fonts::kerning(font, glyph, previous)) * scale_x + gui_numeric::number(x);
        const int advance = font->flags & 262144
                                ? *reinterpret_cast<const std::int16_t *>(glyph + 14)
                                : int(static_cast<std::int8_t>(glyph[8]));
        const auto advance_width = number(advance) * scale_x;
        const auto glyph_x = number(static_cast<std::int8_t>(glyph[9])) * scale_x + pen_x;
        const float glyph_y =
            float(number(static_cast<std::int8_t>(glyph[10])) * scale_y + gui_numeric::number(y));
        if (glyph_x < gui_numeric::number(minimum_x))
            minimum_x = float(glyph_x);
        if (glyph_y < minimum_y)
            minimum_y = glyph_y;
        auto glyph_width = number(glyph[2]) * scale_x;
        if (code == 32)
            glyph_width = glyph_width + advance_width;
        const auto right = glyph_x + glyph_width;
        const float bottom = float(number(glyph[3]) * scale_y + gui_numeric::number(glyph_y));
        if (gui_numeric::number(maximum_x) < right)
            maximum_x = float(right);
        if (maximum_y < bottom)
            maximum_y = bottom;
        x = float(advance_width + pen_x);
        previous = code;
    }
    if (left)
        *left = minimum_x < maximum_x ? minimum_x : 0;
    if (top)
        *top = minimum_y < maximum_y ? minimum_y : 0;
    if (width)
        *width = minimum_x < maximum_x
                     ? float(gui_numeric::number(maximum_x) - gui_numeric::number(minimum_x))
                     : 0;
    if (height) {
        if constexpr (sizeof(Character) == 1)
            *height = minimum_y < maximum_y
                          ? float(gui_numeric::number(maximum_y) - gui_numeric::number(minimum_y))
                          : 0;
        else
            *height = minimum_y < maximum_y
                          ? float(gui_numeric::number(maximum_y) - gui_numeric::number(minimum_y))
                          : 0;
    }
}
} // namespace

void measure_ascii_text(fonts::Font *font, const Byte *text, float *left, float *top, float *width,
                        float *height) {
    measure_glyphs(font, text, left, top, width, height);
}

Dword __fastcall measure_string(Engine *engine, void *, Dword string_address,
                               Dword rectangle_address) {
    const auto *text = reinterpret_cast<const String *>(string_address);
    if (!engine->context || !string_data(text) || !text->length)
        return 0;
    auto *font = get_font(engine->font->font);
    if (!font)
        return 0;

    font->scale_x =
        float(gui_numeric::number(engine->scale_x) * gui_numeric::number(engine->font->scale_x) *
              gui_numeric::number(horizontal_scale));
    font->scale_y =
        float(gui_numeric::number(engine->font->scale_y) * gui_numeric::number(engine->scale_y) *
              gui_numeric::number(vertical_scale));
    float left = 0, top = 0, width = 0, height = 0;
    if (text->wide)
        measure_glyphs(font, static_cast<const std::uint16_t *>(string_data(text)), &left, &top,
                       &width, &height);
    else
        measure_ascii_text(font, static_cast<const Byte *>(string_data(text)), &left, &top, &width,
                           &height);
    auto *out = reinterpret_cast<Rectangle *>(rectangle_address);
    out->left = int(gui_numeric::truncate(gui_numeric::number(left) + 0.5f));
    out->top = int(gui_numeric::truncate(gui_numeric::number(top) + 0.5f));
    out->width = int(gui_numeric::truncate(gui_numeric::number(width) + 0.5f));
    out->height = int(gui_numeric::truncate(gui_numeric::number(height) + 0.5f));
    Rectangle view;
    to_view(&view, out);
    out->left = view.left;
    out->top = view.top;
    out->width = view.width;
    out->height = view.height;
    destroy_rectangle(&view);
    return 1;
}
} // namespace pending
} // namespace nfs::gui::engine
