#include "nfs/graphics/fonts/patchwork_font.hpp"
#include "nfs/graphics/resources/eagl_assets.hpp"
#include "nfs/graphics/textures/texture_conversion.hpp"
#include "nfs/graphics/display/viewport_transform.hpp"
namespace nfs::fonts::patchwork {
namespace {
const Byte *glyph_record(Font *font, std::uint16_t code) {
    const int stride = (font->flags & 262144) ? 16 : 12;
    const Byte *records = reinterpret_cast<const Byte *>(font) + font->glyph_offset;
    const int index = int(code) - 32;
    if (index < font->count) {
        const Byte *glyph = records + index * stride;
        if (*reinterpret_cast<const std::uint16_t *>(glyph) == code)
            return glyph;
    }
    return fonts::find_code(code, records, font->count, stride);
}
Byte *pixels(Dword *shape) {
    return reinterpret_cast<Byte *>(shape) + ((shape[3] & 4096) ? shape[4] : 16);
}
} // namespace


Glyph *find(Cache *cache, std::uint16_t code, Font *font) {
    if (!glyph_record(font, code))
        return nullptr;
    Glyph *head = cache->head, *glyph = head;
    if (glyph) {
        while ((glyph->code != code || glyph->font != font) && glyph->code != 0 && glyph->next)
            glyph = glyph->next;
        if (glyph != head) {
            head->previous = glyph;
            if (glyph->previous)
                glyph->previous->next = glyph->next;
            if (glyph->next)
                glyph->next->previous = glyph->previous;
            glyph->previous = nullptr;
            glyph->next = head;
            cache->head = glyph;
        }
    }
    return glyph;
}

Byte cache_glyph(Cache *cache, Dword code, Font *font) {
    Glyph *glyph = find(cache, static_cast<std::uint16_t>(code), font);
    if (!glyph || glyph->code == static_cast<std::uint16_t>(code) || !font)
        return 0;
    Page *page = cache->pages + glyph->page;
    if (!page || !page->image)
        return 0;
    const Byte *record = glyph_record(font, static_cast<std::uint16_t>(code));
    if (!record)
        return 0;
    glyph->code = static_cast<std::uint16_t>(code);
    glyph->width = record[2];
    glyph->height = record[3];
    glyph->font = font;
    glyph->advance = static_cast<std::int8_t>(record[(font->flags & 262144) ? 14 : 8]);
    glyph->bearing_x = static_cast<std::int8_t>(record[9]);
    glyph->bearing_y = static_cast<std::int8_t>(record[10]);
    auto *source = reinterpret_cast<Dword *>(reinterpret_cast<Byte *>(font) + font->unknown_1c);
    auto *palette = graphics::textures::shape::palette(source);
    const Byte *indices = pixels(source);
    const int source_pitch = graphics::textures::shape::pitch(source);
    const auto *colours = reinterpret_cast<const Dword *>(pixels(palette));
    auto *destination = pixels(page->image);
    const int destination_pitch = graphics::textures::shape::pitch(page->image);
    if (static_cast<int>(glyph->width) >= cache->cell_width)
        glyph->width = static_cast<Byte>(cache->cell_width);
    if (static_cast<int>(glyph->height) >= cache->cell_height)
        glyph->height = static_cast<Byte>(cache->cell_height);
    const auto *origin = reinterpret_cast<const std::uint16_t *>(record + 4);
    int source_row = origin[1] * source_pitch;
    int destination_row = glyph->y * destination_pitch;
    for (unsigned y = 0; y < glyph->height; ++y) {
        auto *out =
            reinterpret_cast<std::uint16_t *>(destination + (destination_row / 2 + glyph->x) * 2);
        unsigned source_x = origin[0];
        for (unsigned x = 0; x < glyph->width; ++x, ++source_x) {
            Byte packed = indices[source_row + source_x / 2];
            Dword colour = colours[(source_x & 1) ? packed >> 4 : packed & 15];
            out[x] =
                static_cast<std::uint16_t>(((colour >> 16) & 61440) | ((colour >> 12) & 3840) |
                                           ((colour >> 8) & 240) | ((colour >> 4) & 15));
        }
        source_row += source_pitch;
        destination_row += destination_pitch;
    }
    page->dirty = 1;
    return 1;
}
} // namespace nfs::fonts::patchwork
