#include "nfs/gui/engine/gui_engine.hpp"
namespace nfs::gui::engine {
Dword palette_serial{};

Palette *construct_palette(Palette *palette) {
    palette->methods = gui::base_methods;
    for (auto &colour : palette->colours)
        colour = 0;
    gui::string_construct(&palette->name);
    palette_serial += 4;
    palette->handle = palette_serial;
    palette->methods = palette_methods;
    for (auto &colour : palette->colours)
        colour = 4278190080u;
    palette->colours[0] = palette->colours[1] = palette->colours[2] = palette->colours[3] =
        4286545791u;
    palette->colours[4] = 4294967295u;
    palette->colours[6] = 4293717228u;
    palette->colours[7] = 4282335039u;
    palette->colours[11] = 2139062271;
    return palette;
}

void destroy_palette(Palette *palette) {
    gui::string_destruct(&palette->name);
    palette->methods = gui::base_methods;
}

static Palette *__fastcall delete_palette(Palette *palette, void *, unsigned flags) {
    destroy_palette(palette);
    if (flags & 1)
        gui::release(palette);
    return palette;
}
const Destroy palette_methods[] = {reinterpret_cast<Destroy>(delete_palette)};
} // namespace nfs::gui::engine
