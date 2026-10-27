#include "nfs/graphics/configuration/render_settings.hpp"
#include "nfs/graphics/backend/eagl_backend.hpp"

namespace nfs::graphics::startup {
Dword *display_caps = nullptr;
namespace {
const Dword *mode_row(const Dword *caps, int index) {
    return caps + 768 / 4 + index * 6;
}
DisplayMode &selected(RenderSettings *value, std::uint8_t frontend) {
    return frontend ? value->frontend : value->race;
}
void copy_mode(DisplayMode &mode, const Dword *caps) {
    const Dword *row = mode_row(caps, mode.index);
    mode.colour_depth = backend::format_depth(row[5]);
    mode.width = row[1];
    mode.height = row[2];
}
} // namespace

// Availability/size come from the shared caps, format from the argument.
int smallest_mode(const Dword *caps, int depth) {
    int smallest_width = 2147483647, smallest_height = 2147483647, result = -1;
    for (int index = 0; index != 256; ++index) {
        const Dword *row = mode_row(display_caps, index);
        if (!row[0])
            continue;
        int width = row[1], height = row[2];
        int row_depth = backend::format_depth(mode_row(caps, index)[5]);
        if ((depth == -1 || depth == row_depth) &&
            (result == -1 || (width <= smallest_width && height <= smallest_height))) {
            result = index;
            smallest_width = width;
            smallest_height = height;
        }
    }
    return result;
}


int find_mode(const Dword *caps, int width, int height, int depth) {
    if (width == -1 || height == -1)
        return smallest_mode(caps, depth);
    for (int index = 0; index != 256; ++index) {
        const Dword *row = mode_row(display_caps, index);
        if (row[0] && static_cast<int>(row[1]) == width && static_cast<int>(row[2]) == height &&
            (depth == -1 ||
             depth == static_cast<int>(backend::format_depth(mode_row(caps, index)[5]))))
            return index;
    }
    return -1;
}


std::uint8_t fallback_mode(RenderSettings *value, const Dword *caps, std::uint8_t frontend) {
    DisplayMode &mode = selected(value, frontend);
    settings.supports_desktop = 0;
    mode.stencil = 0;
    mode.index = mode.depth_bits = -1;
    if (frontend) {
        mode.index = find_mode(caps, 800, 600, 16);
        if (mode.index == -1)
            mode.index = find_mode(caps, 800, 600, 32);
        if (mode.index == -1)
            return 0;
        mode.colour_depth = backend::format_depth(mode_row(caps, mode.index)[5]);
        mode.width = 800;
        mode.height = 600;
        return 1;
    }
    const int sizes[][2] = {{512, 400}, {512, 480}, {640, 400}, {640, 480}, {-1, -1}};
    for (const auto &size : sizes) {
        mode.index = find_mode(caps, size[0], size[1], 16);
        if (mode.index != -1)
            break;
    }
    if (mode.index == -1)
        return 0;
    copy_mode(mode, caps);
    return 1;
}


std::uint8_t default_mode(RenderSettings *value, const Dword *caps, std::uint8_t frontend,
                          Dword low_colour) {
    DisplayMode &mode = selected(value, frontend);
    mode.stencil = 0;
    mode.index = -1;
    if (mode.colour_depth < 1)
        mode.colour_depth = low_colour ? 16 : 32;
    int depth = mode.colour_depth;
    mode.index = find_mode(caps, 800, 600, depth);
    if (mode.index != -1) {
        mode.width = 800;
        mode.height = 600;
        if (frontend || !low_colour)
            return 1;
    }
    const int sizes[][2] = {{640, 480}, {640, 400}, {512, 384}, {400, 300}, {-1, -1}};
    for (const auto &size : sizes) {
        mode.index = find_mode(caps, size[0], size[1], depth);
        if (mode.index != -1)
            break;
    }
    if (mode.index == -1)
        return 0;
    copy_mode(mode, caps);
    return 1;
}


std::uint8_t choose_depth(RenderSettings *value, const Dword *caps, std::uint8_t frontend) {
    DisplayMode &mode = selected(value, frontend);
    const Dword adapter = caps[528 / 4];
    mode.depth_bits = 0;
    mode.stencil = 0;
    std::uint8_t windowed = !fullscreen(value);
    auto supports = [&](int depth, int stencil) {
        return backend::supports_format(context, adapter, mode.colour_depth, depth, stencil,
                                        windowed);
    };
    if (caps[684 / 4] & 64) {
        if (supports(24, 8))
            mode.stencil = 8;
        else if (supports(24, 4))
            mode.stencil = 4;
        if (mode.stencil)
            mode.depth_bits = 24;
    }
    if (mode.depth_bits)
        return 1;
    if (supports(16, 0))
        mode.depth_bits = 16;
    else if (supports(24, 0))
        mode.depth_bits = 24;
    else if (supports(32, 0))
        mode.depth_bits = 32;
    return mode.depth_bits != 0;
}

// Negative indices are not checked.
std::uint8_t valid_mode(const Dword *caps, const DisplayMode *mode, std::uint8_t frontend) {
    if (mode->index >= 256)
        return 0;
    const Dword *row = mode_row(caps, mode->index);
    if (row[0] && row[1] == static_cast<Dword>(mode->width) &&
        row[2] == static_cast<Dword>(mode->height) &&
        backend::format_depth(row[5]) == static_cast<Dword>(mode->colour_depth) &&
        mode->depth_bits > 0)
        return !frontend || (row[1] == 800 && row[2] == 600);
    return 0;
}


std::uint8_t reconcile_modes(RenderSettings *value, const Dword *caps, std::uint8_t low_colour) {
    if (!valid_mode(caps, &value->frontend, 1) && !default_mode(value, caps, 1, low_colour))
        fallback_mode(value, caps, 1);
    if (value->frontend.index >= 256 || !mode_row(caps, value->frontend.index)[0])
        return 0;
    choose_depth(value, caps, 1);
    if (value->frontend.depth_bits <= 0)
        return 0;
    if (!valid_mode(caps, &value->race, 0) && !default_mode(value, caps, 0, low_colour) &&
        !fallback_mode(value, caps, 0))
        return 0;
    if (value->race.index >= 256 || !mode_row(caps, value->race.index)[0])
        return 0;
    choose_depth(value, caps, 0);
    return value->race.depth_bits > 0;
}


std::uint8_t validate_modes(RenderSettings *value, std::uint8_t low_colour) {
    Dword *caps = display_caps;
    if (reconcile_modes(value, caps, low_colour))
        return 1;
    settings.supports_desktop = 0;
    return reconcile_modes(value, caps, low_colour) != 0;
}


std::uint8_t modes_match(const RenderSettings *value) {
    return value->race.index == value->frontend.index &&
           value->race.depth_bits == value->frontend.depth_bits &&
           value->race.stencil == value->frontend.stencil;
}

std::uint8_t stencil_enabled(const RenderSettings *value, std::uint8_t frontend) {
    return (frontend ? value->frontend.stencil : value->race.stencil) != 0;
}

std::uint8_t mipmaps_enabled(const RenderSettings *value) {
    return value->mipmap != 0;
}

std::uint8_t prefer_low_colour() {
    if (!display_caps)
        return 0;
    Dword adapter = display_caps[516 / 4];
    return adapter == 268587332 || adapter == 268587353;
}
} // namespace nfs::graphics::startup
