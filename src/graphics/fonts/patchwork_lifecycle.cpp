#include "nfs/graphics/fonts/patchwork_font.hpp"
#include "nfs/graphics/textures/shape_conversion.hpp"
#include "nfs/graphics/resources/eagl_assets.hpp"
#include "nfs/graphics/primitives/immediate_draw.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include "nfs/core/lifecycle/object_state.hpp"
#include "nfs/resources/archives/archive_cache.hpp"
#include <cstring>
namespace nfs::fonts::patchwork {
namespace gfx = graphics;
namespace objects = lifecycle::construction::objects;

void *construct_glyph(void *storage) {
    auto *glyph = static_cast<Glyph *>(storage);
    glyph->next = glyph->previous = nullptr;
    glyph->x = glyph->y = 0;
    glyph->width = glyph->height = 1;
    glyph->code = 0;
    return storage;
}
 
void destroy_glyph(void *) {
}

void *construct_page(void *storage) {
    auto *page = static_cast<Page *>(storage);
    page->image = page->texture = nullptr;
    page->dirty = 0;
    return storage;
}

void destroy_page(void *storage) {
    auto *page = static_cast<Page *>(storage);
    if (page->texture) {
        gfx::tar::destroy(page->texture);
        eagl::deallocator(page->texture, 48);
    }
    if (page->image)
        kernel::memory_free(page->image);
}

void initialize_page(Page *page, int width, int height) {
    page->image = gfx::textures::shape::allocate_shape(width, height, 16, 0, 0,
                                                       lifecycle::ui_pool | 1024, 0, 64);
    if (page->image) {
        reinterpret_cast<Byte *>(page->image)[0] = 109;
        auto *pixels = reinterpret_cast<Byte *>(page->image) +
                       ((page->image[3] & 4096) ? page->image[4] : 16);
        std::memset(pixels, 0, Dword(width) * Dword(height) * 2);
    }
}

void *delete_page(Page *page, unsigned flags) {
    if (flags & 2) {
        Dword *cookie = reinterpret_cast<Dword *>(page) - 1;
        objects::destroy_array(page, sizeof(Page), *cookie, destroy_page);
        if (flags & 1)
            kernel::memory_free(cookie);
        return cookie;
    }
    destroy_page(page);
    if (flags & 1)
        gui::application_release(page);
    return page;
}

Cache *construct(Cache *cache, Font *font, int width, int height, int requested_glyphs) {
    cache->cell_width = cache->cell_height = 16;
    cache->page_width = width;
    cache->page_height = height;
    cache->glyph_count = 0;
    cache->glyphs = nullptr;
    gfx::immediate::construct_textured(cache->draw);
    lifecycle::construction::identity_matrix(cache->matrix);
    lifecycle::construction::render_state(cache->render_state);
    if (font->count) {
        const Byte *glyph = reinterpret_cast<const Byte *>(font) + font->glyph_offset;
        const int stride = (font->flags & 262144) ? 16 : 12;
        int max_width = cache->cell_width, last_height = 0;
        for (unsigned i = 0; i < font->count; ++i, glyph += stride) {
            if (max_width < glyph[2])
                max_width = glyph[2];
            last_height = max_width <= glyph[3] ? glyph[3] : max_width;
        }
        cache->cell_width = max_width;
        cache->cell_height = last_height;
    }
    const int columns = cache->page_width / cache->cell_width;
    cache->glyphs_per_page = (cache->page_height / cache->cell_height) * columns;
    if (requested_glyphs < 2)
        requested_glyphs = 1;
    cache->page_count = (requested_glyphs - 1) / cache->glyphs_per_page + 1;
    cache->glyph_count = cache->page_count * cache->glyphs_per_page;
    auto *page_cookie = static_cast<Dword *>(files::archives::allocate_entry(
        Dword(cache->page_count) * sizeof(Page) + 4, "glyphpages", lifecycle::ui_pool));
    Page *pages = nullptr;
    if (page_cookie) {
        *page_cookie = cache->page_count;
        pages = reinterpret_cast<Page *>(page_cookie + 1);
        lifecycle::construction::construct_array(pages, sizeof(Page), cache->page_count,
                                                 construct_page, destroy_page);
    }
    cache->pages = pages;
    if (pages) {
        for (int i = 0; i < cache->page_count; ++i) {
            initialize_page(pages + i, cache->page_width, cache->page_height);
            // Both allocations and the overwrite occur in the constructor.
            initialize_page(pages + i, cache->page_width, cache->page_height);
        }
    }
    auto *glyph_cookie = static_cast<Dword *>(files::archives::allocate_entry(
        Dword(cache->glyph_count) * sizeof(Glyph) + 4, "glyphs", lifecycle::ui_pool));
    Glyph *glyphs = nullptr;
    if (glyph_cookie) {
        *glyph_cookie = cache->glyph_count;
        glyphs = reinterpret_cast<Glyph *>(glyph_cookie + 1);
        lifecycle::construction::construct_array(glyphs, sizeof(Glyph), cache->glyph_count,
                                                 construct_glyph, destroy_glyph);
    }
    cache->glyphs = cache->head = glyphs;
    for (int i = 0; i < cache->glyph_count; ++i) {
        Glyph &glyph = glyphs[i];
        if (i < cache->glyph_count - 1) {
            glyph.next = glyphs + i + 1;
            glyphs[i + 1].previous = &glyph;
        }
        glyph.page = static_cast<Byte>(i / cache->glyphs_per_page);
        const int local = i - glyph.page * cache->glyphs_per_page;
        const int row = local / columns;
        glyph.x = static_cast<std::uint16_t>(cache->cell_width * (local - row * columns));
        glyph.y = static_cast<std::uint16_t>(cache->cell_height * row);
    }
    gfx::geoprim::shading(cache->render_state, 1);
    gfx::geoprim::texture_enable(cache->render_state, 1);
    gfx::geoprim::alpha_test(cache->render_state, 0);
    gfx::geoprim::alpha_blend_mode(cache->render_state, 1);
    gfx::geoprim::transparency(cache->render_state, 1);
    gfx::geoprim::culling(cache->render_state, 0);
    gfx::geoprim::depth_method(cache->render_state, 8);
    gfx::immediate::initialize_textured(cache->draw);
    std::memcpy(cache->draw + 84 / 4, cache->matrix, sizeof(cache->matrix));
    gfx::immediate::set_render_state(cache->draw, cache->render_state);
    return cache;
}

void destroy(Cache *cache) {
    if (cache->pages) {
        auto *cookie = reinterpret_cast<Dword *>(cache->pages) - 1;
        objects::destroy_array(cache->pages, sizeof(Page), *cookie, destroy_page);
        kernel::memory_free(cookie);
    }
    if (cache->glyphs) {
        auto *cookie = reinterpret_cast<Dword *>(cache->glyphs) - 1;
        objects::destroy_array(cache->glyphs, sizeof(Glyph), *cookie, destroy_glyph);
        kernel::memory_free(cookie);
    }
    gfx::geoprim::destroy(cache->render_state);
    gfx::immediate::destroy_textured(cache->draw);
}
} // namespace nfs::fonts::patchwork
