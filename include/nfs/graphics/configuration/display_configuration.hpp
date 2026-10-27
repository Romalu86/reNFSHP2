#pragma once
#include "nfs/graphics/configuration/render_settings.hpp"
namespace nfs::graphics::startup {
extern Dword &fullscreen_override, &windowed_override, &refresh_context;
extern std::uint8_t cursor_clipped;
extern char display_caption[];
int centre_window(std::uint8_t);
int configure_display(std::uint8_t, float *, float *, std::uint8_t *);
Dword set_gamma(const WORD *);
} // namespace nfs::graphics::startup
namespace nfs::graphics::symbols {
extern Dword ambient_colour[4], light_direction[4], light_colour[4], shadow_colour_scale[4];
Dword resolve_lighting(const char *, std::uint8_t *);
Dword resolve_module(const char *, std::uint8_t *);
} // namespace nfs::graphics::symbols
