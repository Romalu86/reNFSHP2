#include "gui_images.hpp"
#include "nfs/graphics/particles/particle_lifecycle.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include <cstdlib>
#include <cstring>
namespace nfs::gui::engine::images::texture_pending {
using game::runtime::at;
using game::runtime::bytes;

static bool shape_conversion_required(graphics::TextureReader *reader, Dword *shape) {
    using namespace graphics::textures;
    int width = at<short>(shape, 4), height = at<short>(shape, 6);
    if (width == 1 && height == 1)
        return false;
    Byte scan_alpha = at<Byte>(reinterpret_cast<const void *>(reader->fields[7]), 4);
    const Dword type = *shape & 127;
    const bool compressed = type == 96 || type == 97 || type == 98;
    const auto format_for_type = [](Dword type) {
        for (const auto &entry : formats)
            if (entry.shape == type)
                return entry.format;
        return Dword(23);
    };
    const Dword current = format_for_type(type);
    bool supported = false;
    if (compressed && current)
        supported = supports_format(reader, current);
    if (static_cast<signed char>(*shape) < 0 || compressed)
        scan_alpha = 0;
    Dword selected = current;
    if (!compressed || !supported) {
        const Dword transparency = alpha(shape, scan_alpha);
        const Dword depth = graphics::tar::image_depth(shape);
        selected = fallback_format(depth, transparency);
    }
    if (current != selected)
        return true;
    clamp_dimensions(reader, reinterpret_cast<Dword *>(&width), reinterpret_cast<Dword *>(&height));
    return width != at<short>(shape, 4) || height != at<short>(shape, 6);
}

Byte conversion_required(graphics::TextureReader *reader, Dword *file) {
    const Dword count = file[2];
    for (Dword index = 0; index < count; ++index)
        if (shape_conversion_required(reader,
                                      reinterpret_cast<Dword *>(bytes(file) + file[5 + index * 2])))
            return 1;
    return 0;
}

void copy_scratch(game::runtime::particles::lifecycle::TextureScratch *scratch, const void *source,
                  Dword size) {
    auto *owner = scratch->owner;
    if (owner->fields[12] < size) {
        owner->fields[12] = size;
        if (owner->fields[11])
            std::free(reinterpret_cast<void *>(owner->fields[11]));
        owner->fields[11] = size ? reinterpret_cast<Dword>(std::malloc(size)) : 0;
        if (size && !owner->fields[11])
            owner->fields[12] = 0;
    }
    if (owner->fields[12] < size)
        size = owner->fields[12];
    std::memcpy(reinterpret_cast<void *>(owner->fields[11]), source, size);
}
} // namespace nfs::gui::engine::images::texture_pending
