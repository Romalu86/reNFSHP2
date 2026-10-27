#pragma once
#include "nfs/graphics/resources/eagl_graphics.hpp"
namespace nfs::graphics::startup {
struct DisplayMode {
    int index, colour_depth, depth_bits, stencil, width, height;
};
struct RenderSettings {
    Dword supports_desktop, limit_resolution, mipmap;
    float gamma;
    DisplayMode race, frontend;
    Dword fullscreen;
};
static_assert(sizeof(RenderSettings) == 68 && offsetof(RenderSettings, frontend) == 40);
extern RenderSettings settings;
extern Dword* display_caps;
extern void *settings_owner;

// neither its capacity nor its command-line producer has been established.
extern Dword &configuration_variant_enabled;
extern char (&configuration_variant)[24];
extern char configuration_buffer[];
const char *configuration_path();
void read_render_settings();
void write_render_settings();
void initialize_render_settings();
void shutdown_render_settings();
int width(const RenderSettings *, std::uint8_t);
int height(const RenderSettings *, std::uint8_t);
bool fullscreen(const RenderSettings *);
int smallest_mode(const Dword*, int);
int find_mode(const Dword*, int, int, int);
std::uint8_t fallback_mode(RenderSettings*, const Dword*, std::uint8_t);
std::uint8_t default_mode(RenderSettings*, const Dword*, std::uint8_t, Dword);
std::uint8_t choose_depth(RenderSettings*, const Dword*, std::uint8_t);
std::uint8_t valid_mode(const Dword*, const DisplayMode*, std::uint8_t);
std::uint8_t reconcile_modes(RenderSettings*, const Dword*, std::uint8_t);
std::uint8_t validate_modes(RenderSettings*, std::uint8_t);
std::uint8_t modes_match(const RenderSettings*);
std::uint8_t stencil_enabled(const RenderSettings*, std::uint8_t);
std::uint8_t mipmaps_enabled(const RenderSettings*);
std::uint8_t prefer_low_colour();
} // namespace nfs::graphics::startup
