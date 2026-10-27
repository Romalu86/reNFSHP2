#pragma once
#include "nfs/graphics/fonts/fonts.hpp"
namespace nfs::fonts::patchwork {
using kernel::Dword;
struct Page {
    Dword *image, *texture;
    Byte dirty, padding[3];
};
struct Glyph {
    Glyph *next, *previous;
    std::uint16_t x, y;
    Byte width, height;
    std::uint16_t code;
    std::int8_t advance, bearing_x, bearing_y;
    Byte page;
    Font *font;
};
struct Cache {
    int cell_width, cell_height, page_width, page_height;
    int glyph_count, glyphs_per_page, page_count;
    Page *pages;
    Glyph *glyphs, *head;
    Dword draw[152 / 4], matrix[16], render_state[68 / 4];
};
static_assert(sizeof(Page) == 12 && sizeof(Glyph) == 24 && sizeof(Cache) == 324);
static_assert(offsetof(Cache, draw) == 40 && offsetof(Cache, matrix) == 192 &&
              offsetof(Cache, render_state) == 256);
void *construct_glyph(void *);
void destroy_glyph(void *);
void *construct_page(void *);
void destroy_page(void *);
void initialize_page(Page *, int, int);
void *delete_page(Page *, unsigned);
Cache *construct(Cache *, Font *, int, int, int);
void destroy(Cache *);
void upload(Cache *, Dword *);
Glyph *find(Cache *, std::uint16_t, Font *);
Byte cache_glyph(Cache *, Dword, Font *);
void draw_glyph(Cache *, const Glyph *, Dword *, float, float, float, float, Dword);
void draw_page(Cache *, Dword *, float, float, float, Dword);
void draw_text(Cache *, Font *, const char *, Dword *, float, float, float, float, Dword);
void draw_wide(Cache *, Font *, const std::uint16_t *, Dword *, float, float, float, float, Dword);
} // namespace nfs::fonts::patchwork
