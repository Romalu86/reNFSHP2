#include "nfs/graphics/fonts/font_renderer.hpp"
#include "nfs/graphics/resources/eagl_assets.hpp"
#include "nfs/gui/lifecycle/gui.hpp"

namespace nfs::graphics::font_renderer {
// The font file's writable runtime extension is at +70h.
void __cdecl initialize(fonts::Font *font) {
    auto *object = reinterpret_cast<Dword *>(font);
    ++references;
    auto *image = reinterpret_cast<Dword *>(reinterpret_cast<std::uint8_t *>(font) + object[7]);
    backend::selected_context(backend::selected_device());
    Dword *caps = backend::adapter_caps();
    Dword format = tar::image_format(image);
    bool supported = false;
    if (format) {
        for (unsigned i = 0; i != 36; ++i)
            if (caps[6912 / 4 + i] == format)
                supported = true;
    }
    const auto *dimensions = reinterpret_cast<const std::int16_t *>(image);
    Dword *texture;
    if (format && ((dimensions[3] & (static_cast<Dword>(dimensions[3]) - 1)) == 0) && supported) {
        auto *storage = static_cast<Dword *>(eagl::allocator(48, "EAGL::TAR new"));
        if (storage)
            tar::construct_shape(storage, image);
        texture = storage;
    } else {
        image = convert_shape(image);
        object[116 / 4] = reinterpret_cast<Dword>(image);
        texture = tar::create_image(image);
    }
    object[112 / 4] = reinterpret_cast<Dword>(texture);
    auto *floats = reinterpret_cast<float *>(object);
    const auto *image_size = reinterpret_cast<const std::int16_t *>(image);
    floats[124 / 4] = 1.0f / image_size[2];
    floats[120 / 4] = 1.0f / image_size[3];
    object[40 / 4] = 0;
    object[44 / 4] = 1065353216;
    object[48 / 4] = object[52 / 4] = 0;
    object[56 / 4] = object[60 / 4] = 1065353216;
    geoprim::primitive(state, 4);
    geoprim::alpha_blend_mode(state, 1);
    geoprim::alpha_method(state, 5);
    geoprim::culling(state, 0);
    geoprim::depth_method(state, 4);
    geoprim::shading(state, 1);
    geoprim::texture_enable(state, 1);
    geoprim::texture_coordinates(state, 1);
    geoprim::transparency(state, 1);
    if (!vertices) {
        Dword count = backend::font_glyph_capacity(backend::selected_device());
        vertices = static_cast<Vertex *>(eagl::allocator(count * 112, "FontVertex new[]"));
    }
    if (!indices) {
        Dword count = backend::font_glyph_capacity(backend::selected_device());
        indices = static_cast<std::uint16_t *>(eagl::allocator(count * 12, "Font indices"));
    }
    stages.values[3][0] = 0;
    stages.values[2][0] = 2;
    stages.values[0][0] = (caps[692 / 4] & 8) ? 4 : 2;
    stages.values[7][0] = 0;
    stages.values[6][0] = 2;
    stages.values[4][0] = (caps[692 / 4] & 8) ? 4 : 2;
}

void __cdecl release(fonts::Font *font) {
    auto *object = reinterpret_cast<Dword *>(font);
    --references;
    auto *texture = reinterpret_cast<Dword *>(object[112 / 4]);
    if (texture) {
        tar::destroy(texture);
        eagl::deallocator(texture, 48);
        object[112 / 4] = 0;
    }
    object[112 / 4] = 0;
    if (!references) {
        if (vertices)
            eagl::deallocator(vertices, 28);
        vertices = nullptr;
        if (indices)
            gui::application_release(indices);
        indices = nullptr;
    }
    auto *converted = reinterpret_cast<Dword *>(object[116 / 4]);
    if (converted) {
        eagl::deallocator(converted - 1, converted[-1]);
        object[116 / 4] = 0;
    }
}
} // namespace nfs::graphics::font_renderer
