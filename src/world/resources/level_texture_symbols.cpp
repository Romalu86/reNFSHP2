#include "nfs/world/lifecycle/world_lifecycle.hpp"
#include "nfs/graphics/resources/eagl_objects.hpp"
#include "nfs/graphics/textures/shape_conversion.hpp"
#include "nfs/graphics/fonts/font_faces.hpp"
#include <cstring>
#include <ctype.h>
namespace nfs::game::runtime::particles::pending {
void fsh_directory_name(kernel::Dword *, int, kernel::Dword *);
}
namespace nfs::graphics::viewports {

void register_shape_symbols(Dword *image) {
    char symbol[11] = "shape_";
    const int count = static_cast<int>(image[2]);
    for (int index = 0; index < count; ++index) {
        auto *shape =
            reinterpret_cast<Dword *>(reinterpret_cast<Dword>(image) + image[5 + index * 2]);
        const char *name = textures::shape::shape_name(shape);
        if (name && std::strlen(name) <= 4) {
            std::strncpy(symbol + 6, name, 4);
        } else {
            game::runtime::particles::pending::fsh_directory_name(
                image, index, reinterpret_cast<Dword *>(symbol + 6));
            char *last = symbol + 9;
            do {
                if (*last != ' ')
                    break;
                *last-- = 0;
            } while (last >= symbol);
        }
        symbol[10] = 0;
        std::uint8_t found;
        eagl::pool_lookup(&eagl::external_symbols, symbol, &found);
        if (!found)
            register_symbol(symbol, reinterpret_cast<Dword>(shape));
    }
}
// Removal pads trailing whitespace with spaces; registration removes trailing spaces. Character
// classification uses the CRT locale and DBCS rules.
 
void unregister_shape_symbols(Dword *image) {
    char symbol[11] = "shape_";
    for (int index = 0; index < static_cast<int>(image[2]); ++index) {
        game::runtime::particles::pending::fsh_directory_name(image, index,
                                                              reinterpret_cast<Dword *>(symbol + 6));
        symbol[10] = 0;
        char *last = symbol + 9;
        while (!*last || _isctype(static_cast<signed char>(*last), 8))
            *last-- = ' ';
        Dword shape = reinterpret_cast<Dword>(image) + image[5 + index * 2];
        std::uint8_t found;
        Dword registered = eagl::pool_lookup(&eagl::external_symbols, symbol, &found);
        if (found && registered == shape)
            eagl::pool_remove(&eagl::external_symbols, symbol);
    }
}
} // namespace nfs::graphics::viewports
namespace nfs::game::world::pending {

void register_level_texture_symbols(void *image) {
    if (image) {
        fonts::faces::begin_font_pool();
        graphics::viewports::register_shape_symbols(static_cast<Dword *>(image));
        fonts::faces::end_font_pool();
    }
}

void unregister_level_texture_symbols(void *image) {
    if (image)
        graphics::viewports::unregister_shape_symbols(static_cast<Dword *>(image));
}
} // namespace nfs::game::world::pending
