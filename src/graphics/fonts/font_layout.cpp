#include "nfs/graphics/fonts/fonts.hpp"
#include "../../gui/lifecycle/gui_render_numeric_x87.hpp"
#include "nfs/core/memory/memory_primitives.hpp"
#include <cstring>
namespace gui_numeric = ::nfs::gui::retained;
namespace nfs::fonts {
// No-op callback.
void __cdecl draw_empty(Font *, const Byte *, float, float) {
}
static const Callbacks default_callbacks{draw_empty, nullptr, nullptr, nullptr, nullptr};
const Callbacks *callbacks = &default_callbacks;
State state{};
namespace {
template <class T> T read(const Byte *p) {
    T value;
    std::memcpy(&value, p, sizeof value);
    return value;
}
const Byte *table(const Font *font, Dword offset) {
    return reinterpret_cast<const Byte *>(font) + offset;
}
const Byte *lookup(const Font *font, Dword code) {
    int stride = (font->flags & 262144) ? 16 : 12;
    auto *first = table(font, font->glyph_offset);
    Dword ordinal = code - 32;

    if (static_cast<std::int32_t>(ordinal) < font->count) {
        auto *glyph =
            reinterpret_cast<const Byte *>(reinterpret_cast<Dword>(first) + ordinal * stride);
        if (read<std::uint16_t>(glyph) == code)
            return glyph;
    }
    return find_code(code, first, font->count, stride);
}
const Byte *selected_glyph(const Font *font, Dword code) {
    const Byte *glyph = lookup(font, code);
    if (glyph || code == 10)
        return glyph;
    int alternate = alternate_case(code);
    if (alternate && (glyph = lookup(font, alternate)) != nullptr)
        return glyph;
    return lookup(font, 127);
}
int advance(const Font *font, const Byte *glyph) {
    return font->flags & 262144 ? read<std::int16_t>(glyph + 14)
                                 : static_cast<std::int8_t>(glyph[8]);
}
template <class Character>
void draw_string(Font *font, float origin_x, float origin_y, const Character *text) {
    float x = origin_x, y = origin_y;
    Dword previous = 0;
    int count = 0;
    DrawItem items[128];
    if (callbacks->begin)
        callbacks->begin(font);
    const float scale = font->scale_x, scale_y = font->scale_y;
    while (*text) {
        Dword code = *text;
        const Byte *glyph = selected_glyph(font, code);
        if (!glyph) {
            if (code == 10) {
                int line =
                    int(gui_numeric::truncate(gui_numeric::integer(font->ascent + font->descent) *
                                              gui_numeric::number(scale_y)));
                x = origin_x;
                previous = 0;
                y = float(gui_numeric::integer(line) + gui_numeric::number(y));
            }
        } else {
            x = float(gui_numeric::integer(kerning(font, glyph, previous)) *
                          gui_numeric::number(scale) +
                      gui_numeric::number(x));
            if (!state.settings.batch)
                callbacks->draw(font, glyph, x, y);
            else {
                items[count++] = {glyph, x, y};
                if (count == 128) {
                    state.settings.batch(font, items, 128);
                    count = 0;
                }
            }
            const int step = advance(font, glyph);
            previous = *text;
            x = float(gui_numeric::integer(step) * gui_numeric::number(scale) +
                      gui_numeric::number(x));
        }
        ++text;
    }
    if (count)
        state.settings.batch(font, items, count);
    if (callbacks->end)
        callbacks->end(font);
}
} // namespace

const Byte *find_code(Dword code, const Byte *first, int count, int stride) {
    while (count) {
        const Byte *middle = first + (count >> 1) * stride;
        Dword found = read<std::uint16_t>(middle);
        if (code == found)
            return middle;
        if (static_cast<std::int32_t>(code - found) >= 0) {
            first = middle + stride;
            --count;
        }
        count >>= 1;
    }
    return nullptr;
}

int alternate_case(int code) {
    if (code > 64 && code < 91)
        return code + 32;
    if (code > 96 && code < 123)
        return code - 32;
    if (code > 191 && code < 215)
        return code + 32;
    if (code > 215 && code < 223)
        return code + 32;
    if (code > 223 && code < 247)
        return code - 32;
    if (code > 247 && code < 255)
        return code - 32;
    return 0;
}

const Byte *find_kerning(int count, const Byte *records, Dword previous) {
    for (int i = 0; i < count; ++i)
        if (read<std::uint16_t>(records + i * 4) == previous)
            return records + i * 4;
    return nullptr;
}

const Byte *find_pair(int count, const Byte *records, Dword previous, Dword code) {
    for (int i = 0; i < count; ++i)
        if (read<std::uint16_t>(records + i * 4) == previous && records[i * 4 + 3] == code)
            return records + i * 4;
    return nullptr;
}

int kerning(const Font *font, const Byte *glyph, Dword previous) {
    int count = static_cast<std::int8_t>(glyph[11]);
    if (!count)
        return 0;
    const Byte *records = table(font, font->kerning_offset);
    const Byte *found;
    if (font->flags & 262144)
        found = find_kerning(count, records + 4 + read<std::uint16_t>(glyph + 12) * 4, previous);
    else
        found = find_pair(read<int>(records), records + 4, previous, read<std::uint16_t>(glyph));
    return found ? static_cast<std::int8_t>(found[2]) : 0;
}

Dword height(const Font *font) {
    return font->height;
}

void draw_text(Font *font, float x, float y, const Byte *text) {
    draw_string(font, x, y, text);
}

void draw_wide(Font *font, float x, float y, const std::uint16_t *text) {
    draw_string(font, x, y, text);
}

void measure(Font *font, const Byte *text, float *left, float *top, float *width,
             float *height_out) {
    float min_x = 1e7f, min_y = 1e7f, max_x = -1e7f, max_y = -1e7f, x = 0, y = 0;
    const float scale_x = font->scale_x, scale_y = font->scale_y;
    Dword previous = 0;
    while (*text) {
        Dword code = *text;
        const Byte *glyph = selected_glyph(font, code);
        if (!glyph) {
            if (code == 10) {
                const int line =
                    int(gui_numeric::truncate(gui_numeric::integer(font->ascent + font->descent) *
                                              gui_numeric::number(scale_y)));
                previous = 0;
                y = float(gui_numeric::integer(line) + gui_numeric::number(y));
                x = 0;
            }
        } else {
            const long double pen_x = gui_numeric::integer(kerning(font, glyph, previous)) *
                                          gui_numeric::number(scale_x) +
                                      gui_numeric::number(x);
            const long double glyph_x = gui_numeric::integer(static_cast<std::int8_t>(glyph[9])) *
                                            gui_numeric::number(scale_x) +
                                        pen_x;
            const float glyph_y = float(gui_numeric::integer(static_cast<std::int8_t>(glyph[10])) *
                                            gui_numeric::number(scale_y) +
                                        gui_numeric::number(y));
            if (!(glyph_x >= gui_numeric::number(min_x)))
                min_x = float(glyph_x);
            if (!(glyph_y >= min_y))
                min_y = glyph_y;
            const long double right =
                gui_numeric::integer(glyph[2]) * gui_numeric::number(scale_x) + glyph_x;
            const float bottom =
                float(gui_numeric::integer(glyph[3]) * gui_numeric::number(scale_y) +
                      gui_numeric::number(glyph_y));
            if (gui_numeric::number(max_x) < right)
                max_x = float(right);
            if (max_y < bottom)
                max_y = bottom;
            previous = code;
            x = float(gui_numeric::integer(advance(font, glyph)) * gui_numeric::number(scale_x) +
                      pen_x);
        }
        ++text;
    }
    if (left)
        *left = max_x <= min_x ? 0 : min_x;
    if (top)
        *top = max_y <= min_y ? 0 : min_y;
    if (width)
        *width =
            max_x <= min_x ? 0 : float(gui_numeric::number(max_x) - gui_numeric::number(min_x));
    if (height_out)
        *height_out =
            max_y <= min_y ? 0 : float(gui_numeric::number(max_y) - gui_numeric::number(min_y));
}

Font *initialize_font(Font *font) {
    if (callbacks->initialize)
        callbacks->initialize(font);
    return font;
}

void release_font(Font *font) {
    if (callbacks->release)
        callbacks->release(font);
}

void set_callbacks(const Callbacks *value) {
    callbacks = value;
}

void configure(const Settings *value) {
    kernel::memory_copy(&state.settings, value, value->size);
}
} // namespace nfs::fonts
