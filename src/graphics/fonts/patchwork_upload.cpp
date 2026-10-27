#include "nfs/graphics/fonts/patchwork_font.hpp"
#include "nfs/graphics/resources/eagl_assets.hpp"
#include "nfs/graphics/display/viewport_transform.hpp"
namespace nfs::fonts::patchwork {

void upload(Cache *cache, Dword *viewport) {
    bool began = false;
    int page_count = static_cast<const volatile Cache *>(cache)->page_count;
    for (int i = 0; i < page_count; ++i) {
        Page *page = cache->pages + i;
        if (!page->texture) {
            if (!began) {
                graphics::viewports::end(viewport);
                began = true;
            }
            auto *storage =
                static_cast<Dword *>(lifecycle::allocate(48, "patchworkfont", lifecycle::ui_pool));
            if (storage)
                storage = graphics::tar::construct_shape(storage, cache->pages[i].image);
            // Allocation and construction may replace the page array independently.
            page = cache->pages + i;
            page->texture = storage;
        } else if (page->dirty) {
            if (!began) {
                graphics::viewports::end(viewport);
                began = true;
            }
            graphics::tar::replace_image(page->texture, page->image);
        }
        page_count = static_cast<const volatile Cache *>(cache)->page_count;
        static_cast<volatile Page *>(page)->dirty = 0;
    }
    if (began)
        graphics::viewports::begin(viewport);
}
} // namespace nfs::fonts::patchwork
